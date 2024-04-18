//***************************************************************************
//
// Program example for labs in subject Operating Systems
//
// 
//
// Example of socket server.
//
// This program is example of socket server and it allows to connect and serve
// the only one client.
// The mandatory argument of program is port number for listening.
//
//***************************************************************************
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <vector>
#include <semaphore.h>
#include <map>
#include <regex>
using namespace std;
#define STR_CLOSE   ":close"
#define STR_QUIT    "quit"

//***************************************************************************
// log messages

#define LOG_ERROR               0       // errors
#define LOG_INFO                1       // information and notifications
#define LOG_DEBUG               2       // debug messages
 #define ErrorCheck(value){    DetectError(value,__LINE__);           } 
void DetectError(int result,int linenum)
{
	if(result <0)
	{
		printf("%d\t%s\n",linenum,strerror(errno));
		
	}
	
}
// debug flag
int g_debug = LOG_INFO;
int solnum = -1;
int ACTIVE = 1;
sem_t* vectsem;
typedef struct
{
	int sock;
	sockaddr_in address;
	socklen_t addr_len;
	pthread_t tid;
	
} connection_t;
	//intialization
map<pthread_t,connection_t*> conns;	


void log_msg( int t_log_level, const char *t_form, ... )
{
    const char *out_fmt[] = {
            "ERR: (%d-%s) %s\n",
            "INF: %s\n",
            "DEB: %s\n" };

    if ( t_log_level && t_log_level > g_debug ) return;

    char l_buf[ 1024 ];
    va_list l_arg;
    va_start( l_arg, t_form );
    vsprintf( l_buf, t_form, l_arg );
    va_end( l_arg );

    switch ( t_log_level )
    {
    case LOG_INFO:
    case LOG_DEBUG:
        fprintf( stdout, out_fmt[ t_log_level ], l_buf );
        break;

    case LOG_ERROR:
        fprintf( stderr, out_fmt[ t_log_level ], errno, strerror( errno ), l_buf );
        break;
    }
}

//***************************************************************************
// help

void help( int t_narg, char **t_args )
{
    if ( t_narg <= 1 || !strcmp( t_args[ 1 ], "-h" ) )
    {
        printf(
            "\n"
            "  Socket server example.\n"
            "\n"
            "  Use: %s [-h -d] port_number\n"
            "\n"
            "    -d  debug mode \n"
            "    -h  this help\n"
            "\n", t_args[ 0 ] );

        exit( 0 );
    }

    if ( !strcmp( t_args[ 1 ], "-d" ) )
        g_debug = LOG_DEBUG;
}

//***************************************************************************


void cleaner(int signum)
{
	ACTIVE = 0;

}




void*handleclient(void*args)
{

  /*      uint l_lsa = sizeof( l_srv_addr );
        // my IP
        getsockname( l_sock_client, ( sockaddr * ) &l_srv_addr, &l_lsa );
        log_msg( LOG_INFO, "My IP: '%s'  port: %d",
        inet_ntoa( l_srv_addr.sin_addr ), ntohs( l_srv_addr.sin_port ) );
        // client IP
        getpeername( l_sock_client, ( sockaddr * ) &l_srv_addr, &l_lsa );
        log_msg( LOG_INFO, "Client IP: '%s'  port: %d",
        inet_ntoa( l_srv_addr.sin_addr ), ntohs( l_srv_addr.sin_port ) );
*/
        // change source from sock_listen to sock_client
        

        sockaddr_in cl_addr;
	cl_addr.sin_family = AF_INET;
	uint lsa = sizeof( cl_addr );
	
        connection_t* conn = (connection_t*)args;
        int l_sock_client = conn->sock;
         pollfd l_readpoll[ 1 ];

        l_readpoll[ 0 ].fd = l_sock_client;
        l_readpoll[ 0 ].events = POLLIN;   
        
        
        
	log_msg( LOG_INFO, "Thread ID %u started operating at socket %d",conn->tid,conn->sock );
        getpeername( conn->sock, ( sockaddr * ) &cl_addr, &lsa );
        log_msg( LOG_INFO, "Connected client IP: '%s'  port: %d",
        inet_ntoa( cl_addr.sin_addr ), ntohs( cl_addr.sin_port ) );
    
       
        while ( ACTIVE  )
        { // communication
            char l_buf[ 1024 ];

            // select from fds
            int l_poll = poll( l_readpoll, 1, -1 );

            if ( l_poll < 0 )
            {
                log_msg( LOG_ERROR, "Function poll failed!" );
                exit( 1 );
            }


            if ( l_readpoll[ 0 ].revents & POLLIN )
            {
                // read data from socket
                int l_len = read( l_sock_client, l_buf, sizeof( l_buf ) );
                if ( !l_len )
                {
                        log_msg( LOG_DEBUG, "Client closed socket!" );
                        close( l_sock_client );
                        break;
                }
                else if ( l_len < 0 )
                        log_msg( LOG_DEBUG, "Unable to read data from client." );
                else
                        log_msg( LOG_DEBUG, "Read %d bytes from client.", l_len );

                // write data from client
                l_len = write( STDOUT_FILENO, l_buf, l_len );
                if ( l_len < 0 )
                        log_msg( LOG_ERROR, "Unable to write data to stdout." );
			
		//broadcast
		map<pthread_t,connection_t*>::iterator it;
		for (it=conns.begin(); it!=conns.end(); ++it)
		{
			if(it->first == conn->tid)
			{
			continue;
			}
			connection_t* clcon = it->second;
			
			
			
		        // send data to client
            		l_len = write( clcon->sock, l_buf, l_len );
            		if ( l_len < 0 )
                		log_msg( LOG_ERROR, "Unable to send data to client %d.",clcon->tid );
            		else
                		log_msg( LOG_DEBUG, "Sent %d bytes to client %d.", l_len,clcon->tid );
			
		
		}
			
			
			
			
                // close request?
                //if ( !strncasecmp( l_buf, "close", strlen( STR_CLOSE ) ) )
                //if (regex_match (l_buf, regex(STR_CLOSE) ))
                
                if(strstr(l_buf,STR_CLOSE))
                {
                        log_msg( LOG_INFO, "Client sent 'close' request to close connection." );
                        close( l_sock_client );
                        						
			log_msg( LOG_INFO, "Connection closed. Thread %u terminating. Socket %d closing" ,conn->tid,conn->sock);
			sem_wait(vectsem);
			conns.erase(conn->tid);
                       sem_post(vectsem);
                       free(conn);
			conn=nullptr;
                        
                        
                        log_msg( LOG_INFO, "Connection closed. Waiting for new client." );
                        break;
                }
            }
            // request for quit
           /* if ( !strncasecmp( l_buf, "quit", strlen( STR_QUIT ) ) )
            {
                //close( l_sock_listen );
                close( l_sock_client );
                log_msg( LOG_INFO, "Request to 'quit' entered" );
                pthread_exit( 0 );
            }*/
        } // while communication

pthread_exit(0);
}


int main( int t_narg, char **t_args )
{
    if ( t_narg <= 1 ) help( t_narg, t_args );

    int l_port = 0;

	signal(SIGINT, cleaner);
    // parsing arguments
    for ( int i = 1; i < t_narg; i++ )
    {
        if ( !strcmp( t_args[ i ], "-d" ) )
            g_debug = LOG_DEBUG;

        if ( !strcmp( t_args[ i ], "-h" ) )
            help( t_narg, t_args );

        if ( *t_args[ i ] != '-' && !l_port )
        {
            l_port = atoi( t_args[ i ] );
            break;
        }
    }

    if ( l_port <= 0 )
    {
        log_msg( LOG_INFO, "Bad or missing port number %d!", l_port );
        help( t_narg, t_args );
    }

    log_msg( LOG_INFO, "Server will listen on port: %d.", l_port );
	int semval=-1;
	vectsem = sem_open( "/vectsem", O_CREAT , 0660,1 );//robo
	sem_init( vectsem, 1, 1 );//semctl( robosem, 0, SETVAL, 1);
	sem_getvalue(vectsem,&semval);
	printf( "Initial robosem: %d\n", semval );

    // socket creation
    int l_sock_listen = socket( AF_INET, SOCK_STREAM, 0 );
    if ( l_sock_listen == -1 )
    {
        log_msg( LOG_ERROR, "Unable to create socket.");
        exit( 1 );
    }
	solnum = l_sock_listen;
    in_addr l_addr_any = { INADDR_ANY };
    sockaddr_in l_srv_addr;
    l_srv_addr.sin_family = AF_INET;
    l_srv_addr.sin_port = htons( l_port );
    l_srv_addr.sin_addr = l_addr_any;

    // Enable the port number reusing
    int l_opt = 1;
    if ( setsockopt( l_sock_listen, SOL_SOCKET, SO_REUSEADDR, &l_opt, sizeof( l_opt ) ) < 0 )
      log_msg( LOG_ERROR, "Unable to set socket option!" );

    // assign port number to socket
    if ( bind( l_sock_listen, (const sockaddr * ) &l_srv_addr, sizeof( l_srv_addr ) ) < 0 )
    {
        log_msg( LOG_ERROR, "Bind failed!" );
        close( l_sock_listen );
        exit( 1 );
    }

    // listenig on set port
    if ( listen( l_sock_listen, 1 ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to listen on given port!" );
        close( l_sock_listen );
        exit( 1 );
    }

    log_msg( LOG_INFO, "Enter 'quit' to quit server." );

    // go!
    while ( ACTIVE )
    {
        //int l_sock_client = -1;

        // list of fd sources
        pollfd l_read_poll[ 2 ];

        l_read_poll[ 0 ].fd = STDIN_FILENO;
        l_read_poll[ 0 ].events = POLLIN;
        l_read_poll[ 1 ].fd = l_sock_listen;
        l_read_poll[ 1 ].events = POLLIN;

        while ( ACTIVE ) // wait for new client
        {
            // select from fds
            int l_poll = poll( l_read_poll, 2, -1 );

            if ( l_poll < 0 )
            {
                log_msg( LOG_ERROR, "Function poll failed!" );
                exit( 1 );
            }

            if ( l_read_poll[ 0 ].revents & POLLIN )
            { // data on stdin
                char buf[ 128 ];
                int len = read( STDIN_FILENO, buf, sizeof( buf) );
                if ( len < 0 )
                {
                    log_msg( LOG_DEBUG, "Unable to read from stdin!" );
                    exit( 1 );
                }
                
                

                log_msg( LOG_DEBUG, "Read %d bytes from stdin" );
              //broadcast
		map<pthread_t,connection_t*>::iterator it;
		for (it=conns.begin(); it!=conns.end(); ++it)
		{
			
			connection_t* clcon = it->second;
			
			
			
		        // send data to client
            		len = write( clcon->sock, buf, len );
            		if ( len < 0 )
                		log_msg( LOG_ERROR, "Unable to send data to client %d.",clcon->tid );
            		else
                		log_msg( LOG_DEBUG, "Sent %d bytes to client %d.", len,clcon->tid );
			
		
		}
                
                
                
                
                // request to quit?
                if ( !strncmp( buf, STR_QUIT, strlen( STR_QUIT ) ) )
                {
                    log_msg( LOG_INFO, "Request to 'quit' entered.");
                    close( l_sock_listen );
                    exit( 0 );
                }
            }

            if ( l_read_poll[ 1 ].revents & POLLIN )
            { // new client?
            
            
            	connection_t *conn = (connection_t*)malloc(sizeof(connection_t));
		//conn->sock=-1;
		conn->tid=-1;
		conn->address.sin_family = AF_INET;
		conn->address.sin_addr = l_addr_any;
		conn->addr_len = sizeof(conn->address);
            
            
                //sockaddr_in l_rsa;
                //int l_rsa_size = sizeof( l_rsa );
                // new connection
                //l_sock_client = accept( l_sock_listen, ( sockaddr * ) &l_rsa, ( socklen_t * ) &l_rsa_size );
                conn->sock = accept( l_sock_listen, (sockaddr*)&conn->address, &conn->addr_len );
                if ( conn->sock == -1 )
                {
                        log_msg( LOG_ERROR, "Unable to accept new client." );
                        close( conn->sock );
                        free(conn);
                        exit( 1 );
                }
                
                
         	else
		{
		//thread creation
			 log_msg( LOG_INFO, "New client accepted" );
			pthread_t tid;
			conn->tid = tid;
			
			sem_wait(vectsem);
			conns[conn->tid]=conn;
			sem_post(vectsem);
			
			int pthres =pthread_create(&tid,NULL,&handleclient,(void*)conn);
			if(pthres != 0)
			{
				 log_msg( LOG_DEBUG, "Thread was not created,error %d:%s",pthres,strerror(pthres) );
				
			}
		}

		
		
		
		
		
		
                break;
            }

        } // while wait for client


    } // while ( 1 )
    ErrorCheck(close( solnum ));
    for(unsigned int v=0;v< conns.size();v++)
    {
	log_msg( LOG_INFO, "Closing %u/%u socket %d.",v,conns.size(),conns.at(v)->sock);
        if(conns.at(v)!=nullptr)
        {
            ErrorCheck(pthread_cancel(conns.at(v)->tid));
            ErrorCheck(pthread_join(conns.at(v)->tid,NULL));
            ErrorCheck(close(conns.at(v)->sock));
			free(conns.at(v));
        }
        
    }
                    
   
    return 0;
}
