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
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <climits>
#include <mqueue.h>
#define rmessage_len ( sizeof( rmsg ) - sizeof( long ) )
#define smessage_len ( sizeof( smsg ) - sizeof( long ) )

#define ErrorCheck(value){    DetectError(value,__LINE__);           } 
#define BOXFULL 1
#define CANWORK 2



void DetectError(int result,int linenum)
{
	if(result <0)
	{
		printf("%d\t%s\n",linenum,strerror(errno));
		
	}
	
}

struct message
{
    long type;
    int value;
   
};


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
 
    int maxcount = 10;
    int sharmem_key = getuid();
	int sharmem_id = shmget(sharmem_key,maxcount*sizeof(int),0644|IPC_CREAT|IPC_EXCL);
    ErrorCheck(sharmem_id);
    if(sharmem_id <0 && errno == EEXIST)
    {
        sharmem_id=shmget(sharmem_key,maxcount*sizeof(int),0644);
    }
	int *sharmem=(int*)shmat(sharmem_id,NULL,0);

    
  
	int currentpos=0;

	int  fullboxmqid = mq_open( "/fullbox", O_CREAT | O_WRONLY|O_EXCL, 0660, nullptr );
    ErrorCheck(fullboxmqid);
    if(fullboxmqid <0 && errno == EEXIST)
    {
        fullboxmqid = mq_open( "/fullbox", O_CREAT | O_WRONLY, 0660, nullptr );
        
    }
	int  workablemqid = mq_open( "/workable", O_CREAT | O_RDWR|O_EXCL, 0660, nullptr );
    ErrorCheck(workablemqid);
    if(workablemqid <0 && errno == EEXIST)
    {
        workablemqid = mq_open( "/workable", O_CREAT | O_RDWR, 0660, nullptr );
        
    }
	mq_attr mqattr;
    mq_getattr( fullboxmqid, &mqattr );
	

    for(int k = 0;k<INT_MAX;k++)
    {   
		//message rmsg;
        //ErrorCheck(msgrcv(msgid,&rmsg,rmessage_len,CANWORK,0));
		char msgcontent[mqattr.mq_msgsize ];
        message* rmsg=(message*)msgcontent;
        //ErrorCheck(msgrcv(msgid,&rmsg,rmessage_len,BOXFULL,0));
		unsigned pri;
		ErrorCheck(mq_receive(workablemqid,msgcontent,mqattr.mq_msgsize,&pri));
		
		
		currentpos = rmsg->value;
        sharmem[currentpos] = getpid();//insert itself
        WriteArray(sharmem,currentpos+1);
        
		printf("Inserting at curpos %d\n",currentpos);
        if(currentpos < maxcount -1)
        {
            
            //message smsg;
            //smsg.type = CANWORK;
			//smsg.value = currentpos+1;
            //ErrorCheck(msgsnd( msgid, &smsg, smessage_len, 0 ));
            
			
			message smsg;
			smsg.type = CANWORK;
			smsg.value = currentpos+1;
			ErrorCheck(mq_send(workablemqid,(const char*)&smsg,sizeof(smsg),smsg.type));
			//printf("Curpos %d lower than max %d\n",currentpos,maxcount);
			 //usleep(2000000);
        }
        else
        {
            //message smsg;
			//smsg.type = BOXFULL;
			//smsg.value = currentpos+1;
            //ErrorCheck(msgsnd( msgid, &smsg, smessage_len, 0 ));
			
			message smsg;
			smsg.type = BOXFULL;
			smsg.value = currentpos+1;
			ErrorCheck(mq_send(fullboxmqid,(const char*)&smsg,sizeof(smsg),smsg.type));
			//printf("BOX FULL\n");
			// usleep(2000000);
            
        }
        //currentpos++;
        usleep(20000);
	}

        

      

        

    
shmctl(sharmem_id,IPC_RMID,0);



    


}

