
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <errno.h>
#include <pthread.h>
#include <algorithm>


#define STR_CLOSE   "close"
#define STR_QUIT    "quit"
 #define ErrorCheck(value){    DetectError(value,__LINE__);           } 
void DetectError(int result,int linenum)
{
	if(result <0)
	{
		printf("%d\t%s\n",linenum,strerror(errno));
		
	}
	
}
//***************************************************************************
// log messages

#define LOG_ERROR               0       // errors
#define LOG_INFO                1       // information and notifications
#define LOG_DEBUG               2       // debug messages
using namespace std;
// debug flag
int debug = LOG_INFO;

typedef struct
{
	int sock;
	struct sockaddr address;
	socklen_t addr_len;
	pthread_t tid;
	
} connection_t;
	//intialization
vector<connection_t*> conns;	
struct innerreader
{
	int size;
	char* buffer;
	int index;
	
};





void log_msg( int log_level, const char *form, ... )
{
    const char *out_fmt[] = {
            "ERR: (%d-%s) %s\n",
            "INF: %s\n",
            "DEB: %s\n" };

    if ( log_level && log_level > debug ) return;

    char buf[ 1024 ];
    va_list arg;
    va_start( arg, form );
    vsprintf( buf, form, arg );
    va_end( arg );

    switch ( log_level )
    {
    case LOG_INFO:
    case LOG_DEBUG:
        fprintf( stdout, out_fmt[ log_level ], buf );
        break;

    case LOG_ERROR:
        fprintf( stderr, out_fmt[ log_level ], errno, strerror( errno ), buf );
        break;
    }
}

//***************************************************************************
// help

void help( int num, char **arg )
{
    if ( num <= 1 ) return;

    if ( !strcmp( arg[ 1 ], "-h" ) )
    {
        printf(
            "\n"
            "  Socket server example.\n"
            "\n"
            "  Use: %s [-h -d] port_number\n"
            "\n"
            "    -d  debug mode \n"
            "    -h  this help\n"
            "\n", arg[ 0 ] );

        exit( 0 );
    }

    if ( !strcmp( arg[ 1 ], "-d" ) )
        debug = LOG_DEBUG;
}
//***************************************************************************
int readline(int fd,void*buf,int len,int timeout_ms,innerreader*inread)
{
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
//****************************************************************************
char* CheckState(char* res,int readb,char separator)
{
	char *result=new char[16];
	int*readnums;
		int tokencount = 0;
		
		for(int i =0;i<readb;i++)
		{
			if(res[i] == separator)
			{
				tokencount++;
			}
			
		}
		if(tokencount<2)
		{
			sprintf(result,"FORMAT CORRUPTED");
			return result;
		}
		readnums= new int[tokencount-1];
		int sum = 0;
		int tokidx = 0;
		int linenum = atoi(strtok(res," "));
		
		//printf("%d:",linenum);
		tokidx++;
		
		
		while(tokidx<tokencount)
		{
			
			int item = atoi(strtok(NULL," "));
			//printf("%d ",item);
			readnums[tokidx-1] = item;
			
			sum+=item;
			tokidx++;
		}
		int readsum = atoi(strtok(NULL," "));
		
		
		
		//printf("\nSum %d\n",sum);
		//printf("Readsum %d\n",readsum);
		if(sum != readsum)
		{
			sprintf(result,"%d:BAD\n",linenum);
			
		}
		else
		{
			sprintf(result,"%d:GOOD\n",linenum);
			
		}
		
		delete[] readnums;
	
	
	
	
	
	return result;
}


//***************************************************************************
void*handleclient(void*args)
{
/*                uint lsa = sizeof( srv_addr );
                // my IP
                getsockname( sock_client, ( sockaddr * ) &srv_addr, &lsa );
                log_msg( LOG_INFO, "My IP: '%s'  port: %d",
                                 inet_ntoa( srv_addr.sin_addr ), ntohs( srv_addr.sin_port ) );
                // client IP
                getpeername( sock_client, ( sockaddr * ) &srv_addr, &lsa );
                log_msg( LOG_INFO, "Client IP: '%s'  port: %d",
                                 inet_ntoa( srv_addr.sin_addr ), ntohs( srv_addr.sin_port ) );
*/
			
			sockaddr_in cl_addr;
			cl_addr.sin_family = AF_INET;
			uint lsa = sizeof( cl_addr );
			int len = 2048;
			char buf[len]={'\0'};
			int ti = 500000;
			
			innerreader inread;
			inread.size = 1000;
			inread.buffer = new char[inread.size];
			inread.index=0;
			connection_t* conn = (connection_t*)args;
			log_msg( LOG_INFO, "Thread ID %d started operating at socket %d",conn->tid,conn->sock );
                getpeername( conn->sock, ( sockaddr * ) &cl_addr, &lsa );
                log_msg( LOG_INFO, "Client IP: '%s'  port: %d",
                                 inet_ntoa( cl_addr.sin_addr ), ntohs( cl_addr.sin_port ) );
			while(1)
			{
				fd_set sockset;
				// empty set
				FD_ZERO( &sockset );
	
				// add listen socket
				FD_SET(conn->sock, &sockset );
				
				int sel = select( conn->sock + 1, &sockset, NULL, NULL, NULL );
	
				if ( sel < 0 )
				{
					log_msg( LOG_ERROR, "Select failed!" );
					pthread_exit( NULL );
				}
				if ( FD_ISSET( conn->sock, &sockset ) )
				{
					// read data from socket
					int l = readline( conn->sock, buf, len, ti,&inread);
					if ( !l )
					{
							log_msg( LOG_INFO, "Client closed socket!" );
							log_msg( LOG_INFO, "Connection closed. Thread %d terminating." ,conn->tid);
							close( conn->sock );
							free(conn);
							conn=nullptr;
							conns.erase(remove(conns.begin(), conns.end(), nullptr), conns.end());
							delete[] inread.buffer;
							
							break;
					}
					else if ( l < 0 )
					{
						log_msg( LOG_ERROR, "Unable to read data from client." );
						
					}
					else
					{
						log_msg( LOG_DEBUG, "Read %d bytes from client.", l );
								
	
					}
	
					// write data to client

					// close request?
					if ( !strncasecmp( buf, "close", strlen( STR_CLOSE ) ) )
					{
							log_msg( LOG_INFO, "Client sent 'close' request to close connection." );
							log_msg( LOG_INFO, "Connection closed. Thread %d terminating." ,conn->tid);
							close( conn->sock );
							free(conn);
							conn=nullptr;
							conns.erase(remove(conns.begin(), conns.end(), nullptr), conns.end());
							delete[] inread.buffer;
							
							break;
					}
                    else
                    {
						//log_msg( LOG_INFO, "\nClient sent data." );
						/*int wl = write( STDOUT_FILENO, buf, l );
						printf("\n");
						if ( wl < 0 )
						{
							log_msg( LOG_ERROR, "Unable to write data to stdout." );
						}*/
						char separator=' ';
                        char* result = CheckState(buf,l,separator);

						int sendlen = write(conn->sock,result,strlen(result));
						ErrorCheck(sendlen);
					
                        
                    }
				}




			}



return NULL;








}

//*************************************************************************
int main( int argn, char **arg )
{
    if ( argn <= 1 ) help( argn, arg );

    int port = 0;
	



	
    // parsing arguments
    for ( int i = 1; i < argn; i++ )
    {
        if ( !strcmp( arg[ i ], "-d" ) )
            debug = LOG_DEBUG;

        if ( !strcmp( arg[ i ], "-h" ) )
            help( argn, arg );

        if ( *arg[ i ] != '-' && !port )
        {
            port = atoi( arg[ i ] );
            break;
        }
    }

    if ( port <= 0 )
    {
        log_msg( LOG_INFO, "Bad or missing port number %d!", port );
        help( argn, arg );
		exit(2);
    }

    log_msg( LOG_INFO, "Server will listen on port: %d.", port );

    // socket creation
    int sock_listen = socket( AF_INET, SOCK_STREAM, 0 );
    if ( sock_listen == -1 )
    {
        log_msg( LOG_ERROR, "Unable to create socket.");
        exit( 1 );
    }

    in_addr addr_any = { INADDR_ANY };
    sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons( port );
    srv_addr.sin_addr = addr_any;

    // Enable the port number reusing
    int opt = 1;
    if ( setsockopt( sock_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) ) < 0 )
      log_msg( LOG_ERROR, "Unable to set socket option!" );

    // assign port number to socket
    if ( bind( sock_listen, (const sockaddr * ) &srv_addr, sizeof( srv_addr ) ) < 0 )
    {
        log_msg( LOG_ERROR, "Bind failed!" );
        close( sock_listen );
        exit( 1 );
    }

    // listenig on set port
    if ( listen( sock_listen, 1 ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to listen on given port!" );
        close( sock_listen );
        exit( 1 );
    }

    log_msg( LOG_INFO, "Enter 'quit' to quit server." );

    // go!
    while ( 1 )
    {
        //int sock_client = -1;

        while ( 1 ) // wait for new client
        {
            // set for handles
            fd_set read_wait_set;
            // empty set
            FD_ZERO( &read_wait_set );
            // add stdin
            FD_SET( STDIN_FILENO, &read_wait_set );
            // add listen socket
            FD_SET( sock_listen, &read_wait_set );

            int sel = select( sock_listen + 1, &read_wait_set, NULL, NULL, NULL );

            if ( sel < 0 )
            {
                log_msg( LOG_ERROR, "Select failed!" );
                exit( 1 );
            }
			else
			{
				log_msg( LOG_DEBUG, "Select succeeded!" );
				
			}

            if ( FD_ISSET( sock_listen, &read_wait_set ) )
            { // new client?
				connection_t *conn = (connection_t*)malloc(sizeof(connection_t));
				conn->sock=-1;
				conn->tid=-1;
				
                //sockaddr_in rsa;
                //int rsa_size = sizeof( rsa );
                // new connection
                //sock_client = accept( sock_listen, ( sockaddr * ) &rsa, ( socklen_t * ) &rsa_size );
                conn->sock = accept( sock_listen, &conn->address, &conn->addr_len );

				if ( conn->sock == -1 )
                {
                        log_msg( LOG_ERROR, "Unable to accept new client." );
                        close( conn->sock );
						free(conn);
                        exit( 1 );
                }
				else
				{
					 log_msg( LOG_INFO, "New client accepted" );
					pthread_t tid;
					conn->tid = tid;
					//conn->sock = sock_client;
					
					conns.push_back(conn);
					int pthres =pthread_create(&tid,NULL,&handleclient,conn);
					if(pthres != 0)
					{
						 log_msg( LOG_DEBUG, "Thread was not created,error %d:%s",pthres,strerror(pthres) );
						
					}

				}

                break;
            }
            if ( FD_ISSET( STDIN_FILENO, &read_wait_set ) )
            { // data on stdin
                char buf[ 128 ];
                int len = read( STDIN_FILENO, buf, sizeof( buf) );
                if ( len < 0 )
                {
                    log_msg( LOG_DEBUG, "Unable to read from stdin!" );
                    exit( 1 );
                }

                log_msg( LOG_DEBUG, "Read %d bytes from stdin" );
                // request to quit?
                if ( !strncmp( buf, STR_QUIT, strlen( STR_QUIT ) ) )
                {
                    log_msg( LOG_INFO, "Request to 'quit' entered.");
                    ErrorCheck(close( sock_listen ));
                    for(unsigned int v=0;v< conns.size();v++)
                    {
						log_msg( LOG_INFO, "Closing socket %d.",conns.at(v)->sock);
                        ErrorCheck(close(conns.at(v)->sock));
                        ErrorCheck(pthread_join(conns.at(v)->tid,NULL));
                        free(conns.at(v));
                        
                        
                    }
                    
                    exit( 0 );
                }
            }
        } // while wait for client
/*
        while ( 1  )
        { // communication
            char buf[ 256 ];
            // set for handles
            fd_set read_wait_set;
            // empty set
            FD_ZERO( &read_wait_set );
            // add stdin
            FD_SET( STDIN_FILENO, &read_wait_set );
            // add client
            //FD_SET( sock_client, &read_wait_set );

            int sel = select( sock_client + 1, &read_wait_set, NULL, NULL, NULL );

            if ( sel < 0 )
            {
                log_msg( LOG_ERROR, "Select failed!" );
                exit( 1 );
            }

            // data on stdin?
            if ( FD_ISSET( STDIN_FILENO, &read_wait_set ) )
            {
                // read data from stdin
                int l = read( STDIN_FILENO, buf, sizeof( buf ) );
                if ( l < 0 )
                        log_msg( LOG_ERROR, "Unable to read data from stdin." );
                else
                        log_msg( LOG_DEBUG, "Read %d bytes from stdin.", l );

                // send data to client
                l = write( sock_client, buf, l );
                if ( l < 0 )
                        log_msg( LOG_ERROR, "Unable to send data to client." );
                else
                        log_msg( LOG_DEBUG, "Sent %d bytes to client.", l );
            }
            // data from client?
            else if ( FD_ISSET( sock_client, &read_wait_set ) )
            {
                // read data from socket
                int l = read( sock_client, buf, sizeof( buf ) );
                if ( !l )
                {
                        log_msg( LOG_DEBUG, "Client closed socket!" );
                        close( sock_client );
                        break;
                }
                else if ( l < 0 )
                        log_msg( LOG_DEBUG, "Unable to read data from client." );
                else
                        log_msg( LOG_DEBUG, "Read %d bytes from client.", l );

                // write data to client
                l = write( STDOUT_FILENO, buf, l );
                if ( l < 0 )
                        log_msg( LOG_ERROR, "Unable to write data to stdout." );

                // close request?
                if ( !strncasecmp( buf, "close", strlen( STR_CLOSE ) ) )
                {
                        log_msg( LOG_INFO, "Client sent 'close' request to close connection." );
                        close( sock_client );
                        log_msg( LOG_INFO, "Connection closed. Waiting for new client." );
                        break;
                }
            }
            // request for quit
            if ( !strncasecmp( buf, "quit", strlen( STR_QUIT ) ) )
            {
                close( sock_listen );
                close( sock_client );
                log_msg( LOG_INFO, "Request to 'quit' entered" );
                    for(int v=0;v< conns.size();v++)
                    {
                        ErrorCheck(close(conns[v]->sock));
                        ErrorCheck(pthread_join(conns[v]->tid,NULL));
                        free(conns[v]);
                        
                        
                    }
                exit( 0 );
            }
        } // while communication*/
    } // while ( 1 )

    return 0;
}
