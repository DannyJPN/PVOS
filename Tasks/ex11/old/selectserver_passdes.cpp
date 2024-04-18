

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
using namespace std;
// debug flag
int debug = LOG_INFO;

int ACTIVE = 1;
typedef struct
{
    unsigned short family;
	int sock;
	sockaddr_in inaddress;
    sockaddr_un unaddress;
	socklen_t addr_len;
	
} connection_t;




	//intialization



struct innerreader
{
	int size;
	char* buffer;
	int index;
	int fd;
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
int readline(void*buf,int len,int timeout_ms,innerreader*inread)
{
    struct timeval selecttimeout;
    selecttimeout.tv_sec = timeout_ms/1000;
    selecttimeout.tv_usec = (timeout_ms%1000)*1000;
    int fd = inread->fd;
    
    
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
            if ( readyfds < 0 ) 
            {
					log_msg( LOG_ERROR, "Select failed!" );
					break;
				
            };
            if(readyfds == 0)
            {
                printf("timeout expired\n");
                return -2;
            }
            if ( FD_ISSET( fd ,&target ) )
            {
                int ret	= read( fd, &inread->buffer[inread->index], (inread->size)-(inread->index));
                if(ret <0)
				{return ret;}
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
	char *result=new char[32];
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
		
		char* token;
		while(tokidx<tokencount)
		{
			token = strtok(NULL," ");
            if(token == NULL)
            {
                
                break;
            }
			int item = atoi(token);
			//printf("%d ",item);
			readnums[tokidx-1] = item;
			
			sum+=item;
			tokidx++;
		}
		int readsum = -1;
        token=strtok(NULL," ");
        if(token !=NULL)
        {
            readsum=atoi(token);
		}
		
		
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
            connection_t* conn = (connection_t*)args;
			sockaddr_in cl_iaddr;
			sockaddr_un cl_uaddr;
            uint lsa;
            if(conn->family == AF_INET)
            {
                cl_iaddr.sin_family = AF_INET;
                lsa= sizeof( cl_iaddr );
            }
            else if(conn->family == AF_UNIX)
            {
                cl_uaddr.sun_family = AF_UNIX;
                lsa= sizeof( cl_uaddr );
            }
            
             
			int len = 2048;
			char buf[len]={'\0'};
			int ti = 500000;
			
			innerreader inread;
			inread.size = 1000;
			inread.buffer = new char[inread.size];
			inread.index=0;
		
            inread.fd = conn->sock;
			log_msg( LOG_INFO, "Thread  started operating at socket %d",conn->sock );
             if(conn->family == AF_INET)   
             { 
                getpeername( conn->sock, ( sockaddr * ) &cl_iaddr, &lsa );
                log_msg( LOG_INFO, "Connected client IP: '%s'  port: %d",
                                 inet_ntoa( cl_iaddr.sin_addr ), ntohs( cl_iaddr.sin_port ) );
             }
             else if(conn->family == AF_UNIX)
             {
                 log_msg( LOG_INFO, "Connected client %s" ,cl_uaddr.sun_path);
                 
             }
             
            while(ACTIVE)
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
					break;
				}
 
                
                
			 if ( FD_ISSET( conn->sock, &sockset ) )
				{
					// read data from socket
					int l = readline(  buf, len, ti,&inread);
                    //log_msg(LOG_INFO,"Read Value %d",l);
					if ( !l )
					{
                            if(conn->family == AF_INET)  
							{log_msg( LOG_INFO, "Client %s:%d closed socket!",inet_ntoa( cl_iaddr.sin_addr ) , ntohs( cl_iaddr.sin_port ));}
                            else if(conn->family == AF_UNIX)
                            {log_msg( LOG_INFO, "Client %s closed socket!",cl_uaddr.sun_path);}
							
                            log_msg( LOG_INFO, "Connection closed.  Socket %d closing" ,conn->sock);

							
							
							delete[] inread.buffer;
							
							break;
					}
					else if ( l < 0 )
					{
						log_msg( LOG_ERROR, "Unable to read data from client %s:%d." ,inet_ntoa( cl_iaddr.sin_addr ) , ntohs( cl_iaddr.sin_port ));

						
					}
					else
					{
                        if(conn->family == AF_INET) 
						{log_msg( LOG_INFO, "%s:%d - %s", inet_ntoa( cl_iaddr.sin_addr ) , ntohs( cl_iaddr.sin_port ),buf);}
                        else if(conn->family == AF_UNIX)
                        {log_msg( LOG_INFO, "%s - %s",cl_uaddr.sun_path,buf);}

						char separator=' ';
                        char* result = CheckState(buf,l,separator);

						int sendlen = write(conn->sock,result,strlen(result));
						ErrorCheck(sendlen);	
						delete[] result;
	
					}
	

					
				

							
					
				

					
                        
                    
				}




			}

            ErrorCheck(close(conn->sock));
			delete [] inread.buffer;

return NULL;








}


void cleaner(int signum)
{
	ACTIVE = 0;

}



void SendDescriptor(int spairend,int fdtosend,unsigned short family)
{
	
	    char myinfo[ 16 ];
        if(family == AF_INET)
        {    
            sprintf( myinfo, "INET4"  );
        }
        else if (family == AF_UNIX)
        {
            sprintf( myinfo, "UNIX"  );
        }
        printf( "%s\n", myinfo );



        msghdr msg;
        msg.msg_name = nullptr;
        msg.msg_namelen = 0;
    
        iovec ivec;
        ivec.iov_base = myinfo;
        ivec.iov_len = strlen( myinfo );

        msg.msg_iov = &ivec;
        msg.msg_iovlen = 1;

        char msgbuf[ CMSG_SPACE( sizeof( int ) ) ];
        cmsghdr *cmsg = ( cmsghdr * ) msgbuf;
        cmsg->cmsg_len = CMSG_LEN( sizeof( int ) );
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        * ( int * ) CMSG_DATA( cmsg ) = fdtosend;

        msg.msg_control = cmsg;
        msg.msg_controllen = CMSG_SPACE( sizeof( int ) );
         
        msg.msg_flags = 0;

        printf( "Posilame desctiptor %d\n", fdtosend );

        int ret = sendmsg( spairend, &msg, 0 );
        if ( ret < 0 )
            printf( "sendmsg error %d (%s)\n", errno, strerror( errno ) );

        
 

	
	
	
}

int GetDescriptor(int spairend,unsigned short * family)
{
	 msghdr msg;
        msg.msg_name = nullptr;
        msg.msg_namelen = 0;
    
        char info[ 64 ];
        iovec ivec;
        ivec.iov_base = info;
        ivec.iov_len = sizeof( info );

        msg.msg_iov = &ivec;
        msg.msg_iovlen = 1;

        char msgbuf[ CMSG_SPACE( sizeof( int ) ) ];
        cmsghdr *cmsg = ( cmsghdr * ) msgbuf;
        cmsg->cmsg_len = CMSG_LEN( sizeof( int ) );
         
        msg.msg_control = cmsg;
        msg.msg_controllen = CMSG_SPACE( sizeof( int ) );
         
        msg.msg_flags = 0;

        int ret = recvmsg( spairend, &msg, MSG_DONTWAIT );
        if ( ret < 0 )
        {
            if(errno != 11)
            {printf( "recvmsg error %d (%s)\n", errno, strerror( errno ) );}
            return -1;
        }

        int fd = * ( int * ) CMSG_DATA( cmsg );

        info[ ret ] = 0; // C string
        if(!strcmp(info,"UNIX"))
        {
            *family=AF_UNIX;
            
        }
        else if(!strcmp(info,"INET4"))
        {
            *family=AF_INET;
            
        }
        
	return fd;
}


void ChildCode(int c,int sopairend)
{
    
    vector<connection_t*> netcons;
        
        while(ACTIVE)
        {
        unsigned short fam;
		int fd =GetDescriptor(sopairend,&fam);
        
        if(fd >0)
        {
            connection_t* conn = (connection_t*)malloc(sizeof(connection_t));
            conn->family = fam;
            conn->sock = fd;
            netcons.push_back(conn);
        }
		
		if(netcons.size() < 1)
        {continue;}
		
		fd_set netset;
			for(unsigned int i = 0;i<netcons.size();i++)
			{
									
						// empty set
				FD_ZERO( &netset );
				
				// add listen socket
				FD_SET( netcons.at(i)->sock, &netset );
			}
		
			int sel = select( netcons.at(netcons.size()-1)->sock + 1, &netset, NULL, NULL, NULL );
			ErrorCheck(sel);
				if ( sel < 0 )
				{
					log_msg( LOG_ERROR, "Select failed!" );
					exit( 1 );
				}
				else
				{
					log_msg( LOG_DEBUG, "Select succeeded!" );
					
				}
			for(unsigned int i = 0;i<netcons.size();i++)
			{
				
				
	
				if(FD_ISSET( netcons.at(i)->sock, &netset ))
				{
					handleclient((void*)netcons.at(i));
                    free(netcons.at(i));
					netcons.erase(netcons.begin()+i);
                    
				}
			}
		
		
		
        }
    
}




//*************************************************************************
int main( int argn, char **arg )
{
	
	
    if ( argn <= 1 ) help( argn, arg );

    int port = 0;
	signal(SIGINT, cleaner);
    char unixpath[32]; 
	const int processcount = 10;

	
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
            sprintf(unixpath,"/tmp/kru0142_%d.UNET",port);
            break;
        }
    }

    if ( port <= 0 )
    {
        log_msg( LOG_INFO, "Bad or missing port number %d!", port );
        help( argn, arg );
		exit(2);
    }

    
log_msg( LOG_INFO, "Server will listen on IPv4 port: %d and unix socket %s", port,unixpath );
    //INET
    // INET socket creation
	int lastused =0;
	int sopairs[processcount][2];
	int pid=-1;
	int i;
	for ( i = 0; i < processcount; i++ )
	{
		socketpair( AF_UNIX, SOCK_STREAM, 0, sopairs[i] );
		
	}
	for ( i = 0; i < processcount; i++ )
	{
		pid = fork();
		if ( pid == 0 ) break;
	}
	if(pid >0)
	{
		
		
		int sock_listen = socket( AF_INET, SOCK_STREAM, 0 );
	
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
		
		
		//UNIX SOCKET
		unlink(unixpath);
		int usock_listen = socket(AF_UNIX, SOCK_STREAM, 0);
	
		sockaddr_un saun;
		saun.sun_family = AF_UNIX;
		strncpy( saun.sun_path, unixpath ,UNIX_MAXLEN);
		int ubindres=bind( usock_listen, ( sockaddr * ) &saun, sizeof( saun ) );
		ErrorCheck(ubindres);
		if(ubindres<0)
		{        
			log_msg( LOG_ERROR, "Bind UNIX failed!" );
			close( usock_listen );
			close( sock_listen );
			
			exit( 1 );
		}
		
		int ulistenres= listen( usock_listen, 10 );
		if ( ulistenres < 0 )
		{
			log_msg( LOG_ERROR, "Unable to listen on given UNIX socket!" );
			close( usock_listen );
			close( sock_listen );
			
			exit( 1 );
		}
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
				FD_SET( usock_listen, &read_wait_set );
				
				
				int sel = select( usock_listen + 1, &read_wait_set, NULL, NULL, NULL );
				ErrorCheck(sel);
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
					conn->family = AF_INET;
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
						log_msg( LOG_INFO, "New IPv4 client accepted" );
									// set for handles
					
						SendDescriptor(sopairs[lastused][1],conn->sock,conn->family);
						lastused++;
						if(lastused >= processcount)
						{lastused=0;}
                         free(conn);
	
	
					}
	
					break;
				}
				
				if ( FD_ISSET( usock_listen, &read_wait_set ) )
				{ // new client?
					connection_t *conn = (connection_t*)malloc(sizeof(connection_t));
					conn->sock=-1;
					conn->family=AF_UNIX;
					conn->unaddress.sun_family = AF_UNIX;
					//conn->address.sin_addr = addr_any;
					conn->addr_len = sizeof(conn->unaddress);
					//sockaddr_in rsa;
					//int rsa_size = sizeof( rsa );
					// new connection
					//sock_client = accept( sock_listen, ( sockaddr * ) &rsa, ( socklen_t * ) &rsa_size );
					conn->sock = accept( usock_listen, (sockaddr*)&conn->unaddress, &conn->addr_len );
					ErrorCheck(conn->sock);
					if ( conn->sock == -1 )
					{
							log_msg( LOG_ERROR, "Unable to accept new UNIX client." );
							ErrorCheck(close( conn->sock ));
							free(conn);
							exit( 1 );
					}
					else
					{
						log_msg( LOG_INFO, "New UNIX client accepted" );
					
						SendDescriptor(sopairs[lastused][1],conn->sock,conn->family);
						lastused++;
						if(lastused >= processcount)
						{lastused=0;}
							
	
	
					}
	
					break;
				}
	
					
				
			} // while wait for client
	
	
			
			
	
			
			
				
			
			
			
			
			
			
	
		} // while ( 1 )
	
	
	
		
	
		ErrorCheck(close( sock_listen ));
		ErrorCheck(close( usock_listen ));
		
	
	}
	else
	{
		ChildCode(i,sopairs[i][0]);
		exit(0);
	}
		return 0;
}
