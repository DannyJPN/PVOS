#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>




struct innerreader
{
	int size;
	char* buffer;
	int index;
	
};


int readlinenew(int fd,void*buf,int len,int timeout_ms,innerreader*inread)
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
            if ( readyfds < 0 ) break;
            while(readyfds == 0)
            {
                printf("timeout expired\n");
                return -2;
            }
            if ( FD_ISSET( fd ,&target ) )
            {
                int ret	= read( fd, &inread->buffer[inread->index], (inread->size)-(inread->index));
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
                    if(inread->buffer[i] == '\n')
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
            if ( readyfds < 0 ) break;
			
    
            if ( FD_ISSET( fd ,&target ) )
            {
                int ret	= read( fd, &inread->buffer[inread->index], (inread->size)-(inread->index));
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
    int len = 50;
    char buf[len]={'\0'};
    int ti = 20000;
    
    innerreader inread;
    inread.size = 20;
    inread.buffer = new char[inread.size];
    inread.index=0;
    
    int readchars=readlinenew(0,buf,len,ti,&inread);
    printf("GOT: (%s) with len %d\n",buf,readchars);
    //printf("(%s)\n",buf);
    //printf("Read bytes(%d)\n",readchars);
    readchars=readlinenew(0,buf,len,ti,&inread);
    printf("GOT: (%s) with len %d\n",buf,readchars);
    //printf("(%s)\n",buf);
    //printf("Read bytes(%d)\n",readchars);
    
    
    
    
    delete inread.buffer;
    return 0;
}

