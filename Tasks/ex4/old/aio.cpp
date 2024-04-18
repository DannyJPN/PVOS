#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <aio.h>

int main()
{
    char iobuf[ 222 ];
    aiocb cb; // toto ne!  = {...};
    cb.aio_fildes = 0;
    cb.aio_offset = 0;
    cb.aio_buf = iobuf;
    cb.aio_nbytes = sizeof( iobuf );
    cb.aio_reqprio = 0;
    cb.aio_sigevent.sigev_notify = SIGEV_NONE;

    aio_read( &cb );

    while ( 1 )
    {
        int status = aio_error( &cb );
        if ( status == EINPROGRESS ) 
        {
            fprintf( stderr, "nic se nedeje...\n" );
            usleep( 500000 );
        }
        else if ( status == 0 )
        {
            int ret = aio_return( &cb );
            if ( ret > 0 )
                write( 1, ( void * ) cb.aio_buf, ret );

            aio_read( &cb );
        }
    }

}

