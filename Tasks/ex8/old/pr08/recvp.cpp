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

    mq_attr mqa;
    mq_getattr( msgfd, &mqa );

    while ( 1 )
    {
        char msg[ mqa.mq_msgsize ];
        message *z = ( message * ) msg;
        unsigned prio;
        int err = mq_receive( msgfd, msg, mqa.mq_msgsize, &prio );

        if ( err < 0 )
        {
            printf( "message not sent: %s\n", strerror( errno ) );
            return 1;
        }
        printf( "zprava %ld prio %d ", z->typ, prio ); 
        if ( z->typ == 1 )
        {
            printf( "(%d) recv message %c\n", getpid(), z->c );
        }
        if ( z->typ == 2 )
        {
            printf( "(%d) recv message %d\n", getpid(), z->i );
        }
    }
}
