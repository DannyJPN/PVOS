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





int readline_select_adv(innerreader*inread,char*inbuf,int *len,int timeout_us)
{
	
	
	for(int i = 0;i<inread->size;i++)
	{
		inbuf[i]=0;
		
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
                for (int i =0;i<inread->index+1;i++)
                {
                    if(inread->buffer[i] == '\n' )
                    {
                        newline=i;
                      
                        break;
                        
                    }
                    
                }
      
                
            if(newline >=0)
            {
               // printf("Current: [%s]\n",inread->buffer);
                memcpy((char*)inbuf ,inread->buffer,newline+1);
                ((char*)inbuf)[newline+2] = '\0';
               // printf("Copying %d bytes from [%s]\ninbuf [%s]",newline+1,inread->buffer,inbuf);
                memmove(inread->buffer,&inread->buffer[newline+1],(inread->index)-(newline));
               // printf("Moving %d bytes,result: (%s)\n",(inread->index)-newline,inread->buffer);
                inread->index -= (newline+1);
                //printf("New ININ: %d\n",inread->index);
               
                *len=newline+1;
                return *len;
            }	   
	   
	   
	   
	   
	   
	   
	   
	   
	   
	   
	     int ret = select( max , &target, NULL, NULL, &tim);
        	
	     if(ret <0)
	     {
	     	//*len=inread->index;
	     	return ret;
	     }
	     if(ret == 0)
			{
				memcpy(inbuf ,inread->buffer,inread->index);
				((char*)inbuf)[inread->index+1] = '\0';
				
				*len=inread->index;
				errno=EAGAIN;
				return -2;
			}
			
	    if ( FD_ISSET( inread->fd ,&target ) )
	        {
            //100,5 95
				
	            ret	= read( inread->fd, &inread->buffer[inread->index], (inread->size)-(inread->index));
        	    if ( ret < 0 ) 
        	    {
	        	   	//*len=inread->index;
        	    	return ret;
        	    }
				if(ret==0)
				{
					memcpy(inbuf ,inread->buffer,inread->index);
					((char*)inbuf)[inread->index] = '\0';
					
					*len=inread->index;
				
					return ret;
				
				}
				inread->index+=ret;
			
			
				for (int i =0;i<inread->index+1;i++)
                {
                    if(inread->buffer[i] == '\n' )
                    {
                        newline=i;
                      
                        break;
                        
                    }
                    
                }
				if(newline >=0)
				{
				// printf("Current: [%s]\n",inread->buffer);
					memcpy((char*)inbuf ,inread->buffer,newline+1);
					((char*)inbuf)[newline+2] = '\0';
				// printf("Copying %d bytes from [%s]\ninbuf [%s]",newline+1,inread->buffer,inbuf);
					memmove(inread->buffer,&inread->buffer[newline+1],(inread->index)-(newline));
				// printf("Moving %d bytes,result: (%s)\n",(inread->index)-newline,inread->buffer);
					inread->index -= (newline+1);
					//printf("New ININ: %d\n",inread->index);
				
					*len=newline+1;
					return *len;
				}				
			
			
				
			
			
			
			
	
	        }
 
	        
	        

    }


}

int main()
{
int len = 256*256*4;
char buf[len];
int ti = 10*1000000;
    innerreader inread;
	inread.fd = 0;
    inread.size = 65536;
    inread.buffer = new char[inread.size];
    inread.index=0;

// 
//printf("Your text:\n");
//int ret=readline_select_adv(&inread,buf,&len,ti);

//printf("\nYour response: (%s) (%d) (%d)\n",inread.buffer	,len,ret);


int linenum=0;
char linenumbering[64];
	while ( 1 )
	{
	   len = 256*256*4;
	    int ret = readline_select_adv(&inread,buf,&len,ti);



	 // int ret= readlinenew(inread.fd,buf,len,ti,&inread);
	    if ( ret < 0 && errno == EAGAIN) {printf("\nTimeout\n");sleep(1); }


	    
		

		linenum++;
		sprintf(linenumbering,"%d \t",linenum);
		write(1,linenumbering,strlen(linenumbering));
		write( 1, buf, len );	
	 	if(ret==0)
		{	
			break;
		} 		


	}



}

