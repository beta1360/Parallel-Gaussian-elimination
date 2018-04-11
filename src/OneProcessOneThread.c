#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

int main(int argc,char *argv[]){
	int a_int,b_int;
	int fd1,fd2,fd3;
	float **A;
	float *B;
	float *C;
	int l,i,j,k;
	struct timeval start,end;
	double checkTime;
	float temp_g,temp_b;

	if(argc !=4){
		printf("Need 4 tokens!");
		return -1; 
	}  // Error Code (not 4 tokens)
	gettimeofday(&start,NULL);

	fd1 = open(argv[1],O_RDONLY);
	fd2 = open(argv[2],O_RDONLY);

	read(fd1, &(a_int), sizeof(int));

	A = (float **)malloc(sizeof(float *)*a_int);
	for(i=0; i<a_int; i++)
		A[i] = (float *)malloc(sizeof(float)*a_int);	

	for(i=0; i<a_int; i++){
		for (j=0 ; j<a_int; j++){
			lseek(fd1,sizeof(int) + sizeof(float)*(i*a_int + j), SEEK_SET);
			read(fd1,&(A[i][j]),sizeof(float));
		}
	}	// Read 'MATRIX A'

	read(fd2, &(b_int), sizeof(int));

	B = (float *)malloc(sizeof(float)*b_int);	

	for(k=0; k<b_int; k++){
		lseek(fd2,sizeof(int)+sizeof(float)*k,SEEK_SET);
		read(fd2,&(B[k]),sizeof(float));
	}	// Read 'Vector B'

	C = (float *)malloc(sizeof(float)*b_int);

	gettimeofday(&end,NULL);
	checkTime = (double)(end.tv_sec)+(double)(end.tv_usec)/1000000.0-(double)(start.tv_sec)-(double)(start.tv_usec)/1000000.0;
	printf("[1] Time to read 2 data 'a.dat' and 'b.dat' : %f sec\n",checkTime);
	gettimeofday(&start,NULL);	

	for(i=0;i<a_int-1;i++){
		for(j=i+1;j<a_int;j++){
			temp_g = A[j][i]/A[i][i]; 
			for(l=i;l<a_int;l++){
				A[j][l] = A[j][l] - temp_g*A[i][l]; 
			}
			B[j] = B[j] - temp_g*B[i];
		}
	}	// Gaussian

	C[a_int-1] = B[b_int-1]/A[a_int-1][a_int-1];
	for(i=b_int-2;i>=0;i--){
		temp_b = 0;
		for(j=i-1;j<a_int;j++){
			temp_b += A[i][j]*C[j];
		}
		C[i] = (B[i] - temp_b)/A[i][i];
	}	// Back Substitution
	
	gettimeofday(&end,NULL);
	checkTime = (double)(end.tv_sec)+(double)(end.tv_usec)/1000000.0-(double)(start.tv_sec)-(double)(start.tv_usec)/1000000.0;
	printf("[2] Time to resolve this simultaneous equations : %f sec\n",checkTime);
	
	gettimeofday(&start,NULL);	
			
	fd3 = open(argv[3],O_WRONLY | O_CREAT | O_TRUNC, 0664);
	write(fd3,&(b_int),sizeof(int));
	B = (float*)malloc(sizeof(float)*b_int);
	for(i=0; i<b_int; i++){
		lseek(fd3,sizeof(int)+sizeof(float)*i,SEEK_SET);
		write(fd3,&(C[i]),sizeof(float));
	}

	gettimeofday(&end,NULL);
	checkTime = (double)(end.tv_sec)+(double)(end.tv_usec)/1000000.0-(double)(start.tv_sec)-(double)(start.tv_usec)/1000000.0;
	printf("[3] Time to write this result(vector) on file 'c.dat' : %f sec\n",checkTime);
	for(i=0;i<a_int;i++)
		free(A[i]);
	free(A); free(B); free(C);	

	close(fd1); close(fd2); close(fd3);
	return 0;
}
