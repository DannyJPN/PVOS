#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <string>
#include <cstring>



using namespace std;
int main()
{
int i =0;
char symbols[5] = "-\\|/";
	setvbuf(stdout,NULL,_IONBF,1);
	
	while(1)
	{
	/*
		printf("%c\r",symbols[i%4]);
		i++;
		fflush(stdout);
		usleep(100000);
		*/
		
		printf("%c\r",symbols[i%4]);
		i++;
		usleep(100000);
		
		
	}
	
	

}
