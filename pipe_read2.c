#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>

int main()
{	
	int fifo_fd=-1;
	int child_pid;
	int status;
	int pipe_print_fd[2];

	char fifo_buf[128];
	char command_buf[128];
	char command_buf_2[128];
	char command[128];
	char print_buf[2048];
	char pipe_name[128];
	char outputfile[128];
	char inputfile[128];
	
	scanf("%s",pipe_name);
	
	fifo_fd=open(pipe_name,O_RDONLY);
	
	if(fifo_fd==-1)
		perror("read_pipe create failed!\n");

	if(pipe(pipe_print_fd))
		perror("pipe create failed!\n");

	while(1){
		
			
		memset(print_buf,0,sizeof(print_buf));
		memset(command_buf,0,sizeof(command_buf));
		memset(command_buf_2,0,sizeof(command_buf_2));	
		read(fifo_fd,fifo_buf,128);	
		
		int output=0;
		int input=0;
		int ispipe=0;
		int oldstdout=dup(1);
		int oldstdin=dup(0);
		int command_cursor=127;
		
		for(int i=0;i<128;i++)
		{
			if(fifo_buf[i]=='>')
			{
				if(i+1<127&&fifo_buf[i+1]=='>')
				{
					command_cursor=i-1;
					sscanf(fifo_buf+i+2,"%s",outputfile);
					output=2;
					break;		
				}
				else{
					command_cursor=i-1;
					sscanf(fifo_buf+i+1,"%s",outputfile);
					output=1;
					break;
				}
			}
			if(fifo_buf[i]=='<')
			{
				command_cursor=i-1;
				sscanf(fifo_buf+i+1,"%s",inputfile);
				input=1;
				break;
			}
			if(fifo_buf[i]=='|')
			{
				ispipe=1;
				command_cursor=i-1;
				memcpy(command_buf,fifo_buf+i+2,sizeof(char)*(strlen(fifo_buf)-i-2));
				memcpy(command_buf_2,fifo_buf,command_cursor*sizeof(char));
				command_buf_2[command_cursor]='\0';
				command_buf[strlen(fifo_buf)-i-2]='\0';
				break;
			}
		}
		if(ispipe==0)
		{
			memcpy(command_buf,fifo_buf,command_cursor*sizeof(char));
			command_buf[command_cursor]='\0';
		}
		//printf("%s\n",command_buf);
		//printf("%s\n",command_buf_2);
		
		if(output==1)
		{
			if((access(outputfile,0))==0)
				remove(outputfile);
			int fd_out=open(outputfile,O_WRONLY|O_CREAT,00700);
			dup2(fd_out,STDOUT_FILENO);
		}
		if(output==2)
		{
			int fd_out=open(outputfile,O_WRONLY|O_APPEND|O_CREAT,00700);
			dup2(fd_out,STDOUT_FILENO);
		}
		if(input==1)
		{
			int fd_out=open(outputfile,O_RDONLY);
			dup2(fd_out,STDOUT_FILENO);
		}	
		
		sscanf(command_buf,"%s",command);		

		if(memcmp(command,"pwd",strlen("pwd"))==0)
		{
			getcwd(print_buf,128);
			printf("%s\n",print_buf);
		}
		else if(memcmp(command,"echo",strlen("echo"))==0)
		{
			printf("%s\n",command_buf+strlen("echo")+1);
		}
		else if(memcmp(command,"cd",strlen("cd"))==0)
		{
			char parameter[128];
		
			sscanf(command_buf+3,"%s",parameter);

			if(chdir(parameter)==0)
			{
				chdir(parameter);
				getcwd(print_buf,128);
				printf("%s\n",print_buf);
			}
			else
			{
				exit(0);
			}
		}
		else if(memcmp(command,"exit",strlen("exit"))==0)
		{
			exit(0);
		}
		else{
			if(pipe(pipe_print_fd)!=0)
				perror("pipe create failed!\n");
			
			child_pid=fork();
			if(child_pid)
			{
				close(pipe_print_fd[1]);
				waitpid(-1, &status, 0);
				
				/*
				{
					read(pipe_print_fd[0],print_buf,sizeof(print_buf));
					printf("%s",print_buf);
				}*/
			}
			else
			{
				close(pipe_print_fd[0]);
				char temp[128];
				int cur=0,count=0;
				char *argv[100];
				while(cur<strlen(command_buf))
				{
					sscanf(command_buf+cur,"%s",temp);
					cur+=strlen(temp)+1;
					argv[count]=malloc(128*sizeof(char));
					strcpy(argv[count],temp);
					count++;
				}
				argv[count]=0;
				//dup2(pipe_print_fd[1],STDOUT_FILENO);
				if(ispipe==1)
				{
					int pipe_fd[2];
					pipe(pipe_fd);
					int child_pid_2=fork();
					if(child_pid_2)
					{
						wait(&status);
						close(STDIN_FILENO);
						dup2(pipe_fd[0],STDIN_FILENO);
						close(pipe_fd[1]);
					}
					else
					{
						cur=0;
						count=0;
						sscanf(command_buf_2+cur,"%s",command);
						while(cur<strlen(command_buf_2))
						{
							sscanf(command_buf_2+cur,"%s",temp);
							cur+=strlen(temp)+1;
							argv[count]=malloc(128*sizeof(char));
							strcpy(argv[count],temp);
							count++;
						}
						argv[count]=0;
						close(STDOUT_FILENO);
						dup2(pipe_fd[1],STDOUT_FILENO);
						close(pipe_fd[0]);
						execvp(command,argv);
					}
				}			
				execvp(command,argv);
			}
		}
		dup2(oldstdout,1);
		dup2(oldstdin,0);
	}
	exit(0);
}

