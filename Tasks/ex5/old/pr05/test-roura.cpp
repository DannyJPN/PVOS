#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

void sigalarm( int sig )
{
    printf( "Mam signal %d\n", sig );
}

int main()
{
    int n1, n2, i, j;
    scanf( "%d%n %d%n", &i, &n1, &j, &n2 );

    printf( "%d %d %d %d \n", i, n1, j, n2 );
}

int main2()
{
    //int fd = open( "roura", O_RDONLY | O_NONBLOCK );
    int fd = open( "roura", O_RDWR );
    if ( fd < 0 ) printf( "open selhal %s\n", strerror( errno ) );
    else {} // zrusit alarm
    printf( "Mam otevreno %d\n", fd );

    char buf[ 122 ];
    int len = read( fd, buf, sizeof( buf ) );

    printf( "nacteno %d bajtu\n", len );

    if ( len > 0 ) write( 1, buf, len );

    getchar();

    len = read( fd, buf, sizeof( buf ) );

    printf( "nacteno %d bajtu\n", len );

    if ( len > 0 ) write( 1, buf, len );

    close( fd );

    return 0;
}

int main1()
{
    // problem with open, solved by alarm
    struct sigaction sa;
    sa.sa_handler = sigalarm;
    sa.sa_flags = 0;
    sigemptyset( &sa.sa_mask );
    sigaction( SIGALRM, &sa, nullptr );

    alarm( 5 );

    int fd = open( "roura", O_RDONLY );
    if ( fd < 0 ) printf( "open selhal %s\n", strerror( errno ) );
    else {} // zrusit alarm
    printf( "Mam otevreno %d\n", fd );

    getchar();

    close( fd );

    return 0;

}
