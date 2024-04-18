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


int main(int argc,char**argv)
{


	int max = 10;
	int min = 2;

	char pipename[] = "pipefile";
	int node = mknod(pipename,S_IFIFO|0666,0);
	ErrorCheck(node);
	
	int pipefile = open( pipename, O_WRONLY|O_CREAT|O_APPEND,0666 );
    	//if ( pipefile < 0 ) {printf( "Opening %d failed with error %s\n", pipefile,strerror( errno ) );};
	ErrorCheck(pipefile);
	//if(pipefile <0){pipefile = open( "pipefile", O_WRONLY|O_APPEND,0666 );}
    	srand(getpid());
    	int lines =rand()%(max-min)+min;
    	int times = rand()%max;
	for(int j =0;j<lines;j++)
	{
		int wrong = rand()%10;
		char exportbuf[512]={'\0'};
		int totallen=0;
		char outbuf[16];
		/*ErrorCheck(sprintf(outbuf,"%d:",getpid()));
		memcpy(exportbuf+totallen,outbuf,strlen(outbuf));
		totallen+=strlen(outbuf);
		*/
		
		int rannum = rand()%max +1;
		int * nums = new int[rannum];
		int sum=0;
		
		for(int i =0;i<rannum;i++)
		{
			int ran = rand()%100;
			ErrorCheck(sprintf(outbuf,"%d ",ran));
			memcpy(exportbuf+totallen,outbuf,strlen(outbuf));
			totallen+=strlen(outbuf);        
			nums[i]=ran;
			sum+=ran;    
		}
		if(wrong)
		{
			ErrorCheck(sprintf(outbuf,"%d\n",sum));
		
		}
		else
		{
			ErrorCheck(sprintf(outbuf,"%d\n",10*sum));
		}
		
		memcpy(exportbuf+totallen,outbuf,strlen(outbuf));
		totallen+=strlen(outbuf);	
		//ErrorCheck(write(1,exportbuf,strlen(exportbuf))); 
		ErrorCheck(write(pipefile,exportbuf,strlen(exportbuf))); 
		ErrorCheck(write(1,exportbuf,strlen(exportbuf))); 
		 
		delete[] nums;
		sleep(times);
    	}
    	ErrorCheck(close(pipefile));
    


    return 0;
}

