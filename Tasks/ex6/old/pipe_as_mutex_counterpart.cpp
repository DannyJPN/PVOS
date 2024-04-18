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

struct innerreader
{
	int size;
	char* buffer;
	int index;
	
};


int readline(int fd,void*buf,int len,int timeout_ms,innerreader*inread)
{//2 541 
    struct timeval selecttimeout;
    selecttimeout.tv_sec = timeout_ms/1000;
    selecttimeout.tv_usec = (timeout_ms%1000)*1000;
    
    
    
    while ( 1 )
        {
            fd_set target;
            FD_ZERO( &target );
            FD_SET( fd, &target );
            int max = fd+1;
            int newline=-1;
                for (int i =0;i<inread->index;i++)
                {
                    if(inread->buffer[i] == '\n' )
                    {
                        newline=i;
                        break;
                        
                    }
                    
                }
      
                
            if(newline >=0)
            {
                
                memcpy((char*)buf ,inread->buffer,newline);
                ((char*)buf)[newline] = '\0';
                //printf("Copying %d bytes from %s\n",newline,inread->buffer);
                memmove(inread->buffer,&inread->buffer[newline+1],(inread->index)-(newline+1));
                //printf("Moving %d bytes,result: (%s)\n",(inread->index)-newline,inread->buffer);
                inread->index -= (newline+1);
                //printf("New ININ: %d\n",inread->index);
                
                return newline;
            }
            int readyfds = select( max + 1, &target, NULL, NULL, &selecttimeout );
			ErrorCheck(readyfds);
            if ( readyfds < 0 ) break;
            if(readyfds == 0)
            {
                printf("timeout expired\n");
                return -2;
            }
            if ( FD_ISSET( fd ,&target ) )
            {
                int ret	= read( fd, &inread->buffer[inread->index], (inread->size)-(inread->index));
				ErrorCheck(ret);
                inread->index+=ret;
                //printf("Ret:%d\t(%s)\n",ret,inread->buffer);
                //printf("ININ: %d\n",inread->index);
                if ( ret < 0 ) return -1;
                else if (ret==0)
                {
                    int l=inread->index;
                    memcpy((char*)buf ,inread->buffer,inread->index);
                    ((char*)buf)[inread->index] = '\0';
                    inread->index = 0;
                    return l;
                }
        
            }
    
        }
    
    return -1;
}





int main()
{   
    int len = 512;
    char res[len]={'\0'};
    int timeout = 20000;
    
    innerreader inread;
    inread.size = 128;
    inread.buffer = new char[inread.size];
    inread.index=0;
    char pipename[]="tunnel";

    char separator = ' ';
    int readb=INT_MAX;
  
	int node = mknod(pipename,S_IFIFO|0666,0);
	ErrorCheck(node);
	int pipefile = open( pipename, O_CREAT|O_RDONLY,0666 );
	ErrorCheck(pipefile);	

   /* int semid = semget( 0xface, 1, IPC_CREAT|IPC_EXCL | 0666 );
	//ErrorCheck(semid);
	if(semid>=0)
    {
		ErrorCheck(semctl( semid, 0, SETVAL, 1 ));
	}
	else if(semid<0 && errno == EEXIST)
	{
		semid = semget( 0xface, 1, IPC_CREAT| 0666 );
		ErrorCheck(semid);
	}
	
	
    sembuf up = { 0, 1, 0 }; 
    sembuf down = { 0, -1, 0 };*/
    int *readnums;
    while(1)
	{
		//ErrorCheck(semop( semid, &down, 1 ));
		
				

		
		
		
		readb = readline(pipefile,res,sizeof(res),timeout,&inread);
		ErrorCheck(readb);
        if(readb == 0)
        {
            break;
        }
        if(readb==-2)
        {
            printf("BORED\n");
        }
		int tokencount = 0;
		
		for(int i =0;i<readb;i++)
		{
			if(res[i] == separator)
			{
				tokencount++;
			}
			
		}
		if(tokencount<2){continue;}
		readnums= new int[tokencount-1];
		int sum = 0;
		int tokidx = 0;
		int insertpid = atoi(strtok(res," "));
		
		printf("%d:",insertpid);
		tokidx++;
		
		
		while(tokidx<tokencount)
		{
			
			int item = atoi(strtok(NULL," "));
			printf("%d ",item);
			readnums[tokidx-1] = item;
			
			sum+=item;
			tokidx++;
		}
		int readsum = atoi(strtok(NULL," "));
		
		
		
		printf("\nSum %d\n",sum);
		printf("Readsum %d\n",readsum);
		if(sum != readsum)
		{
			printf("COMPUTATION ERROR!!!\n");
			
		}
		
		delete[] readnums;
		//ErrorCheck(semop( semid, &up, 1 ));
		
		
		
		
		
		
		
		
		
        
		
	}

    ErrorCheck(close(pipefile));
    
    
    
    
    return 0;
}

