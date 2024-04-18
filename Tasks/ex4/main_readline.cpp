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
 
int readline_Noblock(int fd,void*buf,int len,int timeout_us)
{
	int index = 0;
	char*textbuf = ((char*)buf);
	struct timeval tim;
	tim.tv_sec = 0;
	tim.tv_usec = timeout_us;
	struct timeval sub;
	sub.tv_sec = 0;
	sub.tv_usec = 100000;
	
	for(int i = 0;i<len;i++)
	{
		textbuf[i]=0;
		
	}
	fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK );
    while ( 1 )
    {
        
        int ret = read( fd, &textbuf[index], 1 );
        if ( ret == 0 ) return ret;
        if ( ret < 0 )
        {
            if ( errno == EAGAIN )
			{
				usleep( sub.tv_usec );
				timersub(&tim,&sub,&tim);
				
			}
			else{    return ret;}
			
        }
		if(!(timerisset(&tim)))
		{
			return 0;
		}
		if(index>=len || textbuf[index] == '\n')
		{
			return index+1;
		}
		index++;

	
    }
        

}



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
        if ( ret <= 0 ) return ret;

        if ( FD_ISSET( fd ,&target ) )
        {
            
            ret = read( fd, &textbuf[index], 1 );
            if ( ret <= 0 ) return ret;
           
			if(index>=len || textbuf[index] == '\n')
			{
				return index+1;
			}
			index++;
	
        }

    }


}
int readline_poll(int fd,void*buf,int len,int timeout_us)
{
	char*textbuf = ((char*)buf);
	int index=0;
	for(int i = 0;i<len;i++)
	{
		textbuf[i]=0;
		
	}
	while ( 1 )
    {
		struct pollfd target;
		target.fd=fd;
		target.events= POLLIN;
        int ret = poll( &target, fd+1, timeout_us/1000 );
		if(ret<=0){return ret;}
        if ( target.revents & POLLIN )
        {
           
            ret = read( fd, &textbuf[index], 1 );
            if ( ret <= 0 ) return ret;
            if(index>=len || textbuf[index] == '\n')
			{return index+1;}
			index++;
        }

		
    }

}
int main()
{
int len = 256;
char buf[len];
int ti = 10*1000000;


// 
printf("Your text:\n");
//readline_select(0,buf,len,ti);
readline_Noblock(0,buf,len,ti);


//readline_poll(0,buf,len,ti);
printf("Your response: %s\n",buf);
}

