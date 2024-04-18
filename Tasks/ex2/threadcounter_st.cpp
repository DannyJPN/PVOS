



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
	sleep(50);
	return NULL;
}
int main()
{
	int counter=0;
	int n = 1000000;
	pthread_t id[n];
	while(counter<n)
	{	
		int err = pthread_create( &(id[counter]), NULL, demo_thread, NULL );
	        if ( err )
	           {
	            printf("Unable to create thread %d.\n", counter );
	            break;
	            }
        	else
                {
           	 printf( "Thread %d created - system id %ld.\n", counter,gettid() );
		counter++;		
	
		 }
	
	}
	
	for(int i =0;i<counter;i++)
	{
		pthread_join(id[counter],nullptr);
	}
	
	
	printf("Created %d threads in total\n",counter);
	
	
return 0;
}
