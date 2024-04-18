#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <aio.h>


int signalcounter=0;
int endedcounter=0;

int pipeline[ 2 ];
int filedes=-1;
int bufsize = 256;
int ChildCode(int i)
{
printf( "Child active");
// kod potomka
		//setsid();
		srand( getpid() );
		close(pipeline[0]);
		//printf( "I am child %d with PID %d\n", i, getpid() );
		//getchar();
		int stat = rand()% 200;
		//printf("Child says status %d\n",stat);
		while ( stat)
        {
            char buf[ bufsize ];
            sprintf( buf,"I am child %d with PID %d\n", i, getpid() );
            write( pipeline[ 1 ], buf, strlen( buf ) );
            sleep( 1 + stat );
        }
		exit(  stat);

}

void signalhandler( int sig ,siginfo_t*si,void*unused)
{

	printf( "Singalhandler activated");
	char buf[bufsize];
    aiocb cb; // toto ne!  = {...};
    cb.aio_fildes = filedes;
    cb.aio_offset = 0;
    cb.aio_buf = buf;
    cb.aio_nbytes = bufsize;
    cb.aio_reqprio = 0;
    cb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;

	int state= aio_error(&cb);
	

  if ( state == EINPROGRESS ) 
        {
            fprintf( stderr, "nic se nedeje...\n" );
            usleep( 500000 );
        }
        else if ( state == 0 )
        {
            int ret = aio_return( &cb );
            if ( ret > 0 )
                {

    				printf( "Received signal %d at process %d from file descriptor%d,%dB of data: %s\n", sig ,getpid(),si->si_fd,(int)cb.aio_nbytes,(char*)cb.aio_buf);
   				 int wpid=-1;
   				 int  status=-1;
   				 while((wpid= waitpid(-1, &status ,WNOHANG)  )>0)
   				 {
   						 printf( "Ended proces %d with status %d\n", wpid,WEXITSTATUS(status) );
    
  				  }
				}
			

            aio_read( &cb );
        }


   
}

int main( int num_arg, char **args )
{
	int n = 6;
	int pid = -1;
	int i;
	
    pipe( pipeline );
	
	struct sigaction sa;
    sa.sa_sigaction = signalhandler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset( &sa.sa_mask );

    sigaction( SIGIO, &sa, NULL );
filedes=pipeline[0];
	fcntl( filedes, F_SETFL, fcntl( filedes, F_GETFL ) | O_ASYNC );
    fcntl( filedes, F_SETSIG, SIGIO );
    fcntl( filedes, F_SETOWN, getpid() );
	
	
	for ( i = 0; i < n; i++ )
	{
		pid = fork();
		if ( pid == 0 ) break;
	}

	//printf( "Ja jsem proces PID %d\n", getpid() );
	//getchar();

	if ( pid )
	{ // kod rodice
		close(pipeline[1]);
		printf( "I am parent with PID %d\n", getpid() );

		
		
		//getchar();
		printf( "Parent finish...\n" );
	}
	else
	{ 
	ChildCode(i);
	}
	printf( "spolecny kod...\n" );
}

