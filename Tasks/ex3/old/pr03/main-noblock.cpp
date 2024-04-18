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

int main()
{
    fcntl( 0, F_SETFL, fcntl( 0, F_GETFL ) | O_NONBLOCK );
    while ( 1 )
    {
        char buf[ 128 ];
        int ret = read( 0, buf, sizeof( buf ) );
        if ( ret == 0 ) break;
        if ( ret < 0 )
        {
            if ( errno == EAGAIN ) { usleep( 1000 ); continue; }
            break;
        }
        write( 1, buf, ret );
    }
}

