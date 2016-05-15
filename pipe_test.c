#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>

int main()
{
	int fifo_fd;
	fifo_fd=open("pipe",O_WRONLY);
	char buf[128];
	char buf2[128];
	while(scanf("%[^\n]",buf))
	{
		write(fifo_fd,buf,128);
		printf("send finished \n");
		scanf("%c",buf);
	}
	return 0;
}
