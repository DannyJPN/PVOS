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

   int robomsgfd = mq_open( "/roboqueue", O_CREAT | O_RDWR, 0640, nullptr );
   int minimsgfd = mq_open( "/miniqueue", O_CREAT | O_RDWR, 0640, nullptr );
   int maximsgfd = mq_open( "/maxiqueue", O_CREAT | O_RDWR, 0640, nullptr );
   
	Message robomsg;
	robomsg.type = ROBO;
        robomsg.pos = sharmem_index;

	Message minmsg;
	minmsg.type = MINI;
        minmsg.pos = sharmem_index;
		
	ErrorCheck( mq_send( robomsgfd, ( char * ) &robomsg, MsgSize, 0 ));//ErrorCheck((msgsnd( msgid, &robomsg, MsgSize, 0 )));
    

	for(int i = 0;i<maxcount;i++)
	{ ErrorCheck( mq_send( minimsgfd, ( char * ) &minmsg, MsgSize, 0 ));}

  
   



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
    mq_attr mqatr;
    mq_getattr( robomsgfd, &mqatr );
    printf( "Fronta %ld %ld \n", mqatr.mq_maxmsg, mqatr.mq_msgsize );

    if ( pid )
    { 
        int  status=-1;
        printf( "I am parent with PID %d\n", getpid() );
        int wpid = -1;
        int tofinish = processcount;
	Message *rmsg;
	Message smsg;
	char buf[ mqatr.mq_msgsize ];
	unsigned prio;
        while(tofinish)
        {
            
              	
 		for(int j = maxcount;j>0;j--)
		{
			
			ErrorCheck( mq_receive( maximsgfd, buf, sizeof( buf ), &prio ));//ErrorCheck( msgrcv( msgid, &rmsg, MsgSize,MAXI, 0 ));//read(maxisem[0],&semval,sizeof(semval));
			
		}
		ErrorCheck( mq_receive( robomsgfd, buf, sizeof( buf ), &prio ));
		rmsg=(Message*)buf;
			//		read(robosem[0],&semval,sizeof(semval));
		sharmem_index = rmsg->pos;
		for(int j = maxcount-1;j>=0;j--)
		{
			
		 	printf( "I am Parent %d deleting item %d from index %d\t", getpid(),sharmem[j],sharmem_index );
			sharmem[j] = '\0';
			sharmem_index--;
			WriteArray(sharmem,maxcount);
			
               		
		}
		smsg.type = ROBO;
		smsg.pos=sharmem_index;
		
		ErrorCheck( mq_send( robomsgfd, ( char * ) &smsg, MsgSize, 0 ));		//ErrorCheck((msgsnd( robomsgid, &smsg, MsgSize, 0 )));//write(robosem[1],&robovalue,sizeof(robovalue));
		for(int j = maxcount;j>0;j--)
		{
			smsg.type = MINI;
			smsg.pos = sharmem_index;
			ErrorCheck( mq_send( minimsgfd, ( char * ) &smsg, MsgSize, 0 ));//ErrorCheck((msgsnd( minimsgid, &smsg, MsgSize, 0 ))); //write(minisem[1],&minivalue,sizeof(minivalue)); 
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
    	char buf[ mqatr.mq_msgsize ];
	unsigned prio;
	Message* rmsg;
	Message smsg;
	srand(getpid());
     while(iterations--)
     	
     {
     	int item = (rand()%maxval)+1;
		ErrorCheck( mq_receive( minimsgfd, buf, sizeof( buf ), &prio ));//ErrorCheck( msgrcv( msgid, &rmsg, MsgSize,MINI, 0 ));//read(minisem[0],&semval,sizeof(semval));
		ErrorCheck( mq_receive( robomsgfd, buf, sizeof( buf ), &prio ));//read(robosem[0],&semval,sizeof(semval));//sem_wait(robosem);//semop( robosem, &semdown, 1);
		rmsg=(Message*)buf;
		sharmem_index = rmsg->pos;
		sharmem[sharmem_index] = item; 
		
		printf("Robot %d adding number %d into slot %d:\t",i,item,sharmem_index);
		WriteArray(sharmem,maxcount);
	
		
		sharmem_index++;
		smsg.type = ROBO;
		smsg.pos = sharmem_index;
		ErrorCheck( mq_send( robomsgfd, ( char * ) &smsg, MsgSize, 0 ));//ErrorCheck((msgsnd( msgid, &smsg, MsgSize, 0 )));
		smsg.type = MAXI;
		smsg.pos = sharmem_index;
		ErrorCheck( mq_send( maximsgfd, ( char * ) &smsg, MsgSize, 0 ));			//write(maxisem[1],&maxivalue,sizeof(maxivalue));//sem_post(maxisem);//semop(maxisem,&semup,1);
		usleep(100000);
     
     }
        

        exit(0);

        

    }
	shmctl(sharmem_id, IPC_RMID, NULL);
	//msgctl(msgid, IPC_RMID, nullptr);
	 mq_unlink( "/roboqueue");
	 mq_unlink( "/miniqueue" );
	 mq_unlink( "/maxiqueue" );
	
    printf( "Common code...\n" ); 

    


}

