#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
/*
Rozpracovaná verze.
Použil jsem jiný vzor,protože u vašeho příkladu jsem měl příliš velký problém dostat do toho paralelismus.
Sockety mi samy o sobě potíže nedělají.
Posílám jako doklad aktivity.
Má to alespoň náběh na správné řešení?
*/
#define ErrorCheck(value){    DetectError(value,__LINE__);           } 
void DetectError(int result,int linenum)
{
	if(result <0)
	{
		printf("%d\t%s\n",linenum,strerror(errno));
		
	}
	
}
struct innerreader
{
	int size;
	char* buffer;
	int index;
	
};


int readline(int fd,void*buf,int len,int timeout_ms,innerreader*inread)
{//2 541 
    struct timeval selecttimeout;
    selecttimeout.tv_sec = timeout_ms/1000;
    selecttimeout.tv_usec = (timeout_ms%1000)*1000;
    
    
    
    while ( 1 )
        {
            fd_set target;
            FD_ZERO( &target );
            FD_SET( fd, &target );
            int max = fd+1;
            int newline=-1;
                for (int i =0;i<inread->index;i++)
                {
                    if(inread->buffer[i] == '\n' )
                    {
                        newline=i;
                        break;
                        
                    }
                    
                }
      
                
            if(newline >=0)
            {
                
                memcpy((char*)buf ,inread->buffer,newline);
                ((char*)buf)[newline] = '\0';
                //printf("Copying %d bytes from %s\n",newline,inread->buffer);
                memmove(inread->buffer,&inread->buffer[newline+1],(inread->index)-(newline+1));
                //printf("Moving %d bytes,result: (%s)\n",(inread->index)-newline,inread->buffer);
                inread->index -= (newline+1);
                //printf("New ININ: %d\n",inread->index);
                
                return newline;
            }
            int readyfds = select( max + 1, &target, NULL, NULL, &selecttimeout );
            if ( readyfds < 0 ) break;
            while(readyfds == 0)
            {
                printf("timeout expired\n");
                return -2;
            }
            if ( FD_ISSET( fd ,&target ) )
            {
                int ret	= read( fd, &inread->buffer[inread->index], (inread->size)-(inread->index));
                inread->index+=ret;
                //printf("Ret:%d\t(%s)\n",ret,inread->buffer);
                //printf("ININ: %d\n",inread->index);
                if ( ret < 0 ) return -1;
                else if (ret==0)
                {
                    int l=inread->index;
                    memcpy((char*)buf ,inread->buffer,inread->index);
                    ((char*)buf)[inread->index] = '\0';
                    inread->index = 0;
                    return l;
                }
        
            }
    
        }
    
    return -1;
}

typedef struct
{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;

char* CheckState(char* buffer)
{
	char result[16]={'\0'};
	int*readnums;
		int tokencount = 0;
		
		for(int i =0;i<readb;i++)
		{
			if(res[i] == separator)
			{
				tokencount++;
			}
			
		}
		if(tokencount<2){continue;}
		readnums= new int[tokencount-1];
		int sum = 0;
		int tokidx = 0;
		int linenum = atoi(strtok(res," "));
		
		printf("%d:",linenum);
		tokidx++;
		
		
		while(tokidx<tokencount)
		{
			
			int item = atoi(strtok(NULL," "));
			printf("%d ",item);
			readnums[tokidx-1] = item;
			
			sum+=item;
			tokidx++;
		}
		int readsum = atoi(strtok(NULL," "));
		
		
		
		printf("\nSum %d\n",sum);
		printf("Readsum %d\n",readsum);
		if(sum != readsum)
		{
			sprintf(result,"%d:BAD",linenum);
			
		}
		else
		{
			sprintf(result,"%d:GOOD",linenum);
			
		}
		
		delete[] readnums;
	
	
	
	
	
	return result;
}

void * process(void * ptr)
{
	char * buffer;
	//int len;
	connection_t * conn;
	long addr = 0;
	int timeout = 20000;
	if (!ptr) pthread_exit(0); 
	conn = (connection_t *)ptr;


	int len = 510;
    char buf[len]={'\0'};
    int ti = 20000;
    
    innerreader inread;
    inread.size = 200;
    inread.buffer = new char[inread.size];
    inread.index=0;
    
	/* read length of message */
	//read(conn->sock, &len, sizeof(int));
	if (len > 0)
	{
		addr = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
		buffer = (char *)malloc((len+1)*sizeof(char));
		buffer[len] = 0;

		/* read message */
		int meslen =readline(conn->sock, buffer, len,timeout,&inread);
		ErrorCheck(meslen);
		/* print message */
		printf("%d.%d.%d.%d: %s\n",
			(int)((addr      ) & 0xff),
			(int)((addr >>  8) & 0xff),
			(int)((addr >> 16) & 0xff),
			(int)((addr >> 24) & 0xff),
			buffer);
	/*send response*/		
		char* result = CheckState(buffer);

		int sendlen = write(conn->sock,result,strlen(result));
		ErrorCheck(sendlen);
		free(buffer);
	}

	/* close socket and clean up */
	close(conn->sock);
	free(conn);
	delete inread.buffer;
	pthread_exit(0);
}

int main(int argc, char ** argv)
{
	int sock_listen = -1;
	struct sockaddr_in address;
	int port;
	connection_t * connection;
	pthread_t thread;

	/* check for command line arguments */
	if (argc != 2)
	{
		fprintf(stderr, "usage: %s port\n", argv[0]);
		return -1;
	}

	/* obtain port number */
	if (sscanf(argv[1], "%d", &port) <= 0)
	{
		fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
		return -2;
	}

	/* create socket */
	sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock_listen <= 0)
	{
		fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
		return -3;
	}



	int opt = 1;
    if ( setsockopt( sock_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) ) < 0 )
      log_msg( LOG_ERROR, "Unable to set socket option!" );
	
	/* bind socket to port */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(sock_listen, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "%s: error: cannot bind socket to port %d\n", argv[0], port);
		return -4;
	}

	/* listen on port */
	if (listen(sock_listen, 5) < 0)
	{
		fprintf(stderr, "%s: error: cannot listen on port\n", argv[0]);
		return -5;
	}

	printf("%s: ready and listening\n", argv[0]);
	
	while (1)
	{
		/* accept incoming connections */
		connection = (connection_t *)malloc(sizeof(connection_t));
		connection->sock = accept(sock_listen, &connection->address, &connection->addr_len);
		if (connection->sock <= 0)
		{
			free(connection);
		}
		else
		{
			/* start a new thread but do not wait for it */
			pthread_create(&thread, 0, process, (void *)connection);
			pthread_detach(thread);
		}
	}
	
	return 0;
}

