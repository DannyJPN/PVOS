#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <climits>

 #define ErrorCheck(value){    DetectError(value,__LINE__);           } 
void DetectError(int result,int linenum)
{
	if(result <0)
	{
		printf("%d\t%s\n",linenum,strerror(errno));
		
	}
	
}


int readline(int fd,void*buf,int len,int timeout_us)
{
	char*textbuf = ((char*)buf);
	
	for(int i = 0;i<len;i++)
	{
		textbuf[i]=0;
		
	}
	struct timeval tim;
	tim.tv_sec = 0;
	tim.tv_usec = timeout_us;
	int index=0;
    	while ( 1 )
    	{
	        fd_set target;
	        FD_ZERO( &target );
	        FD_SET( fd, &target );
	        int max = fd+1;
       
        	int ret = select( max + 1, &target, NULL, NULL, &tim);
	        if ( ret <= 0 ) return ret;

	        if ( FD_ISSET( fd ,&target ) )
	        {
	            
	        	ret = read( fd, &textbuf[index], 1 );
	        	if ( ret <= 0 ) return ret;
	           
			if(index>=len || textbuf[index] == '\n')
			{
				return index+1;
			}
			index++;
	
        	}

   	}


}




int main()
{
    	int len = 512;
    	char res[len]={'\0'};
    	int timeout = 5000000;
    
   
    	char pipename[]="pipefile";

    	//char separator = ' ';
	int readb=INT_MAX;
  
        int node = mknod(pipename,S_IFIFO|0666,0);
	ErrorCheck(node);
	int pipefile = open( pipename, O_RDONLY,0666 );
	ErrorCheck(pipefile);

    	//int *readnums;
    	while(readb>=0)
	{
		int sum = 0;
		int readsum=0;
		
		
		
		
		readb = readline(pipefile,res,sizeof(res),timeout);
		ErrorCheck(readb);
		if(readb == 0)
		{
			
			printf("Bored\n");
			usleep(15000);
			continue;
		}
		int readarr[128]={0};
		int items=0;
		int totallen=0;
		int i =0;int number=0;
		while(1)
		{
			printf("< ");
			int err=sscanf(res+totallen,"%d %n",&number,&items);
			if(err<=0){printf(">\n");break;}
			printf("Items %d",number);
			totallen+=items;
			readarr[i++]=number;
			sum+=number;
			printf(">\n");
			
		}		
		readsum=number;
		sum-=number;
		printf("Sum %d\n",sum);			
		printf("Readsum %d\n",readsum);
		if(sum != readsum)
		{
			printf("COMPUTATION ERROR!!!\n");
				
		}		
		
		//delete[] readnums;
		
		
		
		
		
		
		
		
		
		
		
	}
	ErrorCheck(close(pipefile));

    
    
    
    
    
    return 0;
}

