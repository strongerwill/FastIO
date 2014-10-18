#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
int main(int argc, char **argv, char **envp) 
{
	int counter = 0;
	FILE *fp;
        fp = fopen("/home/zz/zhi/update_iotlb/fork_xen/firefox.txt", "a");
	if(fp == NULL)
		printf("file open error\n");
	int ret;
	int ret1;
	int ret2;
	//int t=10;
	int flush_status;
	while(1)
	{	
        	//This call returns the size of the kernel messages
        	ret=klogctl(10, NULL,0);
        	char *buf=malloc(ret+8);
        	memset(buf, 0, ret+8); 
		//That one send all messages in "buf" with "ret" as maximum size, and empty the messages
        	ret=klogctl(4, buf, ret+8);
        	//And display the buffer.
        	//printf("%s\n", buf);
		ret1 = fputc('\n', fp);
		if(ret1==EOF)
			printf("fputc error\n");
		ret2 = fputs(buf, fp);
		if(ret2==EOF)
			printf("fputs error\n");
		flush_status=fflush(fp);
		if(flush_status!=0)
			puts("error flushing \n");
		else
			puts("success flushiing \n");
		
		sleep(60);	
		//counter++;
		free(buf);
		//t--;
	}
	fclose(fp);
        return 1;
}













