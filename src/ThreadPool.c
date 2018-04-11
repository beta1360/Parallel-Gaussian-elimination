#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

#define SENTINEL -1

typedef struct {
	int x;
	int y;
}index;        // To manage send() or recv() parameters.

typedef struct {
	index *queue;
	int front, rear;
} Queue;

int queue_Size;                // Write argv[2] (This is the size of queue)
int p;                            // Write argv[1] (This is the quantity of thread) 
int n;                            // Size of Vector
float *A, *B, *C;                // Matrix & Vectors
Queue q;                        // Input thread work
pthread_t *thread;            // Thread Array
sem_t mutex, empty, full, S;    // mutex : This is the semaphore to controll sync queue
								// Queue full --> empty : 0             / full : queue_Size
								// Queue empty --> empty : queue_Size / full : 0
								// S : Syncronize thread pool.

void init(Queue *q);
void enqueue(Queue *q, index send);
index dequeue(Queue *q);

void send(int x, int y);        // send data to Queue(enqueue ; data(index)) & Syncronize 
void recv(int *x, int *y);    // recieve data from Queue(dequeue ; data(index)) & Syncronize

void *Gaussian(void *ptr);             // recieve data and work Gaussian Eliminate ( worker thread )
void *BackSubstitution(void *ptr); // recieve data and work Back Substitution ( worker thread )

int main(int argc, char *argv[]) {
	int x;                        // for using index of normal situation
	int l, i, j;                    // for using index of Gaussian & BackSubstitution
	int a_int, b_int;            // read integer of 'a.dat'(a_int) & integer of 'b.dat'(b_int)
	int fd1, fd2, fd3;            // file descripter
	struct timeval start, end; // for using gettimeofday()
	double checkTime;            // For checking time to resolve this simultaneous equations 

	 // ---------------------------------- Sentence

	if (argc != 6) {
		printf("Need 6 tokens!");
		return -1;
	}  // Error Code (not 5 tokens) 

	p = atoi(argv[1]);
	queue_Size = atoi(argv[2]);
	fd1 = open(argv[3], O_RDONLY);
	fd2 = open(argv[4], O_RDONLY);

	// ---------------------------------- Open tokens

	init(&q);
	thread = (pthread_t *)malloc(sizeof(pthread_t)*p);

	sem_init(&mutex, 0, 1);
	sem_init(&full, 0, 0);
	sem_init(&empty, 0, queue_Size);

	// ---------------------------------- init Queue & create threads, semaphores

	read(fd1, &(a_int), sizeof(int));

	A = (float *)malloc(sizeof(float)*a_int*a_int);

	for (x = 0; x<a_int*a_int; x++) {
		lseek(fd1, sizeof(int) + sizeof(float)*x, SEEK_SET);
		read(fd1, &(A[x]), sizeof(float));
	}    // Read 'MATRIX A'

	read(fd2, &(b_int), sizeof(int));

	B = (float *)malloc(sizeof(float)*b_int);

	for (x = 0; x<b_int; x++) {
		lseek(fd2, sizeof(int) + sizeof(float)*x, SEEK_SET);
		read(fd2, &(B[x]), sizeof(float));
	}    // Read 'Vector B'

	C = (float *)malloc(sizeof(float)*b_int);

	n = a_int = b_int; // Vector Size

					   // ----------------------------------- Read Matrix&Vector / Make array A,B,C

	gettimeofday(&start, NULL);  // Checking start time

	for (x = 0; x<p; x++) {
		if (pthread_create(&thread[x], NULL, Gaussian, NULL)<0) {
			printf("Fail pthread_create function!!\n");
			exit(0);
		}
	}

	int d = (n - 2) - 0 + 1;    // d is quantity of the 'Works
	sem_init(&S, 0, d);        // allocate quantity of the 'Works'

	for (l = 0; l <= n - 2; l++) {
		for (i = l + 1; i <= n - 1; i++) {
			send(l, i);
		}
		//sync step
		for (x = 0; x < d; x++) {
			sem_wait(&S);
		}
		d--;
	}
	for (x = 0; x<p; x++) {
		send(SENTINEL, 0);
	} for (x = 0; x<p; x++) {
		pthread_join(thread[x], NULL);
	}

	// ------------------------------------ Gaussian Elimination

	for (x = 0; x<p; x++) {
		if (pthread_create(&thread[x], NULL, BackSubstitution, NULL)<0) {
			printf("Fail pthread_create function!!\n");
			exit(0);
		}
	}

	d = n - 1;
	for (i = n - 1; i >= 0; i--) {
		C[i] = B[i] / A[i*n + i];
		for (j = 0; j <= i - 1; j++) {
			send(i, j);
		}
		//sync step
		for (x = 0; x < d; x++) {
			sem_post(&S);
		}
		d--;
	}
	for (x = 0; x<p; x++) {
		send(SENTINEL, 0);
	} for (x = 0; x<p; x++) {
		pthread_join(thread[x], NULL);
	}

	// ------------------------------------ Back Substitution

	gettimeofday(&end, NULL);    // Checking end time
	checkTime = (double)(end.tv_sec) + (double)(end.tv_usec) / 1000000.0 - (double)(start.tv_sec) - (double)(start.tv_usec) / 1000000.0;
	printf("- Time to resolve this simultaneous equations : %f sec\n", checkTime);
	// Printing result    

	fd3 = open(argv[5], O_WRONLY | O_CREAT | O_TRUNC, 0666);
	write(fd3, &n, sizeof(int));
	for (x = 0; x<b_int; x++) {
		lseek(fd3, sizeof(int) + sizeof(float)*x, SEEK_SET);
		write(fd3, &(C[x]), sizeof(float));
	}
	// Open 'c.dat' & Write Vector 'X'(Array C[n]) to 'c.dat'  

	sem_destroy(&mutex); sem_destroy(&full);
	sem_destroy(&empty); sem_destroy(&S);
	free(A); free(B); free(C);
	free(thread);
	close(fd1); close(fd2); close(fd3);
	return 0;
}

// ---------------------------------------- Write X Vector & main 

void init(Queue *q) {
	q->front = q->rear = 0;
	q->queue = (index *)malloc(sizeof(q->queue)*queue_Size);
}

void enqueue(Queue *q, index send) {
	q->rear = (q->rear + 1) % queue_Size;
	q->queue[q->rear] = send;
}

index dequeue(Queue *q) {
	q->front = (q->front + 1) % queue_Size;
	return q->queue[q->front];
}

// ---------------------------------------- Method for Queue

void send(int x, int y) { // producer : main thread
	sem_wait(&empty);
	sem_wait(&mutex);

	index temp;
	temp.x = x; temp.y = y;
	enqueue(&q, temp);

	sem_post(&mutex);
	sem_post(&full);
}

void recv(int *x, int *y) { // consumer : worker thread
	sem_wait(&full);
	sem_wait(&mutex);

	index temp = dequeue(&q);
	*x = temp.x;
	*y = temp.y;

	sem_post(&mutex);
	sem_post(&empty);
}

// ---------------------------------------- send() & recv() 

void *Gaussian(void *ptr) {
	int l, i, j;
	recv(&l, &i);
	while (l != SENTINEL) {
		float ratio = A[i*n + l] / A[l*n + l];
		for (j = l; j < n; j++) {
			A[i*n + j] = A[i*n + j] - ratio*A[l*n + j];
		}
		B[i] = B[i] - ratio*B[l];
		sem_post(&S);
		recv(&l, &i);
	}
	pthread_exit(0);
	return NULL;
}

// ------------------------------------- Gaunssian Elimination

void *BackSubstitution(void *ptr) {
	int i, j;
	recv(&i, &j);
	while (i != SENTINEL) {
		B[j] = B[j] - C[i] * A[j*n + i];
		A[j*n + i] = 0;
		sem_wait(&S);
		recv(&i, &j);
	}
	pthread_exit(0);
	return NULL;
}

// ------------------------------------- Back Substitution 
