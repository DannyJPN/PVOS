#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/*
Omlouvám se za pozdní odevzdání,mám závažné technické potíže na své IT výbavě.
Funguju na nouzovém zařízení,jak jsem již zmínil v emailu.
Úkol č. 2 vyžaduje readline,takže ho budu muset dokončit.


*/


int main(int argc, char *argv[])
{
    srand((unsigned int)time(NULL));
	char filename[]="floats.dat";
    float max = 10.0;
	float min = 0.0;
	int count = 10;
	float sum = 0;
	FILE *outfile;
	outfile=fopen(filename,"w+b");
    for (int i=0;i<count;i++)
	{
		float result = ((float)rand()/(float)(RAND_MAX)) * (max-min) +min;
        printf("%f\n", result);
		
		fwrite(&result,sizeof(result),1,outfile);
		sum+=result;
    }
	fwrite(&sum,sizeof(sum),1,outfile);
	fclose(outfile);
	
	printf("---------------------------------\n");
	int infile = open( filename, O_RDWR );
    if ( infile < 0 ) { printf( "Nelze otevrit soubor\n" ); return 1; }

    long infile_len = lseek( infile, 0, SEEK_END );

   float *data = ( float * ) mmap( 0, infile_len, PROT_READ | PROT_WRITE, MAP_PRIVATE, infile, 0 );
float checksum=0;
unsigned int i;
  for(i = 0;i<infile_len/sizeof(float)-1;i++)
  {
	  checksum+=data[i];
	  printf("%f\n", data[i]);
	  
  }

	printf("Initial sum %f\nRead sum %f\nRecomputed sum: %f\n",sum,data[i],checksum);
    
	
	munmap( data, infile_len );

    close( infile );
	return 0;
}