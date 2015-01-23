#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>    
#include <string.h>
#include <pthread.h>
/*
pthread_mutex_t* g_mutex;    
void init_mutex(void)    
{    
    int ret;    
    g_mutex=(pthread_mutex_t*)mmap(NULL, sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);    
    if( MAP_FAILED==g_mutex )    
    {    
        perror("mmap");    
        exit(1);    
SIZE    }    
     pthread_mutexattr_t attr;    
     pthread_mutexattr_init(&attr);    
     ret=pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);    
     if( ret!=0 )    
     {    
         perror("init_mutex pthread_mutexattr_setpshared");    
         exit(1);    
     }    
      pthread_mutex_init(g_mutex, &attr);    
}
*/   

#define SIZE 1 
void createsubprocess(int num)  
{  
    unsigned int i, ret;  
    unsigned int child;  
    unsigned int pid[num]; 
    char *p = NULL;    
    for(i=0;i<num;i++)  
    {  
        if((child = fork()) == -1)  
        {  
	    printf("error\n");
            perror("fork");  
            exit(EXIT_FAILURE);  
        }  
        else if(child==0)       //子进程运行  
        {  
	    pid[i]=getpid(); 
	    //ret=pthread_mutex_lock(g_mutex);    
            //if( ret!=0 )    
            //{    
            //    perror("child pthread_mutex_lock");    
            //}    
	    p = malloc(SIZE);
	    memset(p, 0, SIZE);
	    free(p);
	    printf("success\n");
	    //ret=pthread_mutex_unlock(g_mutex);      
            //if( ret!=0 )    
            //{    
            //    perror("child pthread_mutex_unlock");    
            //}    
	    exit(0);
           // if(execl("/home/is11/123/fork.py","fork.py",(char *)0) == -1 )  
           // {  
           //     perror("execl");  
           //     exit(EXIT_FAILURE);  
           // }  
        }  
          
        else  
        {
	         
        }     
    }  
    for(i=0;i<num;i++)  
    {  
        waitpid(pid[i],NULL,0);  
                  
    }  
}  
int main()
{
	//init_mutex();	
	int counter=1;
	while(counter<=240) 
	{
		createsubprocess(128);
		counter++;
		sleep(0.5);
	}
	 return 0;           
}
