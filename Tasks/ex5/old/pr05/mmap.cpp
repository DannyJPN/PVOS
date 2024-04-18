#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
    int fd = open( "data.dat", O_RDWR );
    if ( fd < 0 ) { printf( "Nelze otevrit soubor\n" ); return 1; }

    long fd_len = lseek( fd, 0, SEEK_END );

    char *data = ( char * ) mmap( 0, fd_len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0 );

    write( 1, data, fd_len );

    munmap( data, fd_len );

    close( fd );

}
