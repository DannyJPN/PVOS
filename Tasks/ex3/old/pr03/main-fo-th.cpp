#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int g_continue = 1;
int g_num_thread = 0;
int g_detach = 1;

void mysig( int sig )
{
    printf( "Ignore signal %d\n", sig );
}

void *vlakno( void * t_nic )
{
    if ( g_detach ) pthread_detach( pthread_self() );
    while ( g_continue ) sleep( 1 );
}

int main()
{
    // limits
    // sysctl vm.max_map_count
    // ulimit -u 
    // /sys/fs/cgroup/pids/user-UID.slice/pids.max
    // cat /proc/1/limits
    // grep -r TasksMax /etc
    // grep -r TasksMax /lib/systemd 
    int l_ret, l_pid = -1, l_num_fork = 0;
    struct sigaction sa;
    sa.sa_handler = mysig;
    sa.sa_flags = 0;
    sigemptyset( &sa.sa_mask );

    printf( "fork test...\n" );
    while ( 1 )
    {
        if ( ( l_pid = fork() ) == 0 ) { sleep( 1000 ); exit( 0 ); }
        if ( l_pid < 0 ) break;
        l_num_fork++;
    }
    printf( "num fork %d\n", l_num_fork );
    printf( "error %d %s\n", errno, strerror( errno ) );

    sigaction( SIGTERM, &sa, NULL );
    printf( "press Enter...\n" );
    getchar();
    printf( "kill ret %d\n", l_ret = kill( -getpgid( 0 ), SIGTERM ) );
    if ( l_ret < 0 ) printf( "error %d %s\n", errno, strerror( errno ) );
    while ( wait( NULL ) > 0 );
    printf( "press Enter...\n" );
    getchar();

    pthread_t l_pth;

    printf( "pthread test ... running threads...\n" );
    while ( ( l_ret = pthread_create( &l_pth, NULL, vlakno, NULL ) ) == 0 ) g_num_thread++;

    printf( "g_num_thread %d\n", g_num_thread );
    printf( "error %d %s\n", errno, strerror( errno ) );
    printf( "press Enter...\n" );
    getchar();
    printf( "finishing threads...\n" );
    g_continue = 0;
    g_detach = 0;
    sleep( 2 ); 
    printf( "after  sleep... \n" );

    printf( "pthread test ... nondetached threads...\n" );
    while ( ( l_ret = pthread_create( &l_pth, NULL, vlakno, NULL ) ) == 0 ) { usleep( 100 ); g_num_thread++; }

    printf( "g_num_thread %d\n", g_num_thread );
    printf( "error %d %s\n", errno, strerror( errno ) );
    printf( "press Enter...\n" );
    getchar();
}

