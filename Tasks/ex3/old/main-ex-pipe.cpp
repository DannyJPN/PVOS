#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int roura1[ 2 ];
    int roura2[ 2 ];
    pipe( roura1 );
    if ( fork() == 0 ) // potomek1
    {
        close( roura1[ 0 ] );

        for ( int i = 0; i < 10; i++ )
        {
            char buf[ 111 ];
            sprintf( buf, "(%d) %d\n", getpid(), i );
            write( roura1[ 1 ], buf, strlen( buf ) );
            sleep( 5 );
        }

        close( roura1[ 1 ] );
        exit( 0 );
    }
    pipe( roura2 );
    if ( fork() == 0 ) // potomek2
    {
        close( roura2[ 0 ] );

        for ( int i = 0; i < 100; i++ )
        {
            char buf[ 111 ];
            sprintf( buf, "(%d) %d\n", getpid(), i );
            write( roura2[ 1 ], buf, strlen( buf ) );
            sleep( 1 );
        }

        close( roura2[ 1 ] );
        exit( 0 );
    }
    // rodic
    close( roura1[ 1 ] );
    close( roura2[ 1 ] );

    struct pollfd proroury[ 2 ];

    proroury[ 0 ].fd = roura1[ 0 ];
    proroury[ 0 ].events = POLLIN;
    proroury[ 1 ].fd = roura2[ 0 ];
    proroury[ 1 ].events = POLLIN;

    while ( 1 )
    {
        int ret = poll( proroury, 2, -1 );
        if ( proroury[ 0 ].revents & POLLIN )
        {
            char data[ 200 ];
            ret = read( roura1[ 0 ], data, sizeof( data ) );
            if ( ret <= 0 ) break;
            write( 1, data, ret );
        }

        if ( proroury[ 1 ].revents & POLLIN )
        {
            char data[ 200 ];
            ret = read( roura2[ 0 ], data, sizeof( data ) );
            if ( ret <= 0 ) break;
            write( 1, data, ret );
        }
    }
/*
    while ( 1 )
    {
        fd_set proroury;
        FD_ZERO( &proroury );
        FD_SET( roura1[ 0 ], &proroury );
        FD_SET( roura2[ 0 ], &proroury );
        int max = roura1[ 0 ];
        if ( roura2[ 0 ] > max ) max = roura2[ 0 ];

        int ret = select( max + 1, &proroury, NULL, NULL, NULL );
        if ( ret < 0 ) break;

        if ( FD_ISSET( roura1[ 0 ], &proroury ) )
        {
            char data[ 200 ];
            ret = read( roura1[ 0 ], data, sizeof( data ) );
            if ( ret <= 0 ) break;
            write( 1, data, ret );
        }

        if ( FD_ISSET( roura2[ 0 ], &proroury ) )
        {
            char data[ 200 ];
            ret = read( roura2[ 0 ], data, sizeof( data ) );
            if ( ret <= 0 ) break;
            write( 1, data, ret );
        }
    }
*/
    close( roura1[ 0 ] );
    close( roura2[ 0 ] );
    wait( NULL );
    wait( NULL );
}

int main2()
{
    int roura[ 2 ];
    pipe( roura );

    if ( fork() == 0 ) // potomek 1
    {
        close( roura[ 0 ] );
        dup2( roura[ 1 ], 1 );
        close( roura[ 1 ] );
        execlp( "ls", "ls", NULL );
        printf( "Pokud jsem zde, neco je spatne!\n" );
        exit( 0 );
    }
    if ( fork() == 0 ) // potomek 2
    {
        close( roura[ 1 ] );
        dup2( roura[ 0 ], 0 );
        close( roura[ 0 ] );
        execlp( "tr", "tr", "[a-z]", "[A-Z]", NULL );
        printf( "Pokud jsem zde, neco je spatne!\n" );
        exit( 0 );
    }

    close( roura[ 1 ] );
    close( roura[ 0 ] );
    wait( NULL );
    wait( NULL );
}

int main1()
{
    int roura[ 2 ];
    pipe( roura );

    if ( fork() == 0 ) // potomek
    {
        close( roura[ 0 ] );

        for ( int i = 0; i < 10; i++ )
        {
            char buf[ 111 ];
            sprintf( buf, "%d\n", i );
            write( roura[ 1 ], buf, strlen( buf ) );
        }

        close( roura[ 1 ] );
    }
    else // rodic
    {
        close( roura[ 1 ] );

        while ( 1 )
        {
            char data[ 200 ];
            int ret = read( roura[ 0 ], data, sizeof( data ) );
            if ( ret <= 0 ) break;
            write( 1, data, ret );
        }

        close( roura[ 0 ] );
        wait( NULL );
    }
}

