
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


//toto vypadá že to funguje



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

    int robosem = semget( 0x7626, 1, IPC_CREAT | 0660 );//robo

    semctl( robosem, 0, SETVAL, 1);
	printf( "Initial robosem: %d\n", semctl(robosem,0,GETVAL,NULL) );
    int countsem = semget( 0x6294, 1, IPC_CREAT | 0660 );//maxi

    semctl( countsem, 0, SETVAL, 0);
	printf( "Initial countsem: %d\n", semctl(countsem,0,GETVAL,NULL) );

    int sharmem_key = getuid();

	int sharmem_id = shmget(sharmem_key,maxcount*sizeof(int),0644|IPC_CREAT);

	int *sharmem=(int*)shmat(sharmem_id,NULL,0);


    sembuf roboup = { 0, 1, 0 }; 

    sembuf robodown = { 0, -1, 0 };
    sembuf countup = { 0, 1, 0 }; 

    sembuf countdown = { 0, -10, 0 };

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
                semop( countsem, &countdown, 1);
				//printf( "counter decrement: %d\n", semctl(countsem,0,GETVAL,NULL) );
                printf( "I am Parent %d changing box\n", getpid() );
				//printf( "Parent will enable robots: %d\n", semctl(robosem,0,GETVAL,NULL) );
				for(int j = 0;j<maxcount;j++)
				{
					sharmem[j] = '\0';
					
				}
                semop(robosem,&roboup,1);
				printf( "Box changed: %d\n", semctl(robosem,0,GETVAL,NULL) );


            
/*
            int currentpos = maxcount - actualval;

            WriteArray(sharmem,currentpos+1);*/

			//printf("blankride\n");


            while((wpid= waitpid(-1, &status ,WNOHANG)  )>0)

            {
				tofinish--;
                printf( "End process %d state %d,to finish: %d\n", wpid,WEXITSTATUS(status) ,tofinish);

                

                

            }



        }    

		semctl (robosem, 0, IPC_RMID);
		semctl (countsem, 0, IPC_RMID);
		

        

        

        printf( "Parent finish...\n" );

    }

    else

    { 

        for(int k = 0;k>-1;k++)

        {

            
			//printf( "Robot %d will start work: %d\n",i, semctl(robosem,0,GETVAL,NULL) );
            semop(robosem,&robodown,1);//critical section insert start
			printf( "I am Child %d with PID %d performing action %d\n", i,getpid(),k );
			//printf( "Robot %d can work: %d\n", i,semctl(robosem,0,GETVAL,NULL) );
            int actualval = semctl(countsem,0,GETVAL,NULL);//what is the current value of the counter

           // int currentpos = maxcount - actualval;//calculate where to insert. 
			int currentpos = actualval;
            

            

            sharmem[currentpos] = getpid();//insert itself
			WriteArray(sharmem,currentpos+1);
			//printf( "counter will increment: %d\n", semctl(countsem,0,GETVAL,NULL) );
            semop(countsem,&countup,1);//counter increment
			//printf( "counter increment: %d\n", semctl(countsem,0,GETVAL,NULL) );
            if(currentpos < maxcount-1 )

            {    
			//printf( "Robot %d will stop work: %d\n", i,semctl(robosem,0,GETVAL,NULL) );
            semop(robosem,&roboup,1);//critical section insert end
			//printf( "Robot %d cannot work: %d\n",i, semctl(robosem,0,GETVAL,NULL) );
            }

            

        }

        

        exit(0);

        

    }

    printf( "Common code...\n" ); 

    


}

