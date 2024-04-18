#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/select.h>

int readline( int fd, char *buf, int len, int mstout )
{
    timeval tout = { mstout / 1000, mstout % 1000 };

    int l = 0;
    while ( l < len )
    {
        fd_set rset;
        FD_ZERO( &rset );
        FD_SET( fd, &rset );
        int r = select( fd + 1, &rset, nullptr, nullptr, &tout );

        if ( r == 0 )
        {
            errno = ETIMEDOUT;
            return -1;
        }
        if ( r < 0 ) 
        {
            return -1;
        }

        r = read( fd, buf + l, 1 );
        if ( r < 0 ) 
        {
            return -1;
        }
        if ( r == 0 )
        {
            if ( l++ ) break;
            return 0;
        }
        if ( buf[ l++ ] == '\n' ) break;
    }
    buf[ l ] = 0; 
    return l;
}

int main()
{

    char buf[ 128 ];

    while ( 1 )
    {
        int r = readline( 0, buf, sizeof( buf ), 5000 );

        if ( r < 0 )
        {
            printf( "errno %d\n", errno );
            break;
        }
        if ( r == 0 )
        {
            printf( "end of file\n" );
            break;
        }
        if ( r > 0 )
        {
            printf( "line: '%s'\n", buf );
        }
    }
}
