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


const int n = 5;
int pipelines[n][ 2 ];
int filedes=-1;
int bufsize = 256;
aiocb cb[n]; 
void ChildCode(int i)
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
	
		
		
		int stat = rand()%200;
		
		
		
		while(count--)
		{
			printf("Child %d has status %d\n",i,stat);
			
        		char buf[ bufsize ];
			sprintf( buf,"I am child %d with PID %d and I send data to writing end %x,read can be done from end %x and my status is %d\n", i, getpid() ,pipelines[i][ 1 ],pipelines[i][ 0 ],count);
	        	write( pipelines[i][ 1 ], buf, strlen( buf ) );

	        	sleep( 1  );
			
		}
		
        	if( close(pipelines[i][ 1 ]) == 0)
		{
			printf("Child %d closed writing end %x\n",i,pipelines[i][ 1 ]);
		}
		else{printf("Child %d FAILED TO CLOSE writing end %x\n",i,pipelines[i][ 1 ]);}
		exit(stat);

}

void signalhandler( int sig ,siginfo_t*si,void*unused)
{
	printf( "Singalhandler activated by child %d\n",si->si_value.sival_int);
	//char buf[bufsize];
    
 
	char *buf;
	while(1)
	{
		int state = aio_error( &cb[si->si_value.sival_int] );
        	if ( state == EINPROGRESS ) 
	        {
	            fprintf( stderr, "Children working..\n");
	            usleep( 1000000 );
	        }
	        else if ( state == 0 )
	        {
	            int ret = aio_return( &cb[si->si_value.sival_int] );
				
	            if ( ret > 0 )
	                {
				buf=(char*)cb[si->si_value.sival_int].aio_buf;
				buf[ret]= '\0';
	
				char outbuf[4*bufsize]={0};
				sprintf(outbuf, "Received signal %d at process %d from file descriptor %x.\nReceived %dB of data: %s\n", sig ,getpid(),cb[si->si_value.sival_int].aio_fildes,ret,buf);
				write(1,outbuf,strlen(outbuf));
	    
					
			}
			
				
			
				
			
        	    	aio_read(&cb[si->si_value.sival_int] );
			break;

	        }
    	}


    
	
	
}

int main( int num_arg, char **args )
{
	
	int pid = -1;
	int i;
	char buf[n][bufsize];
    
	
	
	for ( i = 0; i < n; i++ )
	{
		pipe( pipelines[i] );

	
		cb[i].aio_fildes = pipelines[i][0];
		cb[i].aio_offset = 0;
		cb[i].aio_buf = buf[i];
		cb[i].aio_nbytes = bufsize;	
		cb[i].aio_reqprio = 0;
		cb[i].aio_sigevent.sigev_notify = SIGEV_SIGNAL;
		cb[i].aio_sigevent.sigev_signo=SIGIO;
		//cb[i].aio_sigevent.sigev_value.sival_ptr=&cb[i];
		cb[i].aio_sigevent.sigev_value.sival_int=i;
		//cb[i].aio_lio_opcode = 0;
		aio_read(&cb[i]);
	
	
	
	
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

