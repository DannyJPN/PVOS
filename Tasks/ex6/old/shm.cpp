#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>


int main()
{
    int shmid = shmget( 0xCAFECAFE, sizeof( int ), IPC_CREAT | 0640 );
    if ( shmid < 0 ) { printf( "shmget selhal\n" ); return 1; }
    int *counter = ( int * ) shmat( shmid, 0, 0 );
    if ( counter == nullptr ) { printf( "mmap selhal\n" ); return 1; }

    if ( fork() == 0 ) usleep( 500000 );

    while ( 1 )
    {
        printf( "(%d) Counter %d\n", getpid(), ( * counter )++ );
        sleep( 1 );
    }

}
