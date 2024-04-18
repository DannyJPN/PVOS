#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>



const int n = 6;
int pipelines[n][ 2 ];
int filedes=-1;
int bufsize = 256;
int ChildCode(int i)
{
		int count = 100;
		srand( getpid() );
		int initdel = rand() % 10;
		sleep (initdel);
		int ret= close(pipelines[i][0]);
		
		if(ret <0)
		{
			printf( "Own read close %d %s\n", ret, strerror( errno ) );

		}
		for(int j = 0;j<n;j++)
		{
			if(j!=i)
			{
				ret=close(pipelines[j][0]);
				if(ret <0)
				{
					printf( "%dth read close %d %s\n",j, ret, strerror( errno ) );
	
				}
			
			
				ret = close(pipelines[j][1]);
				if(ret <0)
				{
					printf( "%dth read close %d %s\n",j, ret, strerror( errno ) );
	
				}
			}
		}
		printf("Child %d closed reading end %x\n",i,pipelines[i][ 0 ]);
	
		int stat = rand()% 200;
		printf("Child %d says status %d\n",i,stat);
		
        	char buf[ bufsize ]={0};
		sprintf( buf,"I am child %d with PID %d and I send data to writing end %x and read can be done from end %x\n", i, getpid() ,pipelines[i][ 1 ],pipelines[i][ 0 ]);
        	
        	
        	while(count--)
		{
		      	write( pipelines[i][ 1 ], buf, strlen( buf ) );
		      	sleep( 1  );
        
		}
		ret=close(pipelines[i][ 1 ]);
		if(ret <0)
		{
			printf( "Own write close %d %s\n", ret, strerror( errno ) );

		}		
		printf("Child %d closed writing end %x\n",i,pipelines[i][ 1 ]);
	        
		exit(  stat);

}

void signalhandler( int sig ,siginfo_t*si,void*unused)
{
	char buf[bufsize]={0};
	int data= read(si->si_fd,buf,bufsize);
//    	close(si->si_fd);
	
	if (data > 0) 
	{
    		buf[data] = '\0';
	
    		//printf("Signalhandler received %dB at reading end %x\n",data,si->si_fd);
		char outbuf[4*bufsize]={0};
		sprintf(outbuf, "Received signal %d at process %d from file descriptor %x.\nReceived %dB of data: %s\n", sig ,getpid(),si->si_fd,data,buf);
		write(1,outbuf,strlen(outbuf));
    	}
	
}

int main( int num_arg, char **args )
{
	
	int pid = -1;
	int i;
	
    

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
	
	
		struct sigaction sa;
	    	sa.sa_sigaction = signalhandler;
		sa.sa_flags = SA_SIGINFO|SA_RESTART;
		sigemptyset( &sa.sa_mask );
		sigaction( SIGIO, &sa, NULL );
    		
	
		for ( i = 0; i < n; i++ )
		{
			int ret = close(pipelines[i][1]);
			if(ret <0)
			{
				printf( "Parent's %d write close %d %s\n",i, ret, strerror( errno ) );

			}
			printf("Parent  closed writing end %x \n",pipelines[i][1]);
		}
		printf( "I am parent with PID %d\n", getpid() );
		int wpid=-1;
		int  status=-1;
		while((wpid= waitpid(-1, &status ,0)  )>0)
		{
			printf( "Ended proces %d with status %d\n", wpid,WEXITSTATUS(status) );
		
		}
		for ( i = 0; i < n; i++ )
		{
			int ret = close(pipelines[i][0]);
			if(ret <0)
			{
				printf( "Parent's %d read close %d %s\n",i, ret, strerror( errno ) );

			}
			printf("Parent  closed reading end %x \n",pipelines[i][0]);
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

