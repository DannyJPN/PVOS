#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>
//https://www.softprayog.in/programming/posix-semaphores



 #define ErrorCheck(value){    DetectError(value,__LINE__);           } 
void DetectError(int result,int linenum)
{
	if(result <0)
	{
		printf("%d\t%s\n",linenum,strerror(errno));
		
	}
	
}


void WriteArray(int*arr,int arrsize)
{
        for(int i =0;i<arrsize;i++)
        {
                printf("%d,",arr[i]);
        }
        printf("\n");
}

int main()
{
    int i;
    int pid= -1;
    int maxcount = 10;
    int processcount = 6;
	//int actions = 5;
    //int robosem = semget( 0x7626, 1, IPC_CREAT | 0660 );//robo
    //semctl( robosem, 0, SETVAL, 1);
	//printf( "Initial robosem: %d\n", semctl(robosem,0,GETVAL,NULL) );
   // int countsem = semget( 0x6294, 1, IPC_CREAT | 0660 );//maxi
    //semctl( countsem, 0, SETVAL, 0);
	//printf( "Initial countsem: %d\n", semctl(countsem,0,GETVAL,NULL) );
    char robofilename[32];
	char countfilename[32];

	ErrorCheck(sprintf(robofilename,"/robosem_%d",getpid()));
	ErrorCheck(sprintf(countfilename,"/countsem_%d",getpid()));

	sem_t*robosem = sem_open(robofilename,O_CREAT|O_RDWR,0660,1);
	sem_t*countsem = sem_open(countfilename,O_CREAT|O_RDWR,0660,0);
    //sem_t*fullsem = sem_open("/fullsem",O_CREAT|O_RDWR,0660,0);
   
	int *sharval = ( int * ) mmap( 0, sizeof( int ), PROT_READ | PROT_WRITE,         MAP_ANONYMOUS|    MAP_SHARED, -1, 0 );
	int currentpos=0;
	sharval=&currentpos;
    int sharmem_key = getuid();
	int sharmem_id = shmget(sharmem_key,maxcount*sizeof(int),0644|IPC_CREAT);
	int *sharmem=(int*)shmat(sharmem_id,NULL,0);

    for ( i = 0; i < processcount; i++ )
    {
        pid = fork();
        if ( pid == 0 ) break;
    }
   
    if ( pid )
    { 
        int  status=-1;
        printf( "I am parent with PID %d\n", getpid() );
        int wpid = -1;
        int tofinish = processcount;
        //cycle which should end when all children finish
        while(tofinish)
        {
            //printf( "I am Parent %d waiting\n", getpid() );
            //int actualval = semctl(countsem,0,GETVAL,NULL);
			//printf( "Actualval %d \n", actualval );
       
			//printf( "counter decrement from value: %d\n", semctl(countsem,0,GETVAL,NULL) );
            //semop( countsem, &countdown, 1);
			//printf( "counter decrement: %d\n", semctl(countsem,0,GETVAL,NULL) );
			ErrorCheck(sem_wait(countsem));

            if( (*sharval) < maxcount-1)
            { 
				printf("Box not full: %d\n",*sharval)
				continue;
				
			}
            
            
           ErrorCheck(		   sem_wait(robosem));
            
            
            
            
            printf( "I am Parent %d changing box\n", getpid() );
			//printf( "Parent will enable robots: %d\n", semctl(robosem,0,GETVAL,NULL) );
			for(int j = 0;j<maxcount;j++)
			{
				sharmem[j] = '\0';
				
			}
            

			printf( "Box changed: %d\n", *sharval );
            ErrorCheck(sem_post(robosem));
            

            
/*
            int currentpos = maxcount - actualval;

            WriteArray(sharmem,currentpos+1);*/

			//printf("blankride\n");


 



        }    


           while((wpid= waitpid(-1, &status ,WNOHANG)  )>0)
            {
				tofinish--;
                printf( "End process %d state %d,to finish: %d\n", wpid,WEXITSTATUS(status) ,tofinish);
            }

	
		

        

        

        printf( "Parent finish...\n" );

    }

    else

    { 

        for(int k = 0;k>-1;k++)

        {

            
			//printf( "Robot %d will start work: %d\n",i, semctl(robosem,0,GETVAL,NULL) );
            //sem_wait(emptysem);
            
            ErrorCheck(sem_wait(robosem));//critical section insert start
			printf( "I am Child %d with PID %d performing action %d\n", i,getpid(),k );
			//printf( "Robot %d can work: %d\n", i,semctl(robosem,0,GETVAL,NULL) );
            //int actualval = semctl(countsem,0,GETVAL,NULL);//what is the current value of the counter
            int actualval =0;
            ErrorCheck(sem_getvalue(countsem,&actualval));
            //int currentpos = maxcount - actualval;//calculate where to insert. 
			//int currentpos = actualval;
            

            

            sharmem[(*sharval)] = getpid();//insert itself
			(*sharval)++;
			WriteArray(sharmem,(*sharval)+1);
			//printf( "counter will increment: %d\n", semctl(countsem,0,GETVAL,NULL) );
            ErrorCheck(sem_post(countsem));
            ErrorCheck(sem_post(robosem));
        
			//printf( "counter increment: %d\n", semctl(countsem,0,GETVAL,NULL) );


            

        }

        

        exit(0);

        

    }

		printf( "Common code...\n" ); 
		ErrorCheck(sem_unlink ("/robosem"));
		ErrorCheck(sem_unlink ("/countsem"));
	
		shmctl(sharmem_id,IPC_RMID,0);


}

