#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

void zachyt( int t_sig )
{
    printf( "Mam signal %d\n", t_sig );
}

void papa( void )
{
    printf( "... odchazim....\n" );
}


int main()
{
    struct sigaction l_sa;
    l_sa.sa_handler = zachyt;
    l_sa.sa_flags = SA_RESTART;
    sigemptyset( &l_sa.sa_mask );

    sigaction( SIGINT, &l_sa, nullptr );

    char buf[ 128 ];
    int l_ret = read( 0, buf, sizeof( buf ) );
    printf( "ret = %d\n", l_ret );
    if ( l_ret < 0 ) printf( "Chyba %s\n", strerror( errno ) );
}
