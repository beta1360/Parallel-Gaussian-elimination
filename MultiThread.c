#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>

pthread_t *thread;			// Thread Array
pthread_barrier_t barrier;	// Thread Barrier
float *A,*B,*C;				// Matrix & Vectors
int n;							// Size of Vector
int a,l;						// for static index (in main function & thread functions)
int p;							// Write argv[1] (This is the quantity of thread) 
int *pthread_number;			// For using thread number & index

void *Gaussian(void *index);				// Thread function for Gaussian Elimination
void *BackSubstitution(void *index);	// Thread function for Back Substitution

int main(int argc,char *argv[]){
	int x,k;						// for using index of non-thread;
	int status;				// for using parameter of pthread_join
	int a_int,b_int;			// read integer of 'a.dat'(a_int) & integer of 'b.dat'(b_int)
	int fd1,fd2,fd3;			// file descripter
	struct timeval start,end; // for using gettimeofday()
	double checkTime;			// For checking time to resolve this simultaneous equations 

	// ---------------------------------- Sentence

	if(argc !=5){
		printf("Need 5 tokens!");
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

	thread = (pthread_t *)malloc(sizeof(pthread_t)*p);
	pthread_barrier_init(&barrier,NULL,p);
	pthread_number = (int *)malloc(sizeof(int)*p);
	for(k=0;k<p;k++)
		pthread_number[k] = k;
	
	// ----------------------------------- Pthread & Pthread_barrier	

	gettimeofday(&start,NULL);  // Checking start time

	for(l=0;l<=n-1;l++){
		for(k=0;k<p;k++){
			if(pthread_create(&thread[k],NULL,Gaussian,(void *)&pthread_number[k])<0){
				printf("Fail pthread_create function!!\n");
				exit(0);
			}
		}	// Parallelized Gaussian Elimination by thread
		for(x=0;x<p;x++)
			pthread_join(thread[x],(void *)&status);
	}

	// ------------------------------------ Gaussian Elimination

	for(a=n-1;a>=0;a--){
		C[a] = B[a]/A[a*n+a];
		for(k=0;k<p;k++){
			if(pthread_create(&thread[k],NULL,BackSubstitution,(void *)&pthread_number[k])<0){
				printf("Fail pthread_create function!!\n");
				exit(0);
			}
		}	// Parallelized Back Substitution by thread
		for(x=0;x<p;x++)
			pthread_join(thread[x],(void *)&status);
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
	free(thread);	free(pthread_number);
	close(fd1); close(fd2); close(fd3);
	return 0;
}

// ---------------------------------------- Write X Vector & main 

void *Gaussian(void *index){
	float ratio;
	int i,j;
	int idx = *(int *)index;
	int m = (n-1)-(l+1)+1;

	for(i = l+1+idx*m/p; i <= l+1+(idx+1)*m/p-1; i++){
		ratio = A[i*n+l]/A[l*n+l];
		for(j = l; j < n; j++){
			A[i*n+j]=A[i*n+j]-ratio*A[l*n+j]; 
		} 
		B[i]=B[i]-ratio*B[l];
	}
	
	pthread_exit(0);
	pthread_barrier_wait(&barrier);
	return NULL;
}

// ----------------------------------------- Gaussian Elimination

void *BackSubstitution(void *index){
	int b;
	int idx = *(int *)index;

	for(b=idx*a/p; b<=(idx+1)*a/p-1; b++){
		B[b] = B[b] - C[a]*A[b*n+a];
		A[b*n+a] = 0;		
	}

	pthread_exit(0);
	pthread_barrier_wait(&barrier);
	return NULL;
}

// ------------------------------------------ Back Subtitution
