#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

int main()
{

    for ( int i = 0; i < 1000; i++ )
    {
        printf( "cislo %d\n", i );
        //fflush( stdout );
        usleep( 100000 );
    }
}
