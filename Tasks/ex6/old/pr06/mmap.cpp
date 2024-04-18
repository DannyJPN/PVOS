#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
    int shmfd = shm_open( "/karel", O_RDWR | O_CREAT, 0600 );
    if ( shmfd < 0 ) { printf( "neni share memory\n" ); return 1; }
    ftruncate( shmfd, sizeof( int ) );

    int *counter = ( int * ) mmap( 0, sizeof( int ), PROT_READ | PROT_WRITE, 
            MAP_SHARED, shmfd, 0 );
    if ( counter == nullptr ) { printf( "mmap selhal\n" ); return 1; }

    if ( fork() == 0 ) usleep( 500000 );

    while ( 1 )
    {
        printf( "(%d) Counter %d\n", getpid(), ( * counter )++ );
        sleep( 1 );
    }

}
