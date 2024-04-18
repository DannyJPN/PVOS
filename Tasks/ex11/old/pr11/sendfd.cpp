// **************************************************************************
//
// Empty C++ project for education
//
// petr.olivka@vsb.cz, Dept. of Computer Science, FEECS, VSB-TU Ostrava, CZ
//
// **************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <arpa/inet.h>

int main( int argn, char **arg )
{

    int spair[ 2 ];
    socketpair( AF_UNIX, SOCK_STREAM, 0, spair );

    if ( fork() != 0 ) 
    { // rodic
        char myinfo[ 128 ];
        sprintf( myinfo, "Parent PID %d", getpid() ); 
        printf( "%s\n", myinfo );

        int roura[ 2 ];
        pipe( roura );
        pipe( roura );
        pipe( roura );

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
        * ( int * ) CMSG_DATA( cmsg ) = roura[ 0 ];

        msg.msg_control = cmsg;
        msg.msg_controllen = CMSG_SPACE( sizeof( int ) );
         
        msg.msg_flags = 0;

        printf( "Posilame desctiptor %d\n", roura[ 0 ] );

        int ret = sendmsg( spair[ 1 ], &msg, 0 );
        if ( ret < 0 )
            printf( "sendmsg error %d (%s)\n", errno, strerror( errno ) );

        char pozdrav[] = "Nazdar pres rouru\n";
        write( roura[ 1 ], pozdrav, strlen( pozdrav ) );

        getchar();
    }
    else
    { // potomek
        printf( "Child PID %d\n", getpid() );

        msghdr msg;
        msg.msg_name = nullptr;
        msg.msg_namelen = 0;
    
        char info[ 512 ];
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

        int ret = recvmsg( spair[ 0 ], &msg, 0 );
        if ( ret < 0 )
        {
            printf( "recvmsg error %d (%s)\n", errno, strerror( errno ) );
            return 0;
        }

        int fd = * ( int * ) CMSG_DATA( cmsg );

        info[ ret ] = 0; // C string
        printf( "prijata data: '%s'\n", info );
        printf( "prisel deskriptor fd %d\n", fd );

        char buf[ 112 ];
        int l = read( fd, buf, sizeof( buf ) );
        write( 1, buf, l );

    }

}   

