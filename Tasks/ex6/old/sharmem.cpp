#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>



 #define ErrorCheck(value){    DetectError(value,__LINE__);           } 
void DetectError(int result,int linenum)
{
	if(result <0)
	{
		printf("%d\t%s\n",linenum,strerror(errno));
		
	}
	
}









int ChildCode(int i ,int* sharmem,int arrlen,int maxval,int actionnum)
{
    printf("\n--------------------------------------------------------------------\n");
    printf( "I am child %d with PID %d performing action %d\n", i, getpid() ,actionnum);
    
    
    printf("Array before change: ");
    for(int j = 0;j<arrlen;j++)
    {
        printf("%d,",sharmem[j]);
    }
    printf("\n");
    int checksum =0;
	for(int j = 0;j<arrlen-1;j++)
    {
        checksum+=sharmem[j];
    }
    

    int sum = sharmem[arrlen-1];
    if(sum != checksum)
	{
		printf("ERROR inequal sums!!!\n");
		sleep(10);
		return -1;
		
	}
    else
    {
        printf("SUMS OK!!!\n");
		
        
    }
    
    int difference = 0;
	srand(getpid());
    int ranindex = rand()%(arrlen-1);
    int rannum = rand()%(maxval);
    sum-=sharmem[ranindex];
    difference-=sharmem[ranindex];
    sum+=rannum;
    difference+=rannum;
    printf("Replacing %d by %d,sum changes from %d to %d\n",sharmem[ranindex],rannum,sharmem[arrlen-1],sum);
    sharmem[ranindex] = rannum;
    
    sharmem[arrlen-1]=sum;
    
    printf("Array after change: ");
    for(int j = 0;j<arrlen;j++)
    {
        printf("%d,",sharmem[j]);
    }
    printf("\n--------------------------------------------------------------------\n");
    
    
    
    return abs(difference);
}







int main( int num_arg, char **args )
{
	int actions = 40000;
	int semid = semget( 0xffcb, 1, IPC_CREAT | 0660 );
	semctl( semid, 0, SETVAL, 1 );
	
	sembuf up = { 0, 1, 0 }; 
	sembuf down = { 0, -1, 0 };
    srand(getpid())	;
    int processcount = 100;
	int sharmem_key = getuid();
	int arrlen = 11;
    int maxval= 50;
    
    

	int sharmem_id = shmget(sharmem_key,arrlen*sizeof(int),0644|IPC_CREAT);
    ErrorCheck(sharmem_id);
	int *sharmem=(int*)shmat(sharmem_id,NULL,0);
    
    int sum = 0;
    for(int j = 0;j<arrlen-1;j++)
    {
        int rannum = rand()%maxval;
        sharmem[j] = rannum;
        sum+=rannum;
    }	
    sharmem[arrlen-1] = sum;
    
    for(int j = 0;j<arrlen;j++)
    {
        printf("%d,",sharmem[j]);
    }
    printf("\n");

	int pid = -1;
	int i;
	for ( i = 0; i < processcount; i++ )
	{
		pid = fork();
		if ( pid == 0 ) break;
	}

	if ( pid )
	{ 
		printf( "I am parent with PID %d\n", getpid() );
		int wpid=-1;
		int  status=-1;
		while((wpid= waitpid(-1, &status ,0)  )>0)
		{
		printf( "Ended proces %d with status %d\n", wpid,WEXITSTATUS(status) );
		
		}
		
		printf( "Parent finish...\n" );
	}
	else
	{ 
		srand( getpid() );
		for(int j = 0;j<actions;j++)
		{
		
			
			semop( semid, &down, 1 );
			int childres =ChildCode(i,sharmem,arrlen,maxval,j);
			
			semop( semid, &up, 1 );
            if(childres == -1){exit(-1);}
		}
		exit( 0 );
	}










shmctl(sharmem_id,IPC_RMID,0);

}
