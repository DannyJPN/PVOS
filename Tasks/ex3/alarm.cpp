#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <string>
#include <cstring>

using namespace std;
void my_signal( int sig )
{
	
    printf( "\rReceived signal %d at process %d\nMessage not delivered\n", sig ,getpid());
	exit(2);
    
}

int main()
{
	int seconds= 30;
	struct sigaction sa;
    sa.sa_handler = my_signal;
    sa.sa_flags = 0;
    sigemptyset( &sa.sa_mask );

    sigaction( SIGALRM, &sa, NULL );
    
    
    
    alarm(seconds);
    char buffer[4096];
    printf("Enter text: ");
    fflush(stdout);
	read(0,&buffer,4096);
    
    printf("Message received: %s",buffer);;
    exit(0);
    
    
	

}
