
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <sstream>
#include <fstream>
//***************************************************************************
// log messages



// global variable for all threads
using namespace std;

void *demo_thread( void *params )
{


    int *ppar = ( int * ) params;
    int my_id = ppar[ 0 ];
    int secs = ppar[ 1 ];
       usleep( my_id * 100000 ); // small start delay

//    sleep( 1 );


    printf( "Thread my_id: %d started and will run %d seconds.", my_id, secs );
stringstream s;
s << "outfile" << my_id << ".txt";
string name = s.str();
  ofstream outfile;
  outfile.open (name);
  outfile << "This is a file number "<<my_id<<"\n";
  outfile.close();




    printf( "Thread my_id: %d fished.", my_id );

    pthread_exit( ( void * ) ( ( intptr_t ) -my_id ) );
}

//***************************************************************************

int main( int argn, char **arg )
{


    int numthread = atoi( arg[ 1 ] );




    if ( numthread <= 0 )
    {
        printf("Argumentles" );
		exit(1);
    }

   printf(  "%d thread will be created.", numthread);


    int thread_param[ numthread ][ 2 ];   // threads parameters
    pthread_t thread_id[ numthread ];     // threads identification
    void* thread_status[ numthread ];     // return values from threads


    // thread initialization - thread id and random timeout



    // threads creation
    for ( int i = 0; i < numthread; i++ )
    {
        int err = pthread_create( &thread_id[ i ], NULL, demo_thread, thread_param[ i ] );
        if ( err )
            printf("Unable to create thread %d.", i );
        else
            printf( "Thread %d created - system id 0x%X. and 0x%X", i, thread_id[ i ],pthread_self() );
    }


    // wait for all threads
    for ( int i = 0; i < numthread; i++ )
	{
        pthread_join( thread_id[ i ], &thread_status[ i ] );
	}

    printf(  "All threads finished..." );

    for ( int i = 0; i < numthread; i++ )
	{
		printf( "Thread my_id: %d finished with return code: %d.", i + 1, thread_status[ i ] );
	}
  }
