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
#include <climits>


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

	if(argc < 3)
	{
		printf("Invalid arguments\n");
		return 1;
		
	}
	int lines = atoi(argv[1]);
	int timeus = atoi(argv[2]);
   
    int semid = semget( 0xface, 1, IPC_CREAT|IPC_EXCL | 0666 );
	ErrorCheck(semid);
	if(semid>=0)
    {
		ErrorCheck(semctl( semid, 0, SETVAL, 1 ));
	}
	else if(semid<0 && errno == EEXIST)
	{
		semid = semget( 0xface, 1, IPC_CREAT| 0666 );
		ErrorCheck(semid);
	}
	
    char pipename[]="pipefile";
    sembuf up = { 0, 1, 0 }; 
    sembuf down = { 0, -1, 0 };
	int pipenode = mknod(pipename,S_IFIFO|0666,0);
	ErrorCheck(pipenode);
    
   
	
    int pipefile = open( pipename, O_WRONLY|O_CREAT|O_APPEND,0666 );
    //if ( pipefile < 0 ) {printf( "Opening %d failed with error %s\n", pipefile,strerror( errno ) );};
	ErrorCheck(pipefile);
	//if(pipefile <0){pipefile = open( "pipefile", O_WRONLY|O_APPEND,0666 );}
    srand(getpid());
     
	for(int j =0;j<lines;j++)
	{
        
        ErrorCheck(semop( semid, &down, 1 ));
        printf("Process %d\tIteration %d\n",getpid(),j);
		char exportbuf[512]={'\0'};
		int totallen=0;
		char outbuf[16];
		ErrorCheck(sprintf(outbuf,"%d ",getpid()));
		memcpy(exportbuf+totallen,outbuf,strlen(outbuf));
		totallen+=strlen(outbuf);
		
		
		int rannum = rand()%20 +1;
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
		ErrorCheck(sprintf(outbuf,"%d\n",sum));
		memcpy(exportbuf+totallen,outbuf,strlen(outbuf));
		totallen+=strlen(outbuf);	
		//ErrorCheck(write(1,exportbuf,strlen(exportbuf))); 
		ErrorCheck(write(pipefile,exportbuf,strlen(exportbuf)));  
		delete[] nums;
		usleep(timeus);
        ErrorCheck(semop( semid, &up, 1 ));
    }
    ErrorCheck(close(pipefile));
    
    
   
   // ErrorCheck(semop(semid,0,IPC_RMID));
    return 0;
}

