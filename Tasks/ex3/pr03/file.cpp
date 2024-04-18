#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

int main()
{
    while ( 1 )
    {
        printf( "." );
        fflush( stdout );
        usleep( 1000 );

    }

}
