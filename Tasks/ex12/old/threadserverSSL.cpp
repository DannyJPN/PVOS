
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
#include <sys/socket.h>
#include <sys/un.h>
#define UNIX_MAXLEN 108
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


#include <openssl/rsa.h>      
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


/* define HOME to be dir for key and cert files... */
#define HOME "./"
/* Make these what you want for cert & key files */
#define CERTF  HOME "myserver.crt"
#define KEYF  HOME  "mypriv.pem"


#define CHK_NULL(x) if ((x)==NULL) exit (1)
#define CHK_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(2); }


using namespace std;
// debug flag
int debug = LOG_INFO;
int solnum = -1;
int ACTIVE = 1;
typedef struct
{
    sa_family_t family;
	int sock;
	sockaddr_in inaddress;

	socklen_t addr_len;
	SSL *ssl;
	SSL_CTX* ctx;
} connection_t;




	//intialization
vector<pthread_t> threads;




 
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
int readline_SSL(SSL *ssls,char*buf,int len,int timeout_ms)
{
int fd = SSL_get_fd(ssls);
//fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK );
struct timeval delay;
delay.tv_sec = timeout_ms/1000;
delay.tv_usec = (timeout_ms%1000)*1000;

setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &delay, sizeof(delay)); 
    int readchars = 0;
    while ( 1 )
    {
        
        int ret = SSL_read( ssls, buf+readchars, 1);
        if ( ret == 0 ) break;
        if ( ret < 0 )
        {
            if ( errno == EAGAIN ) {  continue; }
            break;
        }
        if(buf[readchars] == '\n')
        {
            break;
        }
        if(readchars >= len-1)
        {
            break;
        }


    }
    buf[readchars]=0;
    return readchars;

}

//****************************************************************************

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
            connection_t* conn = (connection_t*)args;
			sockaddr_in cl_iaddr;
		
            uint lsa;
            if(conn->family == AF_INET)
            {
                cl_iaddr.sin_family = AF_INET;
                lsa= sizeof( cl_iaddr );
            }

             
			int len = 2048;
			char buf[len]={'\0'};
			int ti = 500000;
			

			log_msg( LOG_INFO, "Thread  started operating at socket %d",conn->sock );
             if(conn->family == AF_INET)   
             { 
                getpeername( conn->sock, ( sockaddr * ) &cl_iaddr, &lsa );
                log_msg( LOG_INFO, "Connected client IP: '%s'  port: %d",
                                 inet_ntoa( cl_iaddr.sin_addr ), ntohs( cl_iaddr.sin_port ) );
             }
   
			while(ACTIVE)
			{
				
			
					// read data from socket
					//int l = readline(  buf, len, ti,&inread);
                    int l = readline_SSL(conn->ssl,buf,len,ti);
                    //log_msg(LOG_INFO,"Read Value %d",l);
					if ( !l )
					{
                            if(conn->family == AF_INET)  
							{log_msg( LOG_INFO, "Client %s:%d closed socket!",inet_ntoa( cl_iaddr.sin_addr ) , ntohs( cl_iaddr.sin_port ));}

							
                            log_msg( LOG_INFO, "Connection closed.  Socket %d closing" ,conn->sock);

							
							
				
							
							break;
					}
					else if ( l < 0 )
					{
                        if(conn->family == AF_INET) 
						{log_msg( LOG_ERROR, "Unable to read data from client %s:%d." ,inet_ntoa( cl_iaddr.sin_addr ) , ntohs( cl_iaddr.sin_port ));}
      
						
					}
					else
					{
                        if(conn->family == AF_INET) 
						{log_msg( LOG_INFO, "%s:%d - %s", inet_ntoa( cl_iaddr.sin_addr ) , ntohs( cl_iaddr.sin_port ),buf);}
                  
					}
	
					

					
				

							
					
				

					
                        
                    
				




			}

            ErrorCheck(close(conn->sock));
			
			SSL_free(conn->ssl);
			SSL_CTX_free(conn->ctx);
			free(conn);
            threads.erase(remove(threads.begin( ), threads.end(), pthread_self()), threads.end());

return NULL;








}




void cleaner(int signum)
{
	ACTIVE = 0;

}


//*************************************************************************
int main( int argn, char **arg )
{
	
	
    if ( argn <= 1 ) help( argn, arg );

    int port = 0;
	signal(SIGINT, cleaner);
    


	
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
	int err;
	char*    str;
	SSL_CTX* ctx;
	SSL*     ssl;
	X509*    client_cert;
		const SSL_METHOD *meth;
	
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
	meth = SSLv23_server_method();
	ctx = SSL_CTX_new (meth);
    FILE*testfile;
    testfile=fopen(CERTF,"r");
    if(testfile!=NULL)
    {
        printf("Certfile success\n");
        fclose(testfile);
    }
    else
    {
        printf("Certfile fail\n");
 
    }
    testfile=fopen(KEYF,"r");
    if(testfile!=NULL)
    {
        printf("privkeyfile success\n");
        fclose(testfile);
    }
    else
    {
        printf("privkeyfile fail\n");
 
    } 
    
    
	if (!ctx) {
		ERR_print_errors_fp(stderr);
        printf("EXIT 2\n");
		exit(2);
	}
	
	if (SSL_CTX_use_certificate_file(ctx, CERTF, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		printf("EXIT 3\n");
        exit(3);
	}
	if (SSL_CTX_use_PrivateKey_file(ctx, KEYF, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
        printf("EXIT 4\n");
		exit(4);
	}
	
	if (!SSL_CTX_check_private_key(ctx)) {
		fprintf(stderr,"Private key does not match the certificate public key\n");
        printf("EXIT 5\n");
		exit(5);
	}



    log_msg( LOG_INFO, "Server will listen on IPv4 port: %d ", port);
    //INET
    // INET socket creation
    int sock_listen = socket( AF_INET, SOCK_STREAM, 0 );
	solnum = sock_listen;
    if ( sock_listen == -1 )
    {
        log_msg( LOG_ERROR, "Unable to create IPv4  socket.");
        exit( 1 );
    }

    in_addr addr_any = { INADDR_ANY };
    sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons( port );
    srv_addr.sin_addr = addr_any;

    // Enable the port number reusing
    int opt = 1;
    int sockoptres = setsockopt( sock_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) );
    if ( sockoptres < 0 )
      log_msg( LOG_ERROR, "Unable to set IPv4 socket option!" );
    ErrorCheck(sockoptres);
    // assign port number to socket
    int bindres=bind( sock_listen, (const sockaddr * ) &srv_addr, sizeof( srv_addr ) );
    if (bindres  < 0 )
    {
        log_msg( LOG_ERROR, "Bind IPv4 failed!" );
        close( sock_listen );
        exit( 1 );
    }
ErrorCheck(bindres);
    // listenig on set port
    int listenres= listen( sock_listen, 10 );
    if ( listenres < 0 )
    {
        log_msg( LOG_ERROR, "Unable to listen on given IPv4 port!" );
        close( sock_listen );
        exit( 1 );
    }
    ErrorCheck(listenres);
    //INET END
    
    


  
    ErrorCheck(listenres);

    
    //UNIX SOCKET END
    
    
    
    
    
    
    //log_msg( LOG_INFO, "Enter 'quit' to quit server." );

    // go!
    while ( ACTIVE )
    {
        //int sock_client = -1;

        while ( ACTIVE ) // wait for new client
        {
            // set for handles
            fd_set read_wait_set;
            // empty set
            FD_ZERO( &read_wait_set );
            
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
				
				conn->inaddress.sin_family = AF_INET;
				conn->inaddress.sin_addr = addr_any;
				conn->addr_len = sizeof(conn->inaddress);
                //sockaddr_in rsa;
                //int rsa_size = sizeof( rsa );
                // new connection
                //sock_client = accept( sock_listen, ( sockaddr * ) &rsa, ( socklen_t * ) &rsa_size );
                conn->sock = accept( sock_listen, (sockaddr*)&conn->inaddress, &conn->addr_len );
				ErrorCheck(conn->sock);
				if ( conn->sock == -1 )
                {
                        log_msg( LOG_ERROR, "Unable to accept new IPv4 client." );
                        ErrorCheck(close( conn->sock ));
						free(conn);
                        exit( 1 );
                }
				else
				{
					ssl = SSL_new (ctx);                           CHK_NULL(ssl);
					SSL_set_fd (ssl, conn->sock);
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
    
					    X509_free (client_cert);
					} else
						printf ("Client does not have certificate.\n");
					
					
					
					 log_msg( LOG_INFO, "New IPv4 client accepted" );
					pthread_t tid;
					
					//conn->sock = sock_client;
					threads.push_back(tid);
					conn->ssl=ssl;
					conn->ctx=ctx;
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



	
    ErrorCheck(close( sock_listen ));
 
    
    for(unsigned int i = 0;i<threads.size();i++)
    {
        pthread_join(threads[i],NULL);
        
    }


    return 0;
}
