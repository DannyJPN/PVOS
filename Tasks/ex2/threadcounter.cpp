



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <string>
#include <cstring>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)


using namespace std;

void *demo_thread( void *params )
{
	printf("I am thread %ld\n",gettid());
	//sleep(5);
	return NULL;
}
int main()
{
	int counter=0;

	pthread_t id;
	while(1)
	{	
		int err = pthread_create( &id, NULL, demo_thread, NULL );
	        if ( err )
	           {
	            printf("Unable to create thread %d.\n", counter );
	            break;
	            }
        	else
                {
           	 printf( "Thread %d created - system id %ld.\n", counter,gettid() );
		 }
	counter++;		
	usleep(1000);
	}
	
	
	printf("Created %d threads in total\n",counter);
	
	
return 0;
}
