#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <mqueue.h>

struct message
{
    long typ;
    union 
    {
        char c;
        int i;
    };
};

int main()
{
    int  msgfd = mq_open( "/fronta", O_CREAT | O_RDWR, 0660, nullptr );
    if ( msgfd < 0 )
    {
        printf( "No queue: %s\n", strerror( errno ) );
        return 1;
    }

    while ( 1 )
    {
        message z;
        z.typ = rand() % 2 + 1;
        if ( z.typ == 1 )
        {
            z.c = 'a' + rand() % 26;
            printf( "(%d) new message %c\n", getpid(), z.c );
        }
        if ( z.typ == 2 )
        {
            z.i = rand() % 10000;
            printf( "(%d) new message %d\n", getpid(), z.i );
        }
        int err = mq_send( msgfd, ( const char * ) &z, sizeof( z ), z.typ );
        if ( err < 0 )
        {
            printf( "message not sent: %s\n", strerror( errno ) );
        }

        usleep( 10000 );
    }
}
