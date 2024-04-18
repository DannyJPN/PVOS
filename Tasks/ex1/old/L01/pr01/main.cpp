// **************************************************************************
//
// Empty C++ project for education
//
// petr.olivka@vsb.cz, Dept. of Computer Science, FEECS, VSB-TU Ostrava, CZ
//
// **************************************************************************

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main( int num_arg, char **args )
{
	int pid = -1;
	int i;
	for ( i = 0; i < 10; i++ )
	{
		pid = fork();
		if ( pid == 0 ) break;
	}

	//printf( "Ja jsem proces PID %d\n", getpid() );
	//getchar();

	if ( pid )
	{ // kod rodice
		printf( "I am parent with PID %d\n", getpid() );
		int wpid = -1;
		while ( ( wpid = wait( NULL ) ) > 0 )
		{
			printf( "skoncil proces %d\n", wpid );
		}
		//getchar();
		printf( "Parent finish...\n" );
	}
	else
	{ // kod potomka
		//setsid();
		srand( getpid() );
		sleep( rand() % 10 );
		printf( "I am child %d with PID %d\n", i, getpid() );
		//getchar();
		exit( 0 );
	}
	printf( "spolecny kod...\n" );
}
