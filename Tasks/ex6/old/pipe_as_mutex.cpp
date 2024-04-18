#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>

 #define ErrorCheck(value){    DetectError(value,__LINE__);           } 
void DetectError(int result,int linenum)
{
	if(result <0)
	{
		printf("%d\t%s\n",linenum,strerror(errno));
		
	}
	
}

int main()
{


	char pipename[]="tunnel";
	char semname[]="sempipe";
	int pipenode = mknod(pipename,S_IFIFO|0666,0);
	ErrorCheck(pipenode);
	/*int mutexnode = mknod(semname,S_IFIFO|0666,0);
	ErrorCheck(mutexnode);*/



 //semcheck

	char pipeval[1]={'\0'};
	
	int sempipe;
	
	while(pipeval[0]  == '0')
	{
		pipeval[0]='\0';
		sempipe = open( semname, O_RDONLY|O_CREAT,0666 );
		ErrorCheck(sempipe)
		int readb = read(sempipe,pipeval,1);
		ErrorCheck(readb);
		ErrorCheck(close(sempipe));
		printf("stuck\n");
		usleep(100);
		
	}

 //semcheck
 
	//sem on
	sempipe = open( semname, O_WRONLY|O_CREAT,0666 );
	ErrorCheck(sempipe);
	
	ErrorCheck(write(sempipe,"0",1));
	
	ErrorCheck(close(sempipe)); 
	//sem on
	int pipefile = open( pipename, O_WRONLY|O_CREAT|O_APPEND,0666 );
	ErrorCheck(pipefile);
	char outbuf[16];
	ErrorCheck(sprintf(outbuf,"%d ",getpid()));
	ErrorCheck(write(pipefile,outbuf,strlen(outbuf)));
	srand(getpid());
	int rannum = rand()%15 +1;
	int * nums = new int[rannum];
	int sum=0;
	for(int i =0;i<rannum;i++)
	{
		int ran = getpid()%50;
		ErrorCheck(sprintf(outbuf,"%d ",ran));
		ErrorCheck(write(pipefile,outbuf,strlen(outbuf)));  
		nums[i]=ran;
		sum+=ran;    
	}
	ErrorCheck(sprintf(outbuf,"%d\n",sum));
	ErrorCheck(write(pipefile,outbuf,strlen(outbuf)));  
	ErrorCheck(close(pipefile));
	
	delete nums;
	//sem off
	sempipe = open( semname, O_WRONLY|O_CREAT,0666 );
	ErrorCheck(sempipe);
	
	ErrorCheck(write(sempipe,"1",1));
	
	ErrorCheck(close(sempipe)); 
	//sem off
	
	return 0;
}

