#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int l_pid = fork();
    //if ( l_pid ) l_pid = fork();
    if ( l_pid < 0 ) exit( 1 );
    if ( l_pid == 0 )
    {
        printf( "Ja jsem potomek %d\n", getpid() );
        getchar();
        printf( "Potomek konci...\n" );
        exit( ? );
    }
    else
    {
        printf( "Ja jsem rodic %d\n", getpid() );
        //l_pid = wait( nullptr );
        printf( "Rodic konci a prevzal status od %d\n", l_pid );
    }
}
