#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc,char** argv ) 
{
    int lspipe[ 2 ];
    pipe( lspipe );
  
    
    
    if ( fork() == 0 ) 
    {
        dup2( lspipe[ 1 ], 1 ); 
        close( lspipe[ 0 ] );
        close( lspipe[ 1 ] );
        execvp( "ls",argv );
      	printf( "Pokud jsem tu, neco je s ls spatne.\n" );
        exit( 0 );
    }
   
    close( lspipe[ 1 ] );
   
   
    
    
    close( lspipe[ 0 ] );
    printf( "... pred wait\n" );
	
    while ( 1 )
    {
        char l_buff[ 3333 ];
        int l_ret = read( lspipe[ 0 ], l_buff, sizeof( l_buff ) );
        if ( l_ret <= 0 ) break;

      write( 1, l_buff, l_ret );
        
    }
    
    wait( nullptr );
    close( catpipe[0 ] );
    
    printf( "... rodic pokracuje\n" );
    return 0;
}
