#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

int main( int t_argn, char **t_arg )
{
    if ( t_argn <= 1 )
    {
        printf( "Use: %s port\n", t_arg[ 0 ] );
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sain;
    sain.sin_family = AF_INET;
    sain.sin_port = htons( atoi( t_arg[ 1 ] ) );
    sain.sin_addr.s_addr = INADDR_ANY;

    bind( sock, ( sockaddr * ) &sain, sizeof( sain ) );

    listen( sock, 10 );

    struct sockaddr_in newsain;
    socklen_t socklen = sizeof( newsain );
    int newsock = accept( sock, ( sockaddr * ) &newsain, &socklen );

    char buf[ 128 ];
    int ret = read( newsock, buf, sizeof( buf ) );

    if ( ret > 0 )
        write( 1, buf, ret );

    close( newsock );
    close( sock );

}
