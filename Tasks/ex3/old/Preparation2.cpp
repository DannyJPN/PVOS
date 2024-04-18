//***************************************************************************
//
// Program example for labs in subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2017
//
// The example of using pipe(), fork() and dup2() functions.
// The parent redirects stdout to pipe and the child redirects stdin from pipe.
//
//***************************************************************************//***************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/types.h>

//***************************************************************************
// log messages

#define LOG_ERROR       0       // errors
#define LOG_INFO        1       // information and notifications
#define LOG_DEBUG       2       // debug messages

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

void help( int num, char **arg )
{
    if ( num <= 1 ) return;

    if ( !strcmp( arg[ 1 ], "-h" ) )
    {
        printf(
            "\n"
            "  Stdio and stdout redirection. \n"
            "\n"
            "  Use: %s\n"
            "\n"
            "    -h  this help\n"
            "    -d  debug mode \n"
            "\n", arg[ 0 ] );

        exit( 0 );
    }

    if ( !strcmp( arg[ 1 ], "-d" ) )
        debug = LOG_DEBUG;
}
//***************************************************************************

void producer()
// Producer sends data to stdout.
{
    int count = 0;

    while ( 1 )
        fprintf( stdout, "%d\n", count++ );
}


void consumer()
// consumer reads data from stdio
{
    while ( 1 )
    {
        int i;
        scanf( "%d", &i );
        fprintf( stderr, "Consumer read: %d\n", i );
    }
}

int main( int num, char **arg )
{
    help( num, arg );

    int mypipe[ 2 ];

    if ( pipe( mypipe ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to create pipe!" );
        exit( 1 );
    }

    int child = fork();

    if ( child < 0 )
    {
        log_msg( LOG_ERROR, "Unable to create new process!" );
        exit( 1 );
    }

    if ( child != 0 )
    { // parent
        // redirection of stdin from pipe
        dup2( mypipe[ 1 ], STDOUT_FILENO );
        // close useless pipe
        close( mypipe[ 0 ] );
        close( mypipe[ 1 ] );

        producer();
    }
    else
    { // child
        // redirection of stdout to pipe
        dup2( mypipe[ 0 ], STDIN_FILENO );
        // close useless pipe
        close( mypipe[ 0 ] );
        close( mypipe[ 1 ] );

        consumer();
    }

    return 0;
}
