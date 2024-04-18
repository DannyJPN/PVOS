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

void readline_Noblock(int fd,void*buf,int len,int timeout_ms)
{

fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK );
    while ( 1 )
    {
        
        int ret = read( fd, buf, 1);
        if ( ret == 0 ) break;
        if ( ret < 0 )
        {
            if ( errno == EAGAIN ) { usleep( timeout_ms * 1000); continue; }
            break;
        }

write( fd+1, buf, ret );
	for(int i = 0;i< len;i++)
	{
		if( ((char*)buf)[i] == '\n')
		{return;}
	}
    }
        

}



void readline_select(int fd,void*buf,int len,int timeout_ms)
{


    while ( 1 )
    {
        fd_set target;
        FD_ZERO( &target );
        FD_SET( fd, &target );
        int max = fd+1;
        
        int ret = select( max + 1, &target, NULL, NULL, NULL );
        if ( ret < 0 ) break;

        if ( FD_ISSET( fd ,&target ) )
        {
            
            ret = read( fd, buf, len );
            if ( ret <= 0 ) break;
            write( fd+1, buf, ret );
for(int i = 0;i< len;i++)
	{
		if( ((char*)buf)[i] == '\n')
		{return;}
	}
	usleep(timeout_ms*1000);
        }

    }


}
void readline_poll(int fd,void*buf,int len,int timeout_ms)
{

while ( 1 )
    {
struct pollfd target;
target.fd=fd;
target.events= POLLIN;
        int ret = poll( &target, 2, -1 );
        if ( target.revents & POLLIN )
        {
           
            ret = read( fd, buf, len );
            if ( ret <= 0 ) break;
            write( fd+1, buf, ret );
        }
for(int i = 0;i< len;i++)
	{
		if( ((char*)buf)[i] == '\n')
		{return;}
	}
        usleep(timeout_ms*1000);
    }

}
int main()
{
int len = 256;
char*buf[len];
int ti = 10;


// readline_Noblock(0,buf,len,ti);
//readline_select(0,buf,len,ti);
readline_poll(0,buf,len,ti);
}

