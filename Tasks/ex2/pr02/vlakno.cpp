#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>

pid_t gettid() 
{
    return ((pid_t)syscall(SYS_gettid));
}

void *vlakno( void *t_ptr )
{
    int l_id = * ( int * ) t_ptr;
    printf( "Ja jsem vlakno '%d' TTID %d...\n", l_id, gettid() );
    sleep( 5 );
    //getchar();
    printf( "Vlakno konci ...\n" );


    return nullptr;
}

#define N 10
int main()
{

    pthread_t l_ptht[ N ];
    int l_arg[ N ];
    for ( int i = 0; i < N; i++ )
    {
        l_arg[ i ] = i;
        pthread_create( l_ptht + i, nullptr, vlakno, ( void * ) ( l_arg + i ) );
    }

    //void * l_retptr; 
    for ( int i = 0; i < N; i++ )
        pthread_join( l_ptht[ i ], nullptr );
    //printf( "vlakno %s ukonceno.\n", ( char * ) l_retptr );
    return 0;
}
