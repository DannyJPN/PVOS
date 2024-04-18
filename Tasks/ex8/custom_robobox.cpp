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
#include <sys/msg.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>

#define ROBO 1
#define MAXI 2
#define MINI 3

#define MsgSize ( sizeof( Message ) )
/*
#define ErrorCheck(value) { DetectError(value,__LINE__); } 
void DetectError(int result,int linenum)
{
	if(result <0)
	{
		printf("%d\t%s\n",linenum,strerror(errno));
		
	}
	
}*/


int mutex=-1;
 
int empty = -1; 
int full = -1;



sembuf up = { 0, 1, 0 };
sembuf down = { 0, -1, 0 };

void ErrorCheck(int result)
{
	if(result <0)
	{
		printf("%s\n",strerror(errno));
		
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

struct Message
{
    long type;
	int pos;
};
struct MsQueue
{
	char* name;
	Message* queue;
	int id;
	size_t pos;
	size_t maxcount;
	size_t messize;
};

MsQueue mqopen(char* name,size_t maxcount,size_t messize)
{
	MsQueue msq;
    	int sharmem_key = rand()%getuid();
	int sharmem_id = shmget(sharmem_key,maxcount*messize,0644|IPC_CREAT);
	Message *sharmem=(Message*)shmat(sharmem_id,NULL,0);
	msq.queue = sharmem;
	msq.name = name;
	msq.id = sharmem_id;
	msq.pos = 0;
	msq.maxcount = maxcount;
	msq.messize = messize;
	Message clear;
	clear.type=0;
	clear.pos=0;
	for(unsigned int i = 0;i<maxcount;i++)
	{
		msq.queue[i] = clear;
	}
	return msq;
}

int mqunlink(MsQueue msq)
{
	return shmctl(msq.id, IPC_RMID, NULL);
}

int mqsend(MsQueue* msq,Message mes)
{
	semop(empty, &down, 1 );
   	semop(mutex,&down,1);

	if(msq->pos < msq->maxcount)
	
	{
		msq->queue[msq->pos++] = mes;
	 printf("%d Inserting pos %d\n",getpid(),mes.pos);
 
		
		
	}
	
	semop(mutex,&up,1); 
	semop( full, &up, 1 );
  
  
  
  return 1;
}

int mqreceive(MsQueue* msq,Message* mes)
{
 	semop(full, &down, 1 );
   	semop(mutex,&down,1);

	if(msq->pos >0 )
	{
		mes = &msq->queue[msq->pos--] ;
		printf("%d Returning pos %d\n",getpid(),mes->pos);
	}
	semop(mutex,&up,1); 
	semop( empty, &up, 1 );
  


return 1;
}





int main()
{
    int i;
    int pid= -1;
    int maxcount = 10;
    int processcount = 6;
    int maxval = 25;
    //char semval=0;
    int iterations = 1000000;
    int sharmem_index=0;
	mutex = semget( 0xcafe, 1, 0600 | IPC_CREAT );
	empty = semget( 0xcaff, 1, 0600 | IPC_CREAT );
	full = semget( 0xcaf0, 1, 0600 | IPC_CREAT );

 
    semctl( mutex, 0, SETVAL, 1 );
    semctl( empty, 0, SETVAL, maxcount*10);
    semctl( full, 0, SETVAL, 0 );

  
  
   MsQueue roboqueue = mqopen( (char*)"roboqueue", maxcount*10,MsgSize);
   MsQueue miniqueue = mqopen( (char*)"miniqueue", maxcount*10,MsgSize);
   MsQueue maxiqueue = mqopen( (char*)"maxiqueue", maxcount*10,MsgSize);
   
   	

	Message robomsg;
	robomsg.type = ROBO;
        robomsg.pos = sharmem_index;

	Message minmsg;
	minmsg.type = MINI;
        minmsg.pos = sharmem_index;
		
	ErrorCheck( mqsend( &roboqueue, robomsg ));//ErrorCheck((msgsnd( msgid, &robomsg, MsgSize, 0 )));
    

	for(int i = 0;i<maxcount;i++)
	{ ErrorCheck( mqsend( &miniqueue,minmsg ));}

  
   



    	int sharmem_key = getuid();
	int sharmem_id = shmget(sharmem_key,maxcount*sizeof(int),0644|IPC_CREAT);
	int *sharmem=(int*)shmat(sharmem_id,NULL,0);

  	

	for(int j = maxcount;j>=0;j--)
	{
		
		sharmem[j] = '\0';
		
              		
	}
	


   
    for ( i = 0; i < processcount; i++ )
    {
        pid = fork();
        if ( pid == 0 ) break;
    }
  //  mq_attr mqatr;
   // mq_getattr( roboqueue, &mqatr );
   // printf( "Fronta %ld %ld \n", mqatr.mq_maxmsg, mqatr.mq_msgsize );

    if ( pid )
    { 
        int  status=-1;
        printf( "I am parent with PID %d\n", getpid() );
        int wpid = -1;
        int tofinish = processcount;
	Message rmsg;rmsg.pos=0;
	Message smsg;
	//char buf[ mqatr.mq_msgsize ];
	//unsigned prio;
        while(tofinish)
        {
            
              	
 		for(int j = maxcount;j>0;j--)
		{
			
			ErrorCheck( mqreceive( &maxiqueue, &rmsg ));//ErrorCheck( msgrcv( msgid, &rmsg, MsgSize,MAXI, 0 ));//read(maxisem[0],&semval,sizeof(semval));
			
		}
		ErrorCheck( mqreceive( &roboqueue,&rmsg ));
		
			//		read(robosem[0],&semval,sizeof(semval));
		sharmem_index = rmsg.pos;
		printf("Parent %d Sharmem %d\n",getpid(),sharmem_index);
		for(int j = maxcount-1;j>=0;j--)
		{
			
		 	printf( "I am Parent %d deleting item %d from index %d\t", getpid(),sharmem[j],sharmem_index );
			sharmem[j] = '\0';
			sharmem_index--;
			WriteArray(sharmem,maxcount);
			usleep(100000);
               		
		}
		smsg.type = ROBO;
		smsg.pos=sharmem_index;
		
		ErrorCheck( mqsend( &roboqueue, smsg ));		//ErrorCheck((msgsnd( robomsgid, &smsg, MsgSize, 0 )));//write(robosem[1],&robovalue,sizeof(robovalue));
		for(int j = maxcount;j>0;j--)
		{
			smsg.type = MINI;
			smsg.pos = sharmem_index;
			ErrorCheck( mqsend( &miniqueue, smsg));//ErrorCheck((msgsnd( minimsgid, &smsg, MsgSize, 0 ))); //write(minisem[1],&minivalue,sizeof(minivalue)); 
		}
		
             
		

           

            while((wpid= waitpid(-1, &status ,WNOHANG)  )>0)
            {
			
               printf( "End process %d state %d,to finish: %d\n", wpid,WEXITSTATUS(status) ,tofinish);
                
                
            }



        }    

		
        printf( "Parent finish...\n" );

    }

    else

    { 
    	Message rmsg;rmsg.pos=0;
	Message smsg;
	srand(getpid());
     while(iterations--)
     	
     {
     	int item = (rand()%maxval)+1;
		ErrorCheck( mqreceive( &miniqueue, &rmsg ));//ErrorCheck( msgrcv( msgid, &rmsg, MsgSize,MINI, 0 ));//read(minisem[0],&semval,sizeof(semval));
		ErrorCheck( mqreceive( &roboqueue, &rmsg ));//read(robosem[0],&semval,sizeof(semval));//sem_wait(robosem);//semop( robosem, &semdown, 1);
	
		sharmem_index = rmsg.pos;
		sharmem[sharmem_index] = item; 
		printf("Robot %d(%d) Sharmem %d\n",i,getpid(),sharmem_index);
		printf("Robot %d(%d) adding number %d into slot %d:\t",i,getpid(),item,sharmem_index);
		WriteArray(sharmem,maxcount);
	
		
		sharmem_index++;
		smsg.type = ROBO;
		smsg.pos = sharmem_index;
		ErrorCheck( mqsend( &roboqueue, smsg ));//ErrorCheck((msgsnd( msgid, &smsg, MsgSize, 0 )));
		smsg.type = MAXI;
		smsg.pos = sharmem_index;
		ErrorCheck( mqsend( &maxiqueue, smsg ));			//write(maxisem[1],&maxivalue,sizeof(maxivalue));//sem_post(maxisem);//semop(maxisem,&semup,1);
		usleep(100000);
     
     }
        

        exit(0);

        

    }
	shmctl(sharmem_id, IPC_RMID, NULL);
	//msgctl(msgid, IPC_RMID, nullptr);
	 mqunlink( roboqueue);
	 mqunlink( miniqueue );
	 mqunlink( maxiqueue );
	
    printf( "Common code...\n" ); 

    


}

