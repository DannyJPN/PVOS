#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

void sigal( int sig )
{
    printf( "Signal %d\n", sig );
}

int g_fd = -1;

void sigio( int sig )
{
    printf( "Mam signal %d\n", sig );
    char buf[ 111 ];
    int ret = read( g_fd, buf, sizeof( buf ) );
    if ( ret < 0 ) printf( "Read selhal!\n" );
    else
    {
        buf[ ret ] = 0;
        printf( "Vstup: %s", buf );
    }
}

int main()
{
    int roura[ 2 ];
    pipe( roura );

    if ( fork() == 0 )
    {
        close( roura[ 0 ] );
        while ( 1 )
        {
            char buf[ 333 ];
            sprintf( buf, "(%d) %d\n", getpid(), rand() % 10000 );
            write( roura[ 1 ], buf, strlen( buf ) );
            sleep( 1 + rand() % 5 );
        }
    }

    close( roura[ 1 ] );

    struct sigaction sa;
    sa.sa_handler = sigio;
    sa.sa_flags = SA_RESTART;
    sigemptyset( &sa.sa_mask );

    sigaction( SIGIO, &sa, NULL );

    g_fd = roura[ 0 ];
    fcntl( g_fd, F_SETFL, fcntl( g_fd, F_GETFL ) | O_ASYNC );
    fcntl( g_fd, F_SETSIG, SIGIO );
    fcntl( g_fd, F_SETOWN, getpid() );

    while ( 1 ) sleep( 1 );

    return 0;
}

int main2()
{
    struct sigaction sa;
    sa.sa_handler = sigio;
    sa.sa_flags = SA_RESTART;
    sigemptyset( &sa.sa_mask );

    sigaction( SIGIO, &sa, NULL );

    g_fd = 0;
    fcntl( g_fd, F_SETFL, fcntl( g_fd, F_GETFL ) | O_ASYNC );
    fcntl( g_fd, F_SETSIG, SIGIO );
    fcntl( g_fd, F_SETOWN, getpid() );

    while ( 1 ) sleep( 1 );

    return 0;
}

int main1()
{
    struct sigaction sa;
    sa.sa_handler = sigal;
    sa.sa_flags = SA_RESTART;
    sigemptyset( &sa.sa_mask );

    sigaction( SIGALRM, &sa, NULL );

    alarm( 5 );

    char buf[ 333 ];
    int ret = read( 0, buf, sizeof( buf ) );

    if ( ret < 0 ) printf( "Error: %s\n", strerror( errno ) );

    return 0;
}
