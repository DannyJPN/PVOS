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

#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
using namespace std;
#define DOWNCMD "DOWN"
#define INITCMD "INIT"
#define UPCMD "UP"
#define UNLINKCMD "UNLINK"



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
int maxclients = 255;
int maxsems = 30;

//sem_t* vectsem;
typedef struct
{
	char name[128]="";
	sem_t sem;
	
} namedsem_t;
typedef struct
{
	char cmd [128];
	char name[128];
	int val=0;
	
	
} command;


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

/*sem_t getsem(namedsem_t* arr,char*name)
{
	for(int i = 0;i< maxsems;i++)
	{
		if(!strcmp(name,arr[i].name))
		{
			return arr[i].sem;
		}
		
	}
	return NULL;
}*/
int getsemidx(namedsem_t* arr,char*name)
{
	for(int i = 0;i< maxsems;i++)
	{
		if(!strcmp(name,arr[i].name))
		{
			return i;
		}
		
	}
	return -1;
}
int getfirstblank(namedsem_t* arr)
{
	for(int i = 0;i< maxsems;i++)
	{
		if(!strcmp("",arr[i].name))
		{
			return i;
		}
		
	}	
	return -1;
}

int DOWN(namedsem_t* arr,char* name)
{
	int current = getsemidx(arr,name);
	//printf("DOWN: index %d\n",current);
	if(current < 0)
	{
		return -1;
	}
	int semval;
	
	int err=sem_getvalue(&arr[current].sem,&semval);
	if(err >=0)
	{
		printf("DOWN: Semaphore %s (%16x) exists with val %d\n",name,&arr[current].sem,semval);
		
	}
	else
	{
		printf("DOWN: Semaphore problem %s\n",name);
		
	}	
	
	int wait = sem_wait(&arr[current].sem);;
	err=sem_getvalue(&arr[current].sem,&semval);
	printf("DOWN: Semaphore %s (%16x) changed to val %d\n",name,&arr[current].sem,semval);
	
	return wait;
	
}

int UP(namedsem_t* arr,char* name)
{
	
	int current = getsemidx(arr,name);
	//printf("UP: index %d\n",current);
	if(current <0)
	{
		return -1;
	}
		int semval;
	
	int err=sem_getvalue(&arr[current].sem,&semval);
	if(err >=0)
	{
		printf("UP: Semaphore %s (%16x) exists with val %d\n",name,&arr[current].sem,semval);
		
	}
	else
	{
		printf("UP: Semaphore problem %s\n",name);
		
	}
	int post = sem_post(&arr[current].sem);
	err=sem_getvalue(&arr[current].sem,&semval);
	printf("UP: Semaphore %s (%16x) changed to val %d\n",name,&arr[current].sem,semval);
	
	return post;
	
	
}

int INIT(namedsem_t* arr,char* name,int val)
{
	
	int current = getsemidx(arr,name);
	printf("INIT: index %d\n",current);
	printf("INIT: initval %d\n",val);
	/*if(current <0)
	{
		int semval;
		//sem_t cursem = arr[current].sem;
		//int err=sem_getvalue(&cursem, &semval);
		//if(err >=0)
		//{
			int index = getfirstblank(arr);
			printf("Semaphore %s (%16x) exists with val %d\n",name,&cursem,semval);
			namedsem_t ns;
			sprintf(ns.name,"%s",name);
			ns.sem=cursem;
			arr[index] = ns;
			int cur = getsemidx(arr,name);
			printf("INIT: Old Semaphore inserted at idx %d is truly at %d\n",index,cur);
		
			return 0;
			
		//}
	
	}*/
	
	if(current >=0)
	{
		return 0;
	}
	
	
	if(val <=0)
	{val=2;}
	//sem_t cursem ;//= sem_open( semname, O_CREAT , 0660,val );//robo
	int index = getfirstblank(arr);
	if(index > -1 && index < maxsems)
	{/*
		namedsem_t ns;
		sprintf(ns.name,"%s",name);
		int initer = sem_init( &cursem, 1, val );
		ns.sem=cursem;
		arr[index] = ns;*/
		
		sprintf(arr[index].name,"%s",name);
		int initer = sem_init( &arr[index].sem, 1, val );
		
		
		
		int cur = getsemidx(arr,name);
		printf("INIT: New Semaphore inserted at idx %d is truly at %d\n",index,cur);
		return initer;
	}
	else
	{
		printf("Semaphore limit reached\n");
		exit(2);
	}
	
	
	
	
}

int UNLINK(namedsem_t *arr,char* name)
{
	int currentidx = getsemidx(arr,name);
	if(currentidx <0)
	{
		return -1;
	}
	
	char semname[128]={0};
	sprintf(semname,"/%s",name);
	int err = sem_unlink(semname);
	
	sprintf(arr[currentidx].name,"%s","");
	
	
	
	return err;
	
	
}





void handleclient(int sock,namedsem_t* sharmem)
{

        sockaddr_in cl_addr;
	cl_addr.sin_family = AF_INET;
	uint lsa = sizeof( cl_addr );
	
        //connection_t* conn = (connection_t*)args;
        int l_sock_client = sock;
         pollfd l_readpoll[ 1 ];

        l_readpoll[ 0 ].fd = l_sock_client;
        l_readpoll[ 0 ].events = POLLIN;   
        
        
        
		log_msg( LOG_INFO, "Process ID %u started operating at socket %d",getpid(),sock );
        getpeername( sock, ( sockaddr * ) &cl_addr, &lsa );
        log_msg( LOG_INFO, "Connected client IP: '%s'  port: %d",
        inet_ntoa( cl_addr.sin_addr ), ntohs( cl_addr.sin_port ) );
    
       
        while ( ACTIVE  )
        { // communication
            char l_buf[ 1024 ];
			char responsebuf[1024];
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
                int l_len = readline( l_sock_client, l_buf, sizeof( l_buf ) );
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
				command c;
				
				//printf("Received command (%s)\n",l_buf);
				int scanned = sscanf( l_buf, "%s %s %d", c.cmd, c.name, &c.val );
				if ( scanned < 2 || scanned >3 )
					printf("Command %s incorrect\n",l_buf);
                
				
                if(!strcmp(c.cmd,DOWNCMD))
				{
					printf("Child ID %d: %s %s Request\n",getpid(),c.cmd,c.name);
					DOWN(sharmem,c.name);
					sprintf(responsebuf,"%s-OK\n",c.cmd);
					printf("Child ID %d: %s %s OK Answer\n",getpid(),c.cmd,c.name);
					
				}
				else if(!strcmp(c.cmd,UPCMD))
				{
					printf("Child ID %d: %s %s Request\n",getpid(),c.cmd,c.name);
					UP(sharmem,c.name);
					sprintf(responsebuf,"%s-OK\n",c.cmd);
					printf("Child ID %d: %s %s OK Answer\n",getpid(),c.cmd,c.name);
				}
				else if(!strcmp(c.cmd,INITCMD))
				{
					printf("Child ID %d: %s %s Request\n",getpid(),c.cmd,c.name);
					INIT(sharmem,c.name,c.val);
					sprintf(responsebuf,"%s-OK\n",c.cmd);
					printf("Child ID %d: %s %s OK Answer\n",getpid(),c.cmd,c.name);
				}
				else if(!strcmp(c.cmd,INITCMD))
				{
					printf("Child ID %d: %s %s Request\n",getpid(),c.cmd,c.name);
					UNLINK(sharmem,c.name);
					sprintf(responsebuf,"%s-OK\n",c.cmd);
					printf("Child ID %d: %s %s OK Answer\n",getpid(),c.cmd,c.name);
				}
				else
				{
					printf("Child ID %d: %s \n",getpid(),l_buf);
					sprintf(responsebuf,"ERR \"%s\" is not a command",l_buf);
					
				}
				writeline(l_sock_client,responsebuf,strlen(responsebuf));
			
		
		
			
			
			
            }
  
        } // while communication
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
	
	//prepare sharmem
	
	int sharmem_key = getuid();
	int sharmem_id = shmget(sharmem_key,maxsems*sizeof(namedsem_t),0644|IPC_CREAT);
	namedsem_t *sharmem=(namedsem_t*)shmat(sharmem_id,NULL,0);
	bzero(sharmem,maxsems*sizeof(namedsem_t));
	//sem_init(&sharmem[0].sem,1,1);
	//printf("Check init\n");
	//sem_wait(&sharmem[0].sem);
	//printf("Check waot\n");
	//sem_wait(&sharmem[0].sem);
	//printf("Check waot 2\n");
	
    // go!
    while ( ACTIVE )
    {
        //int l_sock_client = -1;
		int pollsize = 1;
        // list of fd sources
        pollfd l_read_poll[ pollsize];

        l_read_poll[ 0 ].fd = l_sock_listen;
        l_read_poll[ 0 ].events = POLLIN;
		
	
	
        while ( ACTIVE ) // wait for new client
        {
            // select from fds
            int l_poll = poll( l_read_poll, pollsize, -1 );

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
		
					if(pid < 0)
					{
						log_msg( LOG_DEBUG, "Process was not created,error %d:%s",errno,strerror(errno) );
						ErrorCheck(close( acc_sock ));
						
					}
					else if (pid > 0)
					{
						
						
						
						clients[pid] = acc_sock;
						printf("PARENT: Launched process %d\n",pid);
					}
					else
					{
						handleclient(acc_sock,sharmem);
					
					}
					
					
				}

		
		
		
		
		
		
                break;
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
    shmctl(sharmem_id, IPC_RMID, NULL);
	
   
    return 0;
}
