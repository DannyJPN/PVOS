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
    int itemcount=100000;
    int robosem = semget( 0x7626, 1, IPC_CREAT | 0660 );//robo
    semctl( robosem, 0, SETVAL, 1);
    printf( "Initial robosem: %d\n", semctl(robosem,0,GETVAL,NULL) );

    int minisem = semget( 0x6294, 1, IPC_CREAT | 0660 );//mini
    semctl( minisem, 0, SETVAL, maxcount);
    printf( "Initial minisem: %d\n", semctl(minisem,0,GETVAL,NULL) );

    int maxisem = semget( 0x2596, 1, IPC_CREAT | 0660 );//maxi
    semctl( maxisem, 0, SETVAL, 0);
    printf( "Initial maxisem: %d\n", semctl(maxisem,0,GETVAL,NULL) );



    	int sharmem_key = getuid();
	int sharmem_id = shmget(sharmem_key,maxcount*sizeof(int),0644|IPC_CREAT);
	int *sharmem=(int*)shmat(sharmem_id,NULL,0);

  	int sharmem_index_key = getuid()+1;
	int sharmem_index_id = shmget(sharmem_index_key,sizeof(int),0644|IPC_CREAT);
	int *sharmem_index=(int*)shmat(sharmem_index_id,NULL,0);

    sembuf semup = { 0, 1, 0 }; 
    sembuf semdown = { 0, -1, 0 };
    sembuf semfullup = { 0, 10, 0 }; 
    sembuf semfulldown = { 0, -10, 0 };
    
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
            
              	
               printf( "I am Parent %d changing box\n", getpid() );
		semop( maxisem, &semfulldown, 1);
		semop( robosem, &semdown, 1);
		
		for(int j = 0;j<maxcount;j++)
		{
			
			sharmem[j] = '\0';
			
		}
		*sharmem_index=0;
		printf( "Box changed: %d\n", semctl(robosem,0,GETVAL,NULL) );
              	semop(robosem,&semup,1);
               semop(minisem,&semfullup,1);
		

           

            while((wpid= waitpid(-1, &status ,WNOHANG)  )>0)
            {
			
               printf( "End process %d state %d,to finish: %d\n", wpid,WEXITSTATUS(status) ,tofinish);
                
                
            }



        }    

		semctl (robosem, 0, IPC_RMID);
		semctl (minisem, 0, IPC_RMID);
		semctl (maxisem, 0, IPC_RMID);

        

        

        printf( "Parent finish...\n" );

    }

    else

    { 

	srand(getpid());
     while(itemcount--)
     
     {
     	int item = (rand()%maxval)+1;
	semop(minisem, &semdown,1); 
	semop( robosem, &semdown, 1);
	sharmem[*sharmem_index] = item; 
	(*sharmem_index)++;
	printf("Robot %d adding number %d into slot %d:\t",i,item,*sharmem_index);
	WriteArray(sharmem,maxcount);
	
		
	
	
	semop(robosem,&semup,1);
	semop(maxisem,&semup,1);
	usleep(100000);
     
     }
        

        exit(0);

        

    }
	shmctl(sharmem_id, IPC_RMID, NULL);
	shmctl(sharmem_index_id, IPC_RMID, NULL);
	
    printf( "Common code...\n" ); 

    


}

