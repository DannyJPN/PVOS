#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>



const	int n = 2;
int signalcounter=0;
int endedcounter=0;

int pipelines[n][ 2 ];
int filedes=-1;
int bufsize = 256;
int ChildCode(int i)
{
// kod potomka
		//setsid();
		srand( getpid() );
		//close(pipelines[i][0]);
		//printf("Child %d closed reading end %x\n",i,pipelines[i][ 0 ]);
		//printf( "I am child %d with PID %d\n", i, getpid() );
		//getchar();
		int stat = rand()% 200;
		printf("Child %d says status %d\n",i,stat);
		if ( stat)
        {
            char buf[ bufsize ];
			sprintf( buf,"I am child %d with PID %d. Writing end %x. Reading end %x\n", i, getpid() ,pipelines[i][1],pipelines[i][0]);
            write( pipelines[i][ 1 ], buf, strlen( buf ) );
			//close(pipelines[i][ 1 ]);
            //printf("Child %d closed writing end %x\n",i,pipelines[i][ 1 ]);
			sleep( 1 + stat );
        }
		printf("Child exits status %d\n",stat);
		exit(  stat);

}



void signalhandler( int sig ,siginfo_t*si,void*unused)
{
	
	char buf[bufsize];
	int data= read(si->si_fd,buf,bufsize);
    //close(si->si_fd);
	char outbuf[4*bufsize];
	sprintf(outbuf, "Received signal %d at process %d from file descriptor %x.\nReceived %dB of data: %s\n", sig ,getpid(),si->si_fd,data,buf);
	write(1,outbuf,strlen(outbuf));
	
	//sprintf(outbuf, "Signalhandler closing reading end %x\n",si->si_fd);
	//write(1,outbuf,strlen(outbuf));
	
	
	
	
	
	
	

	
    
}

int main( int num_arg, char **args )
{
	int pid = -1;
	int i;
	
    //pipe( pipelines );
	
	struct sigaction sa;
    sa.sa_sigaction = signalhandler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset( &sa.sa_mask );

    sigaction( SIGIO, &sa, NULL );

	for ( i = 0; i < n; i++ )
	{
		pipe( pipelines[i] );
		filedes=pipelines[i][0];
		fcntl( filedes, F_SETFL, fcntl( filedes, F_GETFL ) | O_ASYNC );
		fcntl( filedes, F_SETSIG, SIGIO );
		fcntl( filedes, F_SETOWN, getpid() );	
	}
	
	
	for ( i = 0; i < n; i++ )
	{
		
		pid = fork();
		if ( pid == 0 ) break;
	}

	//printf( "Ja jsem proces PID %d\n", getpid() );
	//getchar();

	if ( pid )
	{ // kod rodice
		for ( i = 0; i < n; i++ )
		{
			close(pipelines[i][1]);
		}	
		printf( "I am parent with PID %d\n", getpid() );
	
		int wpid=-1;
		int  status=-1;
		
		while((wpid= waitpid(-1, &status ,WNOHANG)  )>0)
		{
			printf( "Ended proces %d with status %d\n", wpid,WEXITSTATUS(status) );
		
		}
		
		//getchar();
		printf( "Parent finish...\n" );
		}
	else
	{ 
		ChildCode(i);
	}
	printf( "spolecny kod...\n" );
}

