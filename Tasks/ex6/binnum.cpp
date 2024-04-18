#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int FileWrite(char* filename,char* mode,double* array,int len)
{
	FILE *file;
	file=fopen(filename,mode);
	
	for (int i=0;i<len;i++)
	{
		int res =fwrite(&array[i],sizeof(array[i]),1,file);	
		if(res <0){ printf("problem %s\n",strerror(errno));return res;}
	}
	fclose(file);
	return len;
}




int main(int argc, char *argv[])
{

	if(argc < 2)
	{
	printf("Missing argument\n");return 1;
	}
    	srand((unsigned int)time(NULL));
	char filename[]="doubles.dat";
	char mode[]="w+b";
    	double max = 10.0;
	double min = 0.0;
	int count = atoi(argv[1]);
	double sum = 0;
	double arr[count+1];
	
    	for (int i=0;i<count;i++)
	{
		double result = ((double)rand()/(double)(RAND_MAX)) * (max-min) +min;
        	printf("Num: %lf\n", result);
		arr[i]=result;
		
		sum+=result;
    	}
	arr[count]=sum;
	FileWrite(filename,mode,arr,count+1);
	
	printf("---------------------------------\n");
	int infile = open( filename, O_RDWR );
    	if ( infile < 0 ) { printf( "Nelze otevrit soubor\n" ); return 1; }

    	long infile_len = lseek( infile, 0, SEEK_END );

   	double *data = ( double * ) mmap( 0, infile_len, PROT_READ | PROT_WRITE, MAP_PRIVATE, infile, 0 );
	double checksum=0;
	unsigned int i;
	  for(i = 0;i<infile_len/sizeof(double)-1;i++)
	  {
	  	checksum+=data[i];
	  	printf("FromFile %lf\n", data[i]);
	  
	  }

	printf("Initial sum %f\nRead sum %f\nRecomputed sum: %f\n",sum,data[i],checksum);
    
	
	munmap( data, infile_len );

	close( infile );
	return 0;
}
