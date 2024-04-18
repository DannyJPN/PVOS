//***************************************************************************
//
// Program example for subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2021
//
// Example of socket server/client.
//
// This program is example of socket client.
// The mandatory arguments of program is IP adress or name of server and
// a port number.
//
//***************************************************************************

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
#include <netdb.h>

#define DOWNCMD "DOWN"
#define INITCMD "INIT"
#define UPCMD "UP"
#define UNLINKCMD "UNLINK"

//***************************************************************************
// log messages

#define LOG_ERROR               0       // errors
#define LOG_INFO                1       // information and notifications
#define LOG_DEBUG               2       // debug messages

// debug flag
int g_debug = LOG_INFO;
/*
int readline( SSL *ssl, char *buf, int length )
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
}*/

int readline( int sock, char *buf, int length )
{
	int num = 0;
	while ( num < length - 1 )
	{
		int err = read( sock, buf + num, 1 );
		if ( err <= 0 ) return err;
		if ( buf[ num++ ] == '\n' ) break;
	}
	buf[ num ] = 0;
	return num;
}

/*
int writeline( SSL *ssl, char *buf, int length )
{
	int err = SSL_write( ssl, buf, length );
	return err;

}*/

int writeline( int sock, char *buf, int length )
{
	int err = write( sock, buf, length );
	return err;
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
            "  Socket client example.\n"
            "\n"
            "  Use: %s [-h -d] ip_or_name port_number\n"
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

int main( int t_narg, char **t_args )
{
	int lines = 1000;
    if ( t_narg <= 2 ) help( t_narg, t_args );

    int l_port = 0;
    char *l_host = nullptr;
	int l_initval = 0;
	char * l_semname = nullptr;

    // parsing arguments
    for ( int i = 1; i < t_narg; i++ )
    {
        if ( !strcmp( t_args[ i ], "-d" ) )
            g_debug = LOG_DEBUG;

        if ( !strcmp( t_args[ i ], "-h" ) )
            help( t_narg, t_args );

        if ( *t_args[ i ] != '-' )
        {
            if ( !l_host )
                l_host = t_args[ i ];
            else if ( !l_port )
                l_port = atoi( t_args[ i ] );
			else if ( !l_semname )
                l_semname =  t_args[ i ] ;
			else if ( !l_initval )
                l_initval = atoi( t_args[ i ] );
			
        }
    }

    if ( !l_host || !l_port )
    {
        log_msg( LOG_INFO, "Host or port is missing!" );
        help( t_narg, t_args );
        exit( 1 );
    }
	if(!l_semname)
	{
	    log_msg( LOG_INFO, "Semaphor name is missing!" );
        help( t_narg, t_args );
        exit( 1 );
    	
		
	}
	
    log_msg( LOG_INFO, "Connection to '%s':%d. \n Semaphor '%s' val %d", l_host, l_port ,l_semname,l_initval);

    addrinfo l_ai_req, *l_ai_ans;
    bzero( &l_ai_req, sizeof( l_ai_req ) );
    l_ai_req.ai_family = AF_INET;
    l_ai_req.ai_socktype = SOCK_STREAM;

    int l_get_ai = getaddrinfo( l_host, nullptr, &l_ai_req, &l_ai_ans );
    if ( l_get_ai )
    {
        log_msg( LOG_ERROR, "Unknown host name!" );
        exit( 1 );
    }

    sockaddr_in l_cl_addr =  *( sockaddr_in * ) l_ai_ans->ai_addr;
    l_cl_addr.sin_port = htons( l_port );
    freeaddrinfo( l_ai_ans );

    // socket creation
    int l_sock_server = socket( AF_INET, SOCK_STREAM, 0 );
    if ( l_sock_server == -1 )
    {
        log_msg( LOG_ERROR, "Unable to create socket.");
        exit( 1 );
    }

    // connect to server
    if ( connect( l_sock_server, ( sockaddr * ) &l_cl_addr, sizeof( l_cl_addr ) ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to connect server." );
        exit( 1 );
    }

    uint l_lsa = sizeof( l_cl_addr );
    // my IP
    getsockname( l_sock_server, ( sockaddr * ) &l_cl_addr, &l_lsa );
    log_msg( LOG_INFO, "My IP: '%s'  port: %d",
             inet_ntoa( l_cl_addr.sin_addr ), ntohs( l_cl_addr.sin_port ) );
    // server IP
    getpeername( l_sock_server, ( sockaddr * ) &l_cl_addr, &l_lsa );
    log_msg( LOG_INFO, "Server IP: '%s'  port: %d",
             inet_ntoa( l_cl_addr.sin_addr ), ntohs( l_cl_addr.sin_port ) );

    log_msg( LOG_INFO, "Enter 'close' to close application." );

    // list of fd sources
	int pollsize= 1;
    pollfd l_read_poll[ 1 ];

    l_read_poll[ 0 ].fd = l_sock_server;
    l_read_poll[ 0 ].events = POLLIN;

//init
	char cmd[1024]={0};
		
	if(l_initval)
	{
		sprintf(cmd,"%s %s %d\n",INITCMD,l_semname,l_initval);
		
	
	}

	else
	{
		sprintf(cmd,"%s %s\n",INITCMD,l_semname);
	
	}
	printf("START: (%s) Sent\n",cmd);
	writeline(l_sock_server,cmd,strlen(cmd));
	
        char l_buf[ 1024];
		char resp_buf[2048]={0};

		int l_len = readline( l_sock_server, l_buf, sizeof( l_buf ) );
		sprintf(resp_buf,"%s Received",l_buf);
        l_len = write( STDOUT_FILENO, resp_buf, l_len );
        if ( l_len < 0 )
            log_msg( LOG_ERROR, "Unable to write to stdout." );
	

    // go!
    while ( lines-- )
    {
		bzero(cmd,1024);
		if(lines <=0)
		{
			sprintf(cmd,"%s %s\n",UNLINKCMD,l_semname);
			writeline(l_sock_server,cmd,strlen(cmd));
			printf("END: %s Sent\n",cmd);
			
		}
		else
		{
			
			sprintf(cmd,"%s %s\n",DOWNCMD,l_semname);
			writeline(l_sock_server,cmd,strlen(cmd));
			printf("INF: (%s) Sent\n",cmd);
			
			
			
		}
		




        // select from fds
        if ( poll( l_read_poll, pollsize, -1 ) < 0 ) break;

        

        // data from server?
        if ( l_read_poll[ 0 ].revents & POLLIN )
        {
            // read data from server
            int l_len = readline( l_sock_server, l_buf, sizeof( l_buf ) );
            if ( !l_len )
            {
                log_msg( LOG_DEBUG, "Server closed socket." );
                break;
            }
            else if ( l_len < 0 )
                log_msg( LOG_DEBUG, "Unable to read data from server." );
            else
                log_msg( LOG_DEBUG, "Read %d bytes from server.", l_len );

            // display on stdout
			sprintf(resp_buf,"%s Received",l_buf);
            l_len = write( STDOUT_FILENO, resp_buf, l_len );
            if ( l_len < 0 )
                log_msg( LOG_ERROR, "Unable to write to stdout." );

            
        }
		
		for(int i = 0;i <15;i++)
		{
			printf("Robim %d\n",i);
			usleep(250*1000);
		
		}
		
		if(lines > 0)
		
		{
			sprintf(cmd,"%s %s\n",UPCMD,l_semname);
			writeline(l_sock_server,cmd,strlen(cmd));
			printf("INF: (%s) Sent\n",cmd);
		}
		else break;
			
		
		if ( poll( l_read_poll, pollsize, -1 ) < 0 ) break;

        

        // data from server?
        if ( l_read_poll[ 0 ].revents & POLLIN )
        {
            // read data from server
            int l_len = readline( l_sock_server, l_buf, sizeof( l_buf ) );
            if ( !l_len )
            {
                log_msg( LOG_DEBUG, "Server closed socket." );
                break;
            }
            else if ( l_len < 0 )
                log_msg( LOG_DEBUG, "Unable to read data from server." );
            else
                log_msg( LOG_DEBUG, "Read %d bytes from server.", l_len );

            // display on stdout
			sprintf(resp_buf,"%s Received",l_buf);
            l_len = write( STDOUT_FILENO, resp_buf, l_len );
            if ( l_len < 0 )
                log_msg( LOG_ERROR, "Unable to write to stdout." );

            
        }

	
	
		
		
    }
		
    // close socket
    close( l_sock_server );

    return 0;
  }
