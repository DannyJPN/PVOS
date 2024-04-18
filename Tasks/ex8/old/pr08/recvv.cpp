#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct message
{
    long typ;
    union 
    {
        char c;
        int i;
    };
};

#define message_len ( sizeof( message ) - sizeof( long ) )


int main()
{
    int  msgid = msgget( 0xcaca0, IPC_CREAT | 0660 );
    if ( msgid < 0 )
    {
        printf( "No queue: %s\n", strerror( errno ) );
        return 1;
    }

    while ( 1 )
    {
        message z;
        int err = msgrcv( msgid, &z, message_len, 0, 0 );

        if ( err < 0 )
        {
            printf( "message not sent: %s\n", strerror( errno ) );
            return 1;
        }
        
        if ( z.typ == 1 )
        {
            printf( "(%d) recv message %c\n", getpid(), z.c );
        }
        if ( z.typ == 2 )
        {
            printf( "(%d) recv message %d\n", getpid(), z.i );
        }
    }
}
