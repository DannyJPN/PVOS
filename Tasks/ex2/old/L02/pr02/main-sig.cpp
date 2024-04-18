#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

void my_signal( int sig )
{
    printf( "signal %d\n", sig );
}

int main()
{
    struct sigaction sa;
    sa.sa_handler = my_signal;
    sa.sa_flags = 0;
    sigemptyset( &sa.sa_mask );

    sigaction( SIGCHLD, &sa, NULL );

    if ( fork() == 0 )
    { // potomek
        sleep( 5 );
        printf( "Potomek konci\n" );
        return 0;
    }

    printf( "Rodic ceka na klavesu\n" );
    getchar();
    printf( "Rodic konci\n" );
}
