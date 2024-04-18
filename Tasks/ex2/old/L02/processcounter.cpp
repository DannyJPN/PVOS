#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>




int signalcounter=0;
int endedcounter=0;

int ChildCode(int i)
{
// kod potomka
		//setsid();
		srand( getpid() );
		usleep( rand() % 10000000 );
		//printf( "I am child %d with PID %d\n", i, getpid() );
		//getchar();
		int stat = rand()% 4;
		//printf("Child says status %d\n",stat);
		
		exit(  stat);

}

void my_signal( int sig )
{
	signalcounter++;
    //printf( "Received signal %d at process %d\n", sig ,getpid());
    int wpid=-1;
    int  status=-1;
    while((wpid= waitpid(-1, &status ,WNOHANG)  )>0)
    {
    //printf( "skoncil proces %d statusem %d\n", wpid,WEXITSTATUS(status) );
    endedcounter++;
    }
    
}


int main( int num_arg, char **args )
{
	int n = 100;
	int pid = -1;
	int i;
	
	
	    struct sigaction sa;
    sa.sa_handler = my_signal;
    sa.sa_flags = 0;
    sigemptyset( &sa.sa_mask );

    sigaction( SIGCHLD, &sa, NULL );
	
	
	
	for ( i = 0; i < n; i++ )
	{
		pid = fork();
		if ( pid == 0 ) break;
	}

	//printf( "Ja jsem proces PID %d\n", getpid() );
	//getchar();

	if ( pid )
	{ // kod rodice
		
		printf( "I am parent with PID %d\n", getpid() );
		
		while ( 1)
		{
			
			pid = fork();
			if(pid ==0)
			{
				ChildCode(i);
			}
			
			usleep(1000);
				
			
		if(endedcounter % 100 == 0)
		{
			printf("Signals received: %d\tChildren ended: %d\t Difference: %d\n",signalcounter,endedcounter,endedcounter-signalcounter);
		}	
			
			
		}
		
		
		//getchar();
		printf( "Parent finish...\n" );
	}
	//else
	//{ 
	//ChildCode(i);
	//}
	printf( "spolecny kod...\n" );
}

