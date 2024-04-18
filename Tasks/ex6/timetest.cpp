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
 



int readline_select(int fd,void*buf,int len,int timeout_us)
{
	char*textbuf = ((char*)buf);
	
	for(int i = 0;i<len;i++)
	{
		textbuf[i]=0;
		
	}
	struct timeval tim;
	tim.tv_sec = 0;
	tim.tv_usec = timeout_us;
	int index=0;
	
    	while ( 1 )
    	{
        	fd_set target;
	        FD_ZERO( &target );
	        FD_SET( fd, &target );
	        int max = fd+1;
       
	        int ret = select( max + 1, &target, NULL, NULL, &tim);
        	if ( ret < 0 ) return ret;
		if(ret ==0){return index+1;}
	        if ( FD_ISSET( fd ,&target ) )
	        {
            
	            	ret = read( fd, &textbuf[index], 1 );
        	    	if ( ret < 0 ) return ret;
			if(ret ==0){return index+1;}
			if(index>=len || textbuf[index] == '\n')
			{
				return index+1;
			}
			index++;
	
	        }

    }


}

int main()
{
//int len = 256;
//char buf[len];
//int ti = 10*1000000;


// 
printf("Your text:\n");
//int ret=readline_select(0,buf,len,ti);

//printf("\nYour response: %s (%d)\n",buf,ret);

 struct timeval selecttimeout;
    selecttimeout.tv_sec = 000;
    selecttimeout.tv_usec = 3456898765;
printf("Sec %lu Usec %lu\n",selecttimeout.tv_sec,selecttimeout.tv_usec);
}

