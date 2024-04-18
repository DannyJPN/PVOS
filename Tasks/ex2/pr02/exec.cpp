#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int roura[ 2 ];
    pipe( roura );
    if ( fork() == 0 ) 
    {
        dup2( roura[ 1 ], 1 ); 
        close( roura[ 0 ] );
        close( roura[ 1 ] );
        execlp( "ls", "ls", nullptr );
        printf( "Pokud jsem tu, neco je spatne.\n" );
        exit( 0 );
    }
    close( roura[ 1 ] );
    printf( "... pred wait\n" );
    while ( 1 )
    {
        char l_buff[ 3333 ];
        int l_ret = read( roura[ 0 ], l_buff, sizeof( l_buff ) );
        if ( l_ret <= 0 ) break;
        write( 1, l_buff, l_ret );
    }
    wait( nullptr );
    printf( "... rodic pokracuje\n" );
    return 0;
}
