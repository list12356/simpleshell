#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>

int main()
{
	int fifo_fd;
	char name[128];
	printf("Please input the name of the FIFO:");
	scanf("%s",name);
	fifo_fd=open(name,O_WRONLY);
	char buf[128];
	char buf2[128];
	while(1)
	{
		scanf("%[^\n]",buf);
		write(fifo_fd,buf,128);
		printf("send finished \n");
		scanf("%c",buf);
	}
	return 0;
}
