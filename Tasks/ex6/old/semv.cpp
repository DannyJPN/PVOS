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
	   printf("Process: %d\tSemstate before down: %d\n",getpid(),semctl(semid,0,GETVAL,NULL));
       semop( semid, &down, 1 );
	   printf("Process: %d\tSemstate after down: %d\n",getpid(),semctl(semid,0,GETVAL,NULL));
       printf( "(%d) zadej dve cisla:\n", getpid() );
       scanf( "%d", &i );
       scanf( "%d", &j );
       printf("Process: %d\tSemstate before up: %d\n",getpid(),semctl(semid,0,GETVAL,NULL));
	   semop( semid, &up, 1 );
	   printf("Process: %d\tSemstate after up: %d\n",getpid(),semctl(semid,0,GETVAL,NULL));
       printf( "(%d) mam dve cisla: %d %d\n", getpid(), i, j );
   }
}
