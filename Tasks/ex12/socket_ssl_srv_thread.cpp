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
 #include <openssl/rsa.h>       /* SSLeay stuff */
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace std;
#define STR_CLOSE   ":close"
#define STR_QUIT    "quit"

//***************************************************************************
// log messages

#define LOG_ERROR               0       // errors
#define LOG_INFO                1       // information and notifications
#define LOG_DEBUG               2       // debug messages
 #define ErrorCheck(value){    DetectError(value,__LINE__);           } 


/* define HOME to be dir for key and cert files... */
#define HOME "./"
/* Make these what you want for cert & key files */
#define CERTF  HOME "my.crt"
#define KEYF  HOME  "my.key"


#define CHK_NULL(x) if ((x)==NULL) exit (1)
#define CHK_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(2); }

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
 SSL_CTX* ctx;
typedef struct
{
	
	int sock;
	sockaddr_in address;
	socklen_t addr_len;
	pthread_t tid;
	
} connection_t;
	//intialization
map<pthread_t,connection_t*> conns;	

typedef struct
{
	SSL* ssl;
	int fd;
	int size;
	char* buffer;
	int index;
	
}innerreader;


int readline_select_adv(innerreader*inread,char*inbuf,int *len,int timeout_us)
{
	
	
	for(int i = 0;i<inread->size;i++)
	{
		inbuf[i]=0;
		
	}
	struct timeval tim;
	tim.tv_sec = 0;
	tim.tv_usec = timeout_us;

      
	
    	while ( 1 )
    	{
        	fd_set target;
	        FD_ZERO( &target );
	        FD_SET( inread->fd, &target );
	        int max = inread->fd+1;
       
            int newline=-1;
                for (int i =0;i<inread->index+1;i++)
                {
                    if(inread->buffer[i] == '\n' )
                    {
                        newline=i;
                      
                        break;
                        
                    }
                    
                }
      
                
            if(newline >=0)
            {
               // printf("Current: [%s]\n",inread->buffer);
                memcpy((char*)inbuf ,inread->buffer,newline+1);
                ((char*)inbuf)[newline+2] = '\0';
               // printf("Copying %d bytes from [%s]\ninbuf [%s]",newline+1,inread->buffer,inbuf);
                memmove(inread->buffer,&inread->buffer[newline+1],(inread->index)-(newline));
               // printf("Moving %d bytes,result: (%s)\n",(inread->index)-newline,inread->buffer);
                inread->index -= (newline+1);
                //printf("New ININ: %d\n",inread->index);
               
                *len=newline+1;
                return *len;
            }	   
	   
	   
	   
	   
	   
	   
	   
	   
	   
	   
	     int ret = select( max , &target, NULL, NULL, &tim);
        	
	     if(ret <0)
	     {
	     	//*len=inread->index;
	     	return ret;
	     }
	     if(ret == 0)
			{
				memcpy(inbuf ,inread->buffer,inread->index);
				((char*)inbuf)[inread->index+1] = '\0';
				
				*len=inread->index;
				errno=EAGAIN;
				return -2;
			}
			
	    if ( FD_ISSET( inread->fd ,&target ) )
	        {
            //100,5 95
				
	            ret	= SSL_read( inread->ssl, &inread->buffer[inread->index], (inread->size)-(inread->index));
        	    if ( ret < 0 ) 
        	    {
	        	   	//*len=inread->index;
        	    	return ret;
        	    }
				if(ret==0)
				{
					memcpy(inbuf ,inread->buffer,inread->index);
					((char*)inbuf)[inread->index] = '\0';
					
					*len=inread->index;
				
					return ret;
				
				}
				inread->index+=ret;
			
			
				for (int i =0;i<inread->index+1;i++)
                {
                    if(inread->buffer[i] == '\n' )
                    {
                        newline=i;
                      
                        break;
                        
                    }
                    
                }
				if(newline >=0)
				{
				// printf("Current: [%s]\n",inread->buffer);
					memcpy((char*)inbuf ,inread->buffer,newline+1);
					((char*)inbuf)[newline+2] = '\0';
				// printf("Copying %d bytes from [%s]\ninbuf [%s]",newline+1,inread->buffer,inbuf);
					memmove(inread->buffer,&inread->buffer[newline+1],(inread->index)-(newline));
				// printf("Moving %d bytes,result: (%s)\n",(inread->index)-newline,inread->buffer);
					inread->index -= (newline+1);
					//printf("New ININ: %d\n",inread->index);
				
					*len=newline+1;
					return *len;
				}				
			
			
				
			
			
			
			
	
	        }
 
	        
	        

    }


}














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
        //SSL* l_sslsock_client = conn->sslsock;
         pollfd l_readpoll[ 1 ];

        l_readpoll[ 0 ].fd = l_sock_client;
        l_readpoll[ 0 ].events = POLLIN;   
         SSL*     ssl;
  	X509*    client_cert;
  	char*    str;
  	int err;
        
        
	log_msg( LOG_INFO, "Thread ID %u started operating at socket %d",conn->tid,conn->sock );
        getpeername( conn->sock, ( sockaddr * ) &cl_addr, &lsa );
        log_msg( LOG_INFO, "Connected client IP: '%s'  port: %d",
        inet_ntoa( cl_addr.sin_addr ), ntohs( cl_addr.sin_port ) );
    
    
    			  ssl = SSL_new (ctx);                           CHK_NULL(ssl);
 				SSL_set_fd (ssl, l_sock_client);
			  err = SSL_accept (ssl);                        CHK_SSL(err);
  
		  /* Get the cipher - opt */
  
			  printf ("SSL connection using %s\n", SSL_get_cipher (ssl));
  
		  /* Get client's certificate (note: beware of dynamic allocation) - opt */

			  client_cert = SSL_get_peer_certificate (ssl);
			  if (client_cert != NULL) 
			  {
    				printf ("Client certificate:\n");
    
				str = X509_NAME_oneline (X509_get_subject_name (client_cert), 0, 0);
				    CHK_NULL(str);
				    printf ("\t subject: %s\n", str);
				    OPENSSL_free (str);
    
				    str = X509_NAME_oneline (X509_get_issuer_name  (client_cert), 0, 0);
				    CHK_NULL(str);
				    printf ("\t issuer: %s\n", str);
				    OPENSSL_free (str);
    
				    /* We could do all sorts of certificate verification stuff here before
			       deallocating the certificate. */
    
				    X509_free (client_cert);
  
  
  			}
  			 else  
  			  { 
  			 
  			 
  			 printf ("Client does not have certificate.\n");
  			 
  			
                      /* SSL_free (ssl); 
                        ErrorCheck(close( l_sock_client ));
                        
                        						
			log_msg( LOG_INFO, "Connection closed. Thread %u terminating. Socket %d closing" ,conn->tid,conn->sock);
			sem_wait(vectsem);
			conns.erase(conn->tid);
                       sem_post(vectsem);
                       free(conn);
			conn=nullptr;
                        
                        
                        log_msg( LOG_INFO, "Connection closed. Waiting for new client." );
                        break;
  			 
  			 pthread_exit((void*)1);*/
  			 }
			
			
    
    
    
    
       
        while ( ACTIVE  )
        { // communication
        int len = 1024;
            char l_buf[ len ];


/*for ReadLine*/

int ti = 10*1000000;
    innerreader inread;
	inread.fd = l_sock_client;
    inread.size = 65536;
    inread.buffer = new char[inread.size];
    inread.index=0;
    inread.ssl = ssl;





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
                //int l_len = read( l_sock_client, l_buf, sizeof( l_buf ) );
               // int l_len = SSL_read( ssl, l_buf, sizeof( l_buf ) );
                
                int l_len = readline_select_adv(&inread,l_buf,&len,ti);
                
                
                
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
		/*map<pthread_t,connection_t*>::iterator it;
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
			
		
		}*/
			
			
			
			
                // close request?
                //if ( !strncasecmp( l_buf, "close", strlen( STR_CLOSE ) ) )
                //if (regex_match (l_buf, regex(STR_CLOSE) ))
                
                if(strstr(l_buf,STR_CLOSE))
                {
                        log_msg( LOG_INFO, "Client sent 'close' request to close connection." );
                       
                        ErrorCheck(close( l_sock_client ));
                        SSL_free (ssl); 
                        						
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


  //char     buf [4096];
  const SSL_METHOD *meth;
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

  SSL_load_error_strings();
  SSLeay_add_ssl_algorithms();
  meth = SSLv23_server_method();
  ctx = SSL_CTX_new (meth);
  if (!ctx) {
    ERR_print_errors_fp(stderr);
    exit(2);
  }
  
  if (SSL_CTX_use_certificate_file(ctx, CERTF, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(3);
  }
  if (SSL_CTX_use_PrivateKey_file(ctx, KEYF, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(4);
  }

  if (!SSL_CTX_check_private_key(ctx)) {
    fprintf(stderr,"Private key does not match the certificate public key\n");
    exit(5);
  }


    log_msg( LOG_INFO, "Server will listen on port: %d.", l_port );
	int semval=-1;
	vectsem = sem_open( "/vectsem", O_CREAT , 0660,1 );//robo
	sem_init( vectsem, 1, 1 );//semctl( robosem, 0, SETVAL, 1);
	sem_getvalue(vectsem,&semval);
	printf( "Initial robosem: %d\n", semval );

    // socket creation
    int l_sock_listen = socket( AF_INET, SOCK_STREAM, 0 );CHK_ERR(l_sock_listen, "socket");
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
		/*map<pthread_t,connection_t*>::iterator it;
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
                */
                
                
                
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
  SSL_CTX_free (ctx);                  
   
    return 0;
}
