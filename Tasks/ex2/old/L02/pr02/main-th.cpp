#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

void *moje_vlakno( void *t_par )
{
    printf( "Ja jsem vlakno %s\n", ( char * ) t_par );
    sleep( 20 );
    char *str = new char [ 1024 ];
    printf( "Vlakno %d %lx\n", gettid(), pthread_self() );
    sprintf( str, "Vlakno %d %lx\n", gettid(), pthread_self() );
    return str;
}

int main()
{
    pthread_t v1, v2; 
    pthread_create( &v1, NULL, moje_vlakno, ( void * ) "Karel" );
    pthread_create( &v2, NULL, moje_vlakno, ( void * ) "Vaclav" );

    void *r1, *r2;
    pthread_join( v1, &r1 );
    pthread_join( v2, &r2 );
    printf( "v1 return %s", ( char * ) r1 ); 
    printf( "v2 return %s", ( char * ) r2 ); 

    printf( "Main konci\n" );
}
