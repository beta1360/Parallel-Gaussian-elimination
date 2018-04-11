#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <omp.h>

int main(int argc,char *argv[]){
	int x;						// for using index of normal situation
	int n;
	float *A,*B,*C;
	int idx;
	int p;
	int l,i,j;					// for using index of Gaussian & BackSubstitution
	int a_int,b_int;			// read integer of 'a.dat'(a_int) & integer of 'b.dat'(b_int)
	int fd1,fd2,fd3;			// file descripter
	struct timeval start,end; // for using gettimeofday()
	double checkTime;			// For checking time to resolve this simultaneous equations 

	// ---------------------------------- Sentence

	if(argc !=5){
		printf("Need 6 tokens!");
		return -1; 
	}  // Error Code (not 5 tokens) 

	p = atoi(argv[1]);	
	fd1 = open(argv[2],O_RDONLY);
	fd2 = open(argv[3],O_RDONLY);

	// ---------------------------------- Open tokens


	read(fd1, &(a_int), sizeof(int));

	A = (float *)malloc(sizeof(float)*a_int*a_int);

	for(x=0; x<a_int*a_int; x++){
		lseek(fd1,sizeof(int) + sizeof(float)*x, SEEK_SET);
		read(fd1,&(A[x]),sizeof(float));
	}	// Read 'MATRIX A'

	read(fd2, &(b_int), sizeof(int));

	B = (float *)malloc(sizeof(float)*b_int);	

	for(x=0; x<b_int; x++){
		lseek(fd2,sizeof(int)+sizeof(float)*x,SEEK_SET);
		read(fd2,&(B[x]),sizeof(float));
	}	// Read 'Vector B'

	C = (float *)malloc(sizeof(float)*b_int); 
	
	n = a_int = b_int; // Vector Size

	// ----------------------------------- Read Matrix&Vector / Make array A,B,C

	omp_set_num_threads(p);

	// ---------------------- openMP

	gettimeofday(&start,NULL);  // Checking start time
	
	for(l=0;l<=n-2;l++){
		int m = (n-1)-(l+1)+1;
		#pragma omp parallel for
			for(i = l+1+omp_get_thread_num()*m/p; i <= l+1+(omp_get_thread_num()+1)*m/p-1; i++){
				float ratio = A[i*n+l]/A[l*n+l];
				for(j = l; j < n; j++){
					A[i*n+j]=A[i*n+j]-ratio*A[l*n+j]; 
				} 
				B[i]=B[i]-ratio*B[l];
			}
	}

	
	// ------------------------------------ Gaussian Elimination

	for(i=n-1;i>=0;i--){
		C[i] = B[i]/A[i*n+i];
		#pragma omp parallel for
			for(j=omp_get_thread_num()*i/p; j<=(omp_get_thread_num()+1)*i/p-1; j++){
				B[j] = B[j] - C[i]*A[j*n+i];
				A[j*n+i] = 0;		
			}
	}

	// ------------------------------------ Back Substitution
	
	gettimeofday(&end,NULL);	// Checking end time
	checkTime = (double)(end.tv_sec)+(double)(end.tv_usec)/1000000.0-(double)(start.tv_sec)-(double)(start.tv_usec)/1000000.0;
	printf("- Time to resolve this simultaneous equations : %f sec\n",checkTime);
	// Printing result	

	fd3 = open(argv[4],O_WRONLY | O_CREAT | O_TRUNC, 0666);
	write(fd3,&n,sizeof(int));
	for(x=0; x<b_int; x++){
		lseek(fd3,sizeof(int)+sizeof(float)*x,SEEK_SET);
		write(fd3,&(C[x]),sizeof(float));
	}
	// Open 'c.dat' & Write Vector 'X'(Array C[n]) to 'c.dat'  

	free(A); free(B); free(C);
	close(fd1); close(fd2); close(fd3);
	return 0;
}

// ---------------------------------------- Write X Vector & main 

