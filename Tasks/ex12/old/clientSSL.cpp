//***************************************************************************

#include <sys/un.h>
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
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


#define CHK_NULL(x) if ((x)==NULL) exit (1)
#define CHK_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(2); }


#define UNIX_MAXLEN 108
#define STR_CLOSE               "close"

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
int debug = LOG_INFO;

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

void help( int argn, char **arg )
{
    if ( argn <= 1 ) return;

    if ( !strcmp( arg[ 1 ], "-h" ) )
    {
        printf(
            "\n"
            "  Socket client example.\n"
            "\n"
            "  Use: %s [-h -d] ip_or_name port_number\n"
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

int main( int argn, char **arg )
{

    if ( argn <= 2 ) help( argn, arg );

    int sock_server = -1;
    int port = 0;
    char *host = NULL;
	int err;
	
	SSL_CTX* ctx;
	SSL*     ssl;
	X509*    server_cert;
	char*    str;
	
	const SSL_METHOD *meth;
	
	SSLeay_add_ssl_algorithms();
	meth = SSLv23_client_method();
	SSL_load_error_strings();
	ctx = SSL_CTX_new (meth);                        CHK_NULL(ctx);
	
	CHK_SSL(err);
  



    // parsing arguments
    for ( int i = 1; i < argn; i++ )
    {
        if ( !strcmp( arg[ i ], "-d" ) )
            debug = LOG_DEBUG;

        if ( !strcmp( arg[ i ], "-h" ) )
            help( argn, arg );
        

        
        if ( *arg[ i ] != '-' )
        {
            if ( !host )
                host = arg[ i ];
            else if ( !port )
                port = atoi( arg[ i ] );
        }
        
        
    }

    if (( !host || !port ) )
    {
        log_msg( LOG_INFO, "Host or port is missing!" );
        help( argn, arg );
        exit( 1 );
    }
    

   
        log_msg( LOG_INFO, "Connection to '%s':%d.", host, port );

        addrinfo ai_req, *ai_ans;
        bzero( &ai_req, sizeof( ai_req ) );
        ai_req.ai_family = AF_INET;
        ai_req.ai_socktype = SOCK_STREAM;
    
        int get_ai = getaddrinfo( host, NULL, &ai_req, &ai_ans );
        if ( get_ai )
        {
        log_msg( LOG_ERROR, "Unknown host name!" );
        exit( 1 );
        }
    
        sockaddr_in cl_addr =  *( sockaddr_in * ) ai_ans->ai_addr;
        cl_addr.sin_port = htons( port );
        freeaddrinfo( ai_ans );
    
        // socket creation
        sock_server = socket( AF_INET, SOCK_STREAM, 0 );
        if ( sock_server == -1 )
        {
            log_msg( LOG_ERROR, "Unable to create socket.");
            exit( 1 );
        }
        else
        {
            log_msg( LOG_INFO, "Created socket %d.",sock_server);
    
        }
		int sd=connect( sock_server, ( sockaddr * ) &cl_addr, sizeof( cl_addr ) );
        // connect to server
        if (sd  < 0 )
        {
            log_msg( LOG_ERROR, "Unable to connect server." );
            exit( 1 );
        }
    
        uint lsa = sizeof( cl_addr );
        // my IP
        getsockname( sock_server, ( sockaddr * ) &cl_addr, &lsa );
        log_msg( LOG_INFO, "My IP: '%s'  port: %d",
                inet_ntoa( cl_addr.sin_addr ), ntohs( cl_addr.sin_port ) );
        // server IP
        getpeername( sock_server, ( sockaddr * ) &cl_addr, &lsa );
        log_msg( LOG_INFO, "Server IP: '%s'  port: %d",
                inet_ntoa( cl_addr.sin_addr ), ntohs( cl_addr.sin_port ) );
    
        // log_msg( LOG_INFO, "Enter 'close' to close application." );
        
        // go!
        
		ssl = SSL_new (ctx);                         CHK_NULL(ssl);    
		SSL_set_fd (ssl, sock_server);
		err = SSL_connect (ssl);                     CHK_SSL(err);
    
        
         printf ("SSL connection using %s\n", SSL_get_cipher (ssl));
  
		/* Get server's certificate (note: beware of dynamic allocation) - opt */
		
		server_cert = SSL_get_peer_certificate (ssl);       CHK_NULL(server_cert);
		printf ("Server certificate:\n");
		
		str = X509_NAME_oneline (X509_get_subject_name (server_cert),0,0);
		CHK_NULL(str);
		printf ("\t subject: %s\n", str);
		OPENSSL_free (str);
		
		str = X509_NAME_oneline (X509_get_issuer_name  (server_cert),0,0);
		CHK_NULL(str);
		printf ("\t issuer: %s\n", str);
		OPENSSL_free (str);
		
		/* We could do all sorts of certificate verification stuff here before
			deallocating the certificate. */
		
		X509_free (server_cert); 
        log_msg( LOG_INFO, "Press enter to start." );
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
	unsigned long long linenum=0;
    while ( 1 )
    {
        
        // select from handles
		char buf[512];
		//create sendline
		char exportbuf[256]={'\0'};

        time_t rawtime;
        struct tm * timeinfo;
        
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        sprintf (exportbuf ,"%s", asctime (timeinfo) );
        printf("%s\n",exportbuf);


		
		
        // send data to server
        int wl = SSL_write( ssl, exportbuf, strlen(exportbuf) );
        if ( wl < 0 )
		{
			log_msg( LOG_ERROR, "Unable to send  data to server." );
		}
        else
        {
			log_msg( LOG_DEBUG, "Sent %d bytes to server:\n%s", wl,exportbuf );
			linenum++;
            usleep(500);
		}    
// data on stdin?
 
        // data from server?
        	
			
            // read data from server
            int l = SSL_read( ssl, buf, sizeof( buf ) -1);
            if ( !l )
            {
                log_msg( LOG_DEBUG, "Server closed socket." );
                break;
            }
            else if ( l < 0 ) 
                log_msg( LOG_ERROR, "Unable to read data from server." );
            else
			
			{	
				log_msg( LOG_DEBUG, "Read %d bytes from server.", l );
				buf[l]=0;
				int wl = write( STDOUT_FILENO, buf, l );
				if ( wl < 0 )
                log_msg( LOG_ERROR, "Unable to write to stdout." );
	
			}
            // display on stdout
           

        

		
    }

    // close socket
    close( sock_server );
	close(sd);
	SSL_free(ssl);
	SSL_CTX_free(ctx);

    return 0;
  }
