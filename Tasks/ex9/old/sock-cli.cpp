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
#include <arpa/inet.h>

int main( int t_argn, char **t_arg )
{
    if ( t_argn <= 2 )
    {
        printf( "Use: %s ip port\n", t_arg[ 0 ] );
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sain;
    sain.sin_family = AF_INET;
    sain.sin_port = htons( atoi( t_arg[ 2 ] ) );
    inet_pton( AF_INET, t_arg[ 1 ], &sain.sin_addr.s_addr );

    connect( sock, ( sockaddr * ) &sain, sizeof( sain ) );

    char buf[ 128 ] = "Nazdar\n";
    int ret = write( sock, buf, strlen( buf ) );

    close( sock );

}
