
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <aio.h>
#include <sys/wait.h>


int ChildCode(int i)
{
typedef struct aiocb  aiocb_t;
aiocb_t               aiofile;

		//setsid();
		srand( getpid() );
		sleep( i );
		printf( "I am child %d with PID %d\n", i, getpid() );
		char* filename;
		sprintf(filename,"Outfile%d.txt",i);
		FILE *fp = fopen(filename,"a");
		aiofile.aio_fildes = fp;
		char* output; 
		
		sprintf(output, "I am child %d with PID %d\n",i,getpid());
		aiofile.aio_buf = output;
		aiofile.aio_nbytes = sizeof(output);

		aio_write(&aiofile);
		//getchar();
		int stat = errno;
//		printf("Child says status %d\n",stat);
		fclose(fp);
		exit(stat);

}


int main( int num_arg, char **args )
{
int processcount = 20;
	int pid = -1;
	int i;
	for ( i = 0; i < processcount; i++ )
	{
		pid = fork();
		if ( pid == 0 ) break;
	}

	

	if ( pid )
	{ // kod rodice
		int  status=-1;
		printf( "I am parent with PID %d\n", getpid() );
		int wpid = -1;
		while ( ( wpid = wait( &status ) ) > 0 )
		{
//			printf( "Process %d ended with status %d\n", wpid,WEXITSTATUS(status) );


		}


		//getchar();
		printf( "Parent finish...\n" );
	}
	else
	{
	ChildCode(i);
	}

}
