#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>





int ChildCode(int i)
{

	
		
		int stat = rand()% 4;
		for(int j=0;j< 10000000;j++)
		{
			printf("Child %d outputs number %d\n",i,j);
			usleep( 250000 );
		}
		
		
		
		printf("Child says status %d\n",stat);
		
		exit(  stat);

}

void my_signal( int sig )
{
	
    //printf( "Received signal %d at process %d\n", sig ,getpid());
    int wpid=-1;
    int  status=-1;
    while((wpid= waitpid(-1, &status ,WNOHANG)  )>0)
    {
    //printf( "skoncil proces %d statusem %d\n", wpid,WEXITSTATUS(status) );
   
    }
    
}

void startstop_signal( int sig )
{
	
    //printf( "Received signal %d at process %d\n", sig ,getpid());
 
    
}





int main( int num_arg, char **args )
{
	int n = 3;
	int pid = -1;
	int i;
	
	
	    struct sigaction sa;
    sa.sa_handler = my_signal;
    sa.sa_flags = 0;
    sigemptyset( &sa.sa_mask );

    //sigaction( SIGCHLD, &sa, NULL );
	
struct sigaction sass;
    sass.sa_handler = startstop_signal;
    sass.sa_flags = 0;
    sigemptyset( &sass.sa_mask );

    //sigaction( SIGCONT, &sass, NULL );
   //sigaction( SIGSTOP, &sass, NULL );    
 	
	
	
	
	
	pid_t pids[n];
	
	for ( i = 0; i < n; i++ )
	{
		pid = fork();
		
		if ( pid == 0 ) break;
		pids[i]=pid;
	}

	//printf( "Ja jsem proces PID %d\n", getpid() );
	//getchar();

	if ( pid )
	{ // kod rodice
		
		printf( "I am parent with PID %d\n", getpid() );
		
		
		for (int k = 0; k < n; k++ )
		{
			kill(pids[k],19);
		}
		
		int k = 0;
		while(1) 
		{
			kill(pids[k],18);
			printf("Starting child %u\n",k);
			sleep(10);
			kill(pids[k],19);
			printf("Stoping child %u\n",k);
			k++;
			if(k>=n)
			{
				k=0;
			}
			sleep(2);
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

