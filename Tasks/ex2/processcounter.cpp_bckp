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
		printf( "I am child %d with PID %d\n", i, getpid() );
		//getchar();
		int stat = rand()% 4;
		//printf("Child says status %d\n",stat);
		
		exit(  stat);

}




int main( int num_arg, char **args )
{
	
	int pid = -1;
	int i=0;
	

	
	   
 
	printf( "Parent PID %d\n", getpid() );
	while ( 1)
	{
		i++;
		pid = fork();
		if ( pid == 0 ) 
		{
		
			ChildCode(i);
		
			break;
		
		}
		else if (pid < 0)
		{
			printf("Unable to create process %d. Terminating \n",i);
			break;
		}
	}
	int wpid;
	while ( ( wpid = wait( NULL ) ) > 0 )
	{
		printf("Ended process %d\n",wpid);
	}

	//printf( "Ja jsem proces PID %d\n", getpid() );
	//getchar();



	printf( "Total processes %d\n",i );
}

