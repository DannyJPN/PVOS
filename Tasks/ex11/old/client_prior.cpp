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
#define UNIX_MAXLEN 108
#define STR_CLOSE               "close"

//***************************************************************************
// log messages

#define LOG_ERROR               0       // errors
#define LOG_INFO                1       // information and notifications
#define LOG_DEBUG               2       // debug messages
#define IPv4 0
#define UNIX 2
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
    int mode = IPv4;
    int sock_server = -1;
    int port = 0;
    char *host = NULL;
    char*sockpath=NULL;
    // parsing arguments
    for ( int i = 1; i < argn; i++ )
    {
        if ( !strcmp( arg[ i ], "-d" ) )
            debug = LOG_DEBUG;

        if ( !strcmp( arg[ i ], "-h" ) )
            help( argn, arg );
        
        if ( !strcmp( arg[ i ], "-u" ) )
            mode = UNIX;
        
        if ( *arg[ i ] != '-' && mode == IPv4)
        {
            if ( !host )
                host = arg[ i ];
            else if ( !port )
                port = atoi( arg[ i ] );
        }
        if ( *arg[ i ] != '-' && mode == UNIX)
        {
            if ( !sockpath )
                sockpath = arg[ i ];
        }
                
        
    }

    if (( !host || !port ) && mode ==IPv4)
    {
        log_msg( LOG_INFO, "Host or port is missing!" );
        help( argn, arg );
        exit( 1 );
    }
    
    if ( !sockpath  && mode ==UNIX)
    {
        log_msg( LOG_INFO, "Unix socket path is missing!" );
        help( argn, arg );
        exit( 1 );
    }
        
    if(mode == IPv4)
    {
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
    
        // connect to server
        if ( connect( sock_server, ( sockaddr * ) &cl_addr, sizeof( cl_addr ) ) < 0 )
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
        log_msg( LOG_INFO, "Press enter to start." );
        // go!
        }
        else if(mode == UNIX)
    {
        log_msg( LOG_INFO, "Connection to '%s'.", sockpath );


        sockaddr_un cl_addr;
        cl_addr.sun_family=AF_UNIX;
        strncpy(cl_addr.sun_path,sockpath,UNIX_MAXLEN);
        
        // socket creation
        sock_server = socket( AF_UNIX, SOCK_STREAM, 0 );
        ErrorCheck(sock_server);
        if ( sock_server == -1 )
        {
            log_msg( LOG_ERROR, "Unable to create socket.");
            exit( 1 );
        }
        else
        {
            log_msg( LOG_INFO, "Created socket %d.",sock_server);
    
        }
    
        // connect to server
        int conres = connect( sock_server, ( sockaddr * ) &cl_addr, sizeof( cl_addr ) );
        ErrorCheck(conres);
        if ( conres < 0 )
        {
            log_msg( LOG_ERROR, "Unable to connect server." );
            exit( 1 );
        }
    
        //uint lsa = sizeof( cl_addr );

        //getpeername( sock_server, ( sockaddr * ) &cl_addr, &lsa );
        log_msg( LOG_INFO, "Server unixpath: '%s'", cl_addr.sun_path);
    
        // log_msg( LOG_INFO, "Enter 'close' to close application." );
        log_msg( LOG_INFO, "Press enter to start." );
        // go!
        }
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
	unsigned long long linenum=0;
    while ( 1)
    {
		char buf [128];

        // set of handles
        fd_set read_wait_set;
        // clean set
        FD_ZERO( &read_wait_set );
        // add stdin
        FD_SET( STDIN_FILENO, &read_wait_set );
        // add socket
        FD_SET( sock_server, &read_wait_set );
        
        // select from handles
        int sel=select( sock_server + 1, &read_wait_set, 0, 0, 0 );
        ErrorCheck(sel);
        if ( sel < 0 ) break;
		//create sendline
		char exportbuf[512]={'\0'};
		int totallen=0;
		char outbuf[16];
		ErrorCheck(sprintf(outbuf,"%llu ",linenum));
		memcpy(exportbuf+totallen,outbuf,strlen(outbuf));
		totallen+=strlen(outbuf);
		
		
		int rannum = rand()%20 +1;
		//int * nums = new int[rannum];
		int sum=0;
		
		for(int i =0;i<rannum;i++)
		{
			int ran = rand()%100;
			ErrorCheck(sprintf(outbuf,"%d ",ran));
			memcpy(exportbuf+totallen,outbuf,strlen(outbuf));
			totallen+=strlen(outbuf);        
			//nums[i]=ran;
			sum+=ran;    
		}
		ErrorCheck(sprintf(outbuf,"%d\n",sum));
		memcpy(exportbuf+totallen,outbuf,strlen(outbuf));
		totallen+=strlen(outbuf);
		//create sendline end
		


		
		
        // send data to server
        int wl = write( sock_server, exportbuf, strlen(exportbuf) );
        ErrorCheck(wl);
        if ( wl < 0 )
		{
			log_msg( LOG_ERROR, "Unable to send random data to server." );
		}
        else
        {
			log_msg( LOG_DEBUG, "Sent %d bytes to server:\n%s", wl,exportbuf );
			linenum++;
            usleep(50000);
		}    

        if(linenum > 500)
        {
            char closebuf[]="!close";
            int cl = 0;
            
            if ( closebuf[ 0 ] == '!' )
            {
                ErrorCheck(send( sock_server, closebuf, 1, MSG_OOB ));
                cl = write( sock_server, closebuf + 1, strlen(closebuf) - 1 );
				break;
            }
            
            ErrorCheck(cl);
            if ( cl < 0 )
            {
                log_msg( LOG_ERROR, "Unable to send closure data to server." );
            }
            else
            {
                log_msg( LOG_DEBUG, "Sent %d bytes of closure to server:\n%s", wl,closebuf );
                
                
            } 
            
        }


        // data from server?
        if ( FD_ISSET( sock_server, &read_wait_set ) )
        {	
			
            // read data from server
            int l = read( sock_server, buf, sizeof( buf ) );
            if ( !l )
            {
                log_msg( LOG_DEBUG, "Server closed socket." );
                break;
            }
            else if ( l < 0 ) 
                log_msg( LOG_ERROR, "Unable to read data from server." );
            else
                log_msg( LOG_DEBUG, "Read %d bytes from server.", l );

            // display on stdout
            int wl = write( STDOUT_FILENO, buf, l );
            if ( wl < 0 )
                log_msg( LOG_ERROR, "Unable to write to stdout." );


        }

		
    }



    // close socket
    ErrorCheck(close( sock_server ));

    return 0;
  }
