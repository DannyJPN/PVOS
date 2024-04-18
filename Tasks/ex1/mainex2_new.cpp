// **************************************************************************
//
// Empty C++ project for education
//
// daniel.krupa.st@vsb.cz, 
//
// **************************************************************************

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int ChildCode(int i)
{
// kod potomka
		//setsid();
		srand( getpid() );
		sleep( rand() % 20 );
		printf( "I am child %d with PID %d\n", i, getpid() );
		//getchar();
		int stat = rand()% 10;
		printf("Child says status %d\n",stat);
		exit(  stat);

}


int main( int num_arg, char **args )
{
	int pcount = atoi(args[1]);
	int pid = -1;
	int i;
	for ( i = 0; i < pcount; i++ )
	{
		pid = fork();
		if ( pid == 0 ) break;
	}

	//printf( "Ja jsem proces PID %d\n", getpid() );
	//getchar();

	if ( pid )
	{ // kod rodice
		
		printf( "I am parent with PID %d\n", getpid() );
	    int wpid=-1;int  status=-1;
		
		while ( 1 )
		{
			usleep(500000);

			pid = fork();
			if(pid ==0)
			{
				i++;
				ChildCode(i);
			}
			else{
				while((wpid= waitpid(-1, &status ,WNOHANG)  )>0)
				{
						printf( "Child Process %d ended with status %d\n", wpid,WEXITSTATUS(status) );
						
						
				}
			}
			
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
