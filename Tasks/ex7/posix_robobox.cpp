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
#include <errno.h>
#include <string.h>


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
                printf("(%d),",arr[i]);
        }
        printf("\n");
}



int main()
{
    int i;
    int pid= -1;
    int maxcount = 10;
    int processcount = 6;
    int maxval = 25;
    int semval=0;
    
    sem_t* robosem = sem_open( "/robosem", O_CREAT , 0660,1 );//robo
    sem_init( robosem, 1, 1 );//semctl( robosem, 0, SETVAL, 1);
    sem_getvalue(robosem,&semval);
    printf( "Initial robosem: %d\n", semval );

    sem_t* minisem = sem_open( "/minisem", O_CREAT , 0660,maxcount );//mini
    sem_init( minisem, 1, maxcount );//semctl( minisem, 0, SETVAL, maxcount);
    sem_getvalue(minisem,&semval);
    printf( "Initial minisem: %d\n", semval );

    sem_t *maxisem = sem_open( "/maxisem", O_CREAT , 0660,0 );//maxi
    sem_init( maxisem, 1, 0 );//semctl( maxisem, 0, SETVAL, 0);
    sem_getvalue(maxisem,&semval);
    printf( "Initial maxisem: %d\n", semval );



    	int sharmem_key = getuid();
	int sharmem_id = shmget(sharmem_key,maxcount*sizeof(int),0644|IPC_CREAT);
	int *sharmem=(int*)shmat(sharmem_id,NULL,0);

  	int sharmem_index_key = getuid()+1;
	int sharmem_index_id = shmget(sharmem_index_key,sizeof(int),0644|IPC_CREAT);
	int *sharmem_index=(int*)shmat(sharmem_index_id,NULL,0);

	for(int j = maxcount;j>=0;j--)
	{
		
		sharmem[j] = '\0';
		
              		
	}
	*sharmem_index=0;


   
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
      
        while(tofinish)
        {
            
              	
 		for(int j = maxcount;j>0;j--)
		{
			//printf("Will wait at MAXISEM %d\n",j);
			sem_wait(maxisem);//semop( maxisem, &semfulldown, 1);
			sem_getvalue(maxisem,&semval);
			//printf("Maxisem: %d\n",semval);
		}
		//printf("Will wait at ROBOSEM \n");
		sem_wait(robosem);//semop( robosem, &semdown, 1);
		
		sem_getvalue(robosem,&semval);
		//printf("robosem: %d\n",semval);
		for(int j = maxcount-1;j>=0;j--)
		{
			
		 	printf( "I am Parent %d deleting item %d from index %d\t", getpid(),sharmem[j],*sharmem_index );
			sharmem[j] = '\0';
			(*sharmem_index)--;
			WriteArray(sharmem,maxcount);
			
               		
		}
		sem_post(robosem);//	semop(robosem,&semup,1);		             
		sem_getvalue(robosem,&semval);
		//printf("robosem: %d\n",semval);
		
		for(int j = maxcount;j>0;j--)
		{
			
               	sem_post(minisem);//semop(minisem,&semfullup,1);	
			sem_getvalue(minisem,&semval);
			//printf("Minisem: %d\n",semval);		
		
		}
		
             
		

           

            while((wpid= waitpid(-1, &status ,WNOHANG)  )>0)
            {
			
               printf( "End process %d state %d,to finish: %d\n", wpid,WEXITSTATUS(status) ,tofinish);
                
                
            }



        }    

		sem_unlink ("/robosem");
		sem_unlink ("/minisem");
		sem_unlink ("/maxisem");
	
        

        

        printf( "Parent finish...\n" );

    }

    else

    { 

	srand(getpid());
     while(true)
     
     {
     	int item = (rand()%maxval)+1;
	sem_wait( minisem);//semop(minisem, &semdown,1); 
	sem_wait(robosem);//semop( robosem, &semdown, 1);
	sharmem[*sharmem_index] = item; 
	(*sharmem_index)++;
	printf("Robot %d adding number %d into slot %d:\t",i,item,*sharmem_index);
	WriteArray(sharmem,maxcount);
	
		
	
	
	sem_post(robosem);//semop(robosem,&semup,1);
	sem_post(maxisem);//semop(maxisem,&semup,1);
	usleep(100000);
     
     }
        

        exit(0);

        

    }
	shmctl(sharmem_id, IPC_RMID, NULL);
	shmctl(sharmem_index_id, IPC_RMID, NULL);
	
    printf( "Common code...\n" ); 

    


}

