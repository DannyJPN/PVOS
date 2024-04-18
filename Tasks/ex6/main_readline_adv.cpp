#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/time.h>
 

struct innerreader
{
	int fd;
	int size;
	char* buffer;
	int index;
	
};

int readline_select(innerreader*inread,char*inbuf,int *len,int timeout_us)
{
	
	
	for(int i = 0;i<inread->size;i++)
	{
		inread->buffer[i]=0;
		
	}
	struct timeval tim;
	tim.tv_sec = 0;
	tim.tv_usec = timeout_us;

      
	
    	while ( 1 )
    	{
        	fd_set target;
	        FD_ZERO( &target );
	        FD_SET( inread->fd, &target );
	        int max = inread->fd+1;
       
	   
	   
	   
	   
	   
	   
	   
	   
	   
	   
	   
	        int ret = select( max + 1, &target, NULL, NULL, &tim);
        	
			if(ret <=0){*len=inread->index;return ret;}
	        if ( FD_ISSET( inread->fd ,&target ) )
	        {
            
	            	ret = read( inread->fd, &inread->buffer[inread->index], 1 );
        	    	if ( ret < 0 ) {*len=inread->index;return ret;}
			
					if(inread->index>=inread->size || inread->buffer[inread->index] == '\n'||ret==0)
					{
						*len=inread->index;
						return 0;
					}
					inread->index++;
	
	        }

    }


}
int readline_select_adv(innerreader*inread,char*inbuf,int *len,int timeout_us)
{
	
	
	for(int i = 0;i<inread->size;i++)
	{
		inread->buffer[i]=0;
		
	}
	struct timeval tim;
	tim.tv_sec = 0;
	tim.tv_usec = timeout_us;

      
	
    	while ( 1 )
    	{
        	fd_set target;
	        FD_ZERO( &target );
	        FD_SET( inread->fd, &target );
	        int max = inread->fd+1;
       
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
                
                memcpy((char*)inbuf ,inread->buffer,newline+1);
                ((char*)inbuf)[newline+1] = '\0';
                //printf("Copying %d bytes from %s\n",newline,inread->buffer);
                memmove(inread->buffer,&inread->buffer[newline+1],(inread->index)-(newline+1));
                //printf("Moving %d bytes,result: (%s)\n",(inread->index)-newline,inread->buffer);
                inread->index -= (newline+1);
                //printf("New ININ: %d\n",inread->index);
                *len=newline+1;
                return 0;
            }	   
	   
	   
	   
	   
	   
	   
	   
	   
	   
	   
	        int ret = select( max , &target, NULL, NULL, &tim);
        	
			if(ret <0){*len=inread->index;return ret;}
			if(ret == 0)
			{
					memcpy(inbuf ,inread->buffer,inread->index);
						((char*)inbuf)[inread->index] = '\0';
						//inread->index+=strlen(inbuf);
						*len=inread->index+6;
						
				return ret;
			}
			
	        if ( FD_ISSET( inread->fd ,&target ) )
	        {
            
	            	ret	= read( inread->fd, &inread->buffer[inread->index], (inread->size)-(inread->index));
        	    	if ( ret < 0 ) {*len=inread->index;return ret;}
					if(ret==0)
					{
						memcpy(inbuf ,inread->buffer,inread->index);
						((char*)inbuf)[inread->index] = '\0';
						//inread->index+=strlen(inbuf);
						*len=inread->index;
						
						return 0;
						
					}
					if(inread->index>=*len || inread->buffer[inread->index] == '\n')
					{
						
						*len=inread->index;
						return 0;
					}
					inread->index+=ret;
	
	        }

    }


}

int main()
{
int len = 256;
char buf[len];
int ti = 10*1000000;
    innerreader inread;
	inread.fd = 0;
    inread.size = 256;
    inread.buffer = new char[inread.size];
    inread.index=0;

// 
printf("Your text:\n");
int ret=readline_select_adv(&inread,buf,&len,ti);

printf("\nYour response: (%s) (%d)\n",inread.buffer	,len);
}

