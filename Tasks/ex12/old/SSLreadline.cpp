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

#include <openssl/rsa.h>      
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>



/*V tomto souboru jsem to netestoval,je to jen vytažený kód*/
int readline_SSL(SSL *ssls,char*buf,int len,int timeout_ms)
{
int fd = SSL_get_fd();
//fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK );
struct timeval delay;
delay.tv_sec = timeout_ms/1000;
delay.tv_usec = (timeout_ms%1000)*1000;

setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &delay, sizeof(delay)); 
    int readchars = 0;
    while ( 1 )
    {
        
        int ret = SSL_read( ssls, buf+readchars, 1);
        if ( ret == 0 ) break;
        if ( ret < 0 )
        {
            if ( errno == EAGAIN ) {  continue; }
            break;
        }
        if(buf[readchars] == '\n')
        {
            break;
        }
        if(readchars >= len-1)
        {
            break
        }


    }
    buf[readchars]=0;
    return readchars;

}


int main()
{
int len = 256;
char*buf[len];
int ti = 10;


// readline_Noblock(0,buf,len,ti);
//readline_select(0,buf,len,ti);
//readline_SSL(0,buf,len,ti);
}

