#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <sys/mman.h>


int main()
{
    int i, j;
    //sem_unlink( "/zavora" );
    // BAD WAY!!! sem_t *l_semp = ( sem_t * ) malloc( sizeof( sem_t ) );
    sem_t *l_semp = ( sem_t * ) mmap( 0, sizeof( sem_t ) * 2, PROT_READ | PROT_WRITE, 
            MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
    //sem_t *l_semp = sem_open( "/zavora", O_RDWR | O_CREAT, 0640, 1 );
    sem_init( &l_semp[ 0 ], 1, 1 );
    sem_init( &l_semp[ 1 ], 1, 1 );


   ///int semid = semget( 0xcafe, 1, IPC_CREAT | 0660 );
   ///semctl( semid, 0, SETVAL, 1 );

   ///sembuf up = { 0, 1, 0 }; 
   ///sembuf down = { 0, -1, 0 };

   fork();
   while ( 1 )
   {
       ///semop( semid, &down, 1 );
       sem_wait( &l_semp[ 0 ] );
       sem_wait( &l_semp[ 1 ] );
       printf( "(%d) zadej dve cisla:\n", getpid() );
       scanf( "%d", &i );
       scanf( "%d", &j );
       ///semop( semid, &up, 1 );
       sem_post( &l_semp[ 1 ] );
       sem_post( &l_semp[ 0 ] );
       printf( "(%d) mam dve cisla: %d %d\n", getpid(), i, j );
   }
}
