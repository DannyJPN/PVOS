#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

int main( int t_argn, char **t_arg )
{
    if ( t_argn <= 1 )
    {
        printf( "Use: %s path\n", t_arg[ 0 ] );
        return 1;
    }

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un saun;
    saun.sun_family = AF_UNIX;
    strcpy( saun.sun_path, t_arg[ 1 ] );

    connect( sock, ( sockaddr * ) &saun, sizeof( saun ) );

    char buf[ 128 ] = "Nazdar\n";
    int ret = write( sock, buf, strlen( buf ) );

    close( sock );

}
