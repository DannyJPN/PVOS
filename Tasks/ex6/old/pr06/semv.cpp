#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>


int main()
{
   int i, j;
   int semid = semget( 0xcafe, 1, IPC_CREAT | 0660 );
   semctl( semid, 0, SETVAL, 1 );

   sembuf up = { 0, 1, 0 }; 
   sembuf down = { 0, -1, 0 };

   fork();
   while ( 1 )
   {
       semop( semid, &down, 1 );
       printf( "(%d) zadej dve cisla:\n", getpid() );
       scanf( "%d", &i );
       scanf( "%d", &j );
       semop( semid, &up, 1 );
       printf( "(%d) mam dve cisla: %d %d\n", getpid(), i, j );
   }
}
