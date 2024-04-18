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
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

int main( int t_argn, char **t_arg )
{
    if ( t_argn <= 1 )
    {
        printf( "Use: %s path\n", t_arg[ 0 ] );
        return 1;
    }

    unlink( t_arg[ 1 ] );

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un saun;
    saun.sun_family = AF_UNIX;
    strcpy( saun.sun_path, t_arg[ 1 ] );

    bind( sock, ( sockaddr * ) &saun, sizeof( saun ) );

    listen( sock, 10 );

    struct sockaddr_un newsaun;
    socklen_t socklen = sizeof( newsaun );
    int newsock = accept( sock, ( sockaddr * ) &newsaun, &socklen );

    char buf[ 128 ];
    int ret = read( newsock, buf, sizeof( buf ) );

    if ( ret > 0 )
        write( 1, buf, ret );

    close( newsock );
    close( sock );

}
