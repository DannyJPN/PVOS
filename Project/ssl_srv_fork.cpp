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
#include <sys/wait.h>
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
#define CHK_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr);}//exit(2); }
#define eprintf( args... ) fprintf( stderr, ## args )
void DetectError(int result,int linenum)
{
	if(result <0)
	{
		printf("%d\t%s\n",linenum,strerror(errno));
		
	}
	
}
int ssl_readline( SSL *ssl, char *buf, int length )
{
	int num = 0;
	while ( num < length - 1 )
	{
		int err = SSL_read( ssl, buf + num, 1 );
		if ( err <= 0 ) return err;
		if ( buf[ num++ ] == '\n' ) break;
	}
	buf[ num ] = 0;
	return num;
}
// debug flag
int g_debug = LOG_INFO;
int solnum = -1;
int ACTIVE = 1;
int maxclients = 255;
SSL_CTX* ctx;

	//intialization
map<unsigned int,int> clients;	


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




void handleclient(int sock,int sockpairend)
{




        sockaddr_in cl_addr;
	cl_addr.sin_family = AF_INET;
	uint lsa = sizeof( cl_addr );
	
        //connection_t* conn = (connection_t*)args;
        int l_sock_client = sock;
      /*   pollfd l_readpoll[ 1 ];

        l_readpoll[ 0 ].fd = l_sock_client;
        l_readpoll[ 0 ].events = POLLIN;  */ 
        
        
        
	log_msg( LOG_INFO, "Process ID %u started operating at socket %d",getpid(),sock );
        getpeername( sock, ( sockaddr * ) &cl_addr, &lsa );
        log_msg( LOG_INFO, "Connected client IP: '%s'  port: %d",
        inet_ntoa( cl_addr.sin_addr ), ntohs( cl_addr.sin_port ) );
    
        
         SSL*     ssl;
  	X509*    client_cert;
  	char*    str;
  	int err;
        
        
	
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
  			 
  			
             }
			
			
		
			const char *test_req = "test";
			const char *test_ack = "ready";
		
			char buf [4096];
		
			err = SSL_read( ssl, buf, sizeof( buf ) );					CHK_ERR( err, "Unable get test req" );
		
			if ( strncmp( buf, test_req, strlen( test_req ) ) )
			{
				eprintf( "Bad test req." );
				ACTIVE = 0;
			}
			else
			{
				err = SSL_write( ssl, test_ack, strlen( test_ack ) );		CHK_ERR( err, "Unable to send test ack" );
				printf( "Sent test ack." );
			}
		
			/* DATA EXCHANGE - Receive message and send reply. */
		
			long total_bytes = 0;
			long num_lines = 0;
			while ( ACTIVE )
			{
				// read one line
				err = ssl_readline (ssl, buf, sizeof(buf) - 1);					  CHK_SSL(err);
				if ( err <= 0 ) break;
				//printf("LINE: %s",buf);
				total_bytes += err;
		
				// parse line
				int tmp, parsed, buf_inx;
				long sum = 0;
				sscanf( buf, "(%d)%n", &tmp, &buf_inx );
				while ( 1 == sscanf( buf + buf_inx, "%d%n", &tmp, &parsed  ) )
				{
					sum += tmp;
					buf_inx += parsed;
				}
				sum /= 2;
		
				// send answer
				sprintf( buf, "%ld %ld %ld\n", num_lines++, sum, total_bytes );
				err = SSL_write( ssl, buf, strlen( buf ) );						CHK_SSL(err);
			}
		
			/* Clean up. */
				








 SSL_free (ssl); 
printf("Process %d exiting\n",getpid());
exit(0);
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
	//int semval=-1;
	//vectsem = sem_open( "/vectsem", O_CREAT , 0660,1 );//robo
	//sem_init( vectsem, 1, 1 );//semctl( robosem, 0, SETVAL, 1);
	//sem_getvalue(vectsem,&semval);
	//printf( "Initial robosem: %d\n", semval );

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
	int spair[2];
	 ErrorCheck(socketpair(AF_UNIX, SOCK_DGRAM, 0, spair) );
	 
	 	 // SSL preliminary
	 
	
	const SSL_METHOD *meth;
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
 
	 
	 
	 
    // go!
    while ( ACTIVE )
    {
        //int l_sock_client = -1;

        // list of fd sources
        pollfd l_read_poll[ 2 ];

  
        l_read_poll[ 0 ].fd = l_sock_listen;
        l_read_poll[ 0 ].events = POLLIN;
		l_read_poll[ 1 ].fd = spair[0];
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
            { // new client?
            
            
            	//connection_t *conn = (connection_t*)malloc(sizeof(connection_t));
				//conn->sock=-1;
				//conn->pid=-1;
				//conn->address.sin_family = AF_INET;
				//conn->address.sin_addr = l_addr_any;
				//conn->addr_len = sizeof(conn->address);
						sockaddr_in cl_address;
				socklen_t cl_addr_len;
				cl_address.sin_family = AF_INET;
				cl_address.sin_addr = l_addr_any;
					cl_addr_len = sizeof(cl_address);
             
                int acc_sock = accept( l_sock_listen, (sockaddr*)&cl_address, &cl_addr_len );
                if ( acc_sock == -1 )
                {
                        log_msg( LOG_ERROR, "Unable to accept new client." );
                        ErrorCheck(close( acc_sock ));
//                        free(conn);
                        exit( 1 );
                }
                
                
				else
				{
				//process creation
					log_msg( LOG_INFO, "New client accepted" );
		
					unsigned int pid=fork();
					//conn->tid = tid;
					
					//sem_wait(vectsem);
					//conns[conn->tid]=conn;
					//sem_post(vectsem);
					//TODO add to list
					//int pthres =pthread_create(&tid,NULL,&handleclient,(void*)conn);
					if(pid < 0)
					{
						log_msg( LOG_DEBUG, "Process was not created,error %d:%s",errno,strerror(errno) );
						ErrorCheck(close( acc_sock ));
						
					}
					else if (pid > 0)
					{
						
						
						
						clients[pid] = acc_sock;
						printf("PARENT: Launched process %d\n",pid);
						ErrorCheck(close( acc_sock ));
					}
					else
					{
						handleclient(acc_sock,spair[1]);
						
					
					}
					
					
				}

		
		
		
		
		
		
                break;
            }
            
            
           
		if ( l_read_poll[ 1].revents & POLLIN )
		 {//data from client?
		 	char l_buf[1024];
		 	int l_len = read( spair[0], l_buf, sizeof( l_buf ) );
                	if ( !l_len )
	                {
	                        log_msg( LOG_DEBUG, "PARENT: Client closed socket!" );
		                        break;
	                }
	                else if ( l_len < 0 )
                        	log_msg( LOG_DEBUG, "PARENT: Unable to read data from client." );
	                else
	                        log_msg( LOG_DEBUG, "PARENT: Read %d bytes from client.", l_len );


        
		  		
		  	
		 }
		
		
            
            
            
            
            
            int status=-1;
            int pid=-1;
            int wpid=-1;
          while((wpid= waitpid(pid, &status ,WNOHANG)  )>0)
              {
                       ErrorCheck(close( clients[wpid]));
			
                           
                           
              }

        } // while wait for client


    } // while ( 1 )
    ErrorCheck(close( solnum ));
     SSL_CTX_free (ctx);    
   
    return 0;
}
