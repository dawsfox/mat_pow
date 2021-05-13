#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "pthread_mat_pow.h"

//first arg: file name to input matrix
//second: n size (nxn of input matrix)
//third: pow to raise matrix to
int main(int argc, char *argv[]){

	clock_t start_time, end_time;
	double cpu_time;

	if (argc != 4) {
		printf("incorrect number of args given, exiting\n");
		return 1;
	}
	/* reading in matrix from input file and other args from command line */
	FILE *file_ptr;
	int size = atoi(argv[2]);
	int pow = atoi(argv[3]);
	printf("opening file %s\n", argv[1]);
	file_ptr = fopen(argv[1], "r");
	if (file_ptr == NULL) {
		printf("error opening file %s\n", argv[1]);
		return 2;
	}
	int *start = malloc(sizeof(int) * size * size);
	int *end = malloc(sizeof(int) * size * size);
	for (int i=0; i<size; i++) {
		for (int j=0; j<size; j++) {
			if (fscanf(file_ptr, "%d", &(start[i*size + j])) != 1) {
				return 3;
			}
		}
	}
	fclose(file_ptr); //close file after done

	pthread_t id;
	/* Because the ARM processor only has two cores in hardware,
	 * for now I am assuming the best parallel implementation possible
	 * will be to split the power operation into two sets of multiplication
	 * then join them with a final multiplication.
	 * Each level of multiplication will be synchronized with a barrier.
	 * If the size of matrix nxn has odd n, the original thread will take
	 * the extra multiplication.
	 */

	// print input info
	printf("power: %d, input matrix:\n", pow);
	print_mat(start, size);

	start_time = clock();

	if (pow >= 4) { //if 3 or more multiplications to be performed
		pthread_args_t thread_args;
		thread_args.in_mat = (int *) malloc(sizeof(int) * size * size);
		thread_args.out_mat = (int *) malloc(sizeof(int) * size * size);
		thread_args.size = size;
		thread_args.pow = (int) pow / 2; //int division, pthread does one less if odd mults
		//copy a second in_mat, since in_mat isn't preserved
		for (int i=0; i<size; i++) {
			for (int j=0; j<size; j++) {
				thread_args.in_mat[i*size + j] = start[i*size + j];
			}
		}
		// start thread doing half or half - 1 of the multiplications
		printf("creating new thread\n");
		if (pthread_create(&id, NULL, &mat_pow_thread, (void *) &thread_args) < 0) {
			printf("error creating thread\n");
			return 4;
		}
		printf("starting exponentiation on main\n");
		mat_pow(start, size, end, pow - pow / 2);
		int *final = malloc(sizeof(int) * size * size);
		printf("waiting for thread to finish\n");
		pthread_join(id, NULL); //wait for thread to finish
		printf("thread finished, multiplying results\n");
		mult_mat(end, thread_args.out_mat, final, size);

		end_time = clock();

		print_mat(final, size);

	}
	else {
        	printf("starting exponentiation\n");
		int result = mat_pow(start, size, end, pow);

		end_time = clock();

		printf("exponentiation complete\n");
		if (result == 0) {
			print_mat(end, size);
		}
		else if (result == -1) {
			printf("***** no calculation *****\n");
		}
		else if (result == -2) {
			printf("***** pow=1, out=in *****\n");
			print_mat(end, size);
		}
	}

	cpu_time = ((double) (end_time-start_time)) / CLOCKS_PER_SEC;
	printf("mat_pow took %lf seconds\n", cpu_time);

	free(start);
	free(end);

	return EXIT_SUCCESS;
}



void *mat_pow_thread(void *arguments) {
	pthread_args_t *args = (pthread_args_t *) arguments;
	int size = args->size;
	int *in_mat = args->in_mat;
	int *out_mat = args->out_mat;
	int pow = args->pow;
	// perform calculation
	int *inter = malloc(sizeof(int) * size * size);
	for (int l=pow; l>1; l--) { //top level -- number of multiplies
		//printf("executing multiplication (thread) %d\n", pow - l + 1);
		// if handles intermediate copies
		if (l == pow) {
			// on first run, multiplies in by itself, stores in out
			mult_mat(in_mat, in_mat, out_mat, size);
		}
		else {
			// on every other run, multiplies in by inter
			// stores in out
			mult_mat(in_mat, inter, out_mat, size);
		}
		if (l > 2) {
			// copies out back to inter except for last run
			copy_mat(out_mat, inter, size);
		}
	}
	free(inter);
	return 0;
}

int mat_pow(int *in_mat, int size, int *out_mat, int pow) {
	if (pow == 0) {
		return -1;
	}
	else if (pow == 1) {
		for (int i=0; i<size; i++) {
			for (int j=0; j<size; j++) {
				// question: how to organize memory?
				// interim matrix for computation?
				// new one for each multiplication?
				// alternatives?
				out_mat[i*size + j] = in_mat[i*size + j];
			}
		}
		return -2;
	}
	else if (pow > 1) {
		// perform calculation
		int *inter = malloc(sizeof(int) * size * size);
		for (int l=pow; l>1; l--) { //top level -- number of multiplies
			//printf("executing multiplication (main) %d\n", pow - l + 1);
			// if handles intermediate copies
			if (l == pow) {
				// on first run, multiplies in by itself, stores in out
				mult_mat(in_mat, in_mat, out_mat, size);
			}
			else {
				// on every other run, multiplies in by inter
				// stores in out
				mult_mat(in_mat, inter, out_mat, size);
			}
			if (l > 2) {
				// copies out back to inter except for last run
				copy_mat(out_mat, inter, size);
			}
		}
		free(inter);
		return 0;
	}

}

void copy_mat(int *in, int *out, int n) {
	for (int i=0; i<n; i++) {
		for (int j=0; j<n; j++) {
			out[i*n + j] = in[i*n + j];
		}
	}
}

// x * y = out all with size nxn
void mult_mat(int *x, int *y, int *out, int n) {
	for (int i=0; i<n; i++) {
		for (int j=0; j<n; j++) {
			int acc = 0;
			for (int k=0; k<n; k++) {
				acc += x[i*n + k] * y[k*n + j];
			}
			out[i*n + j] = acc;
		}
	}
}

//takes pointer to integer matrix of size nxn and the size n and prints nicely
void print_mat(int *mat, int n) {
	printf("[");
	for (int i=0; i<n; i++) {
		
		for (int j=0; j<n; j++) {
			if (j != 0) {
				printf("\t");
			}
			printf("%d", mat[i*n + j]);
		}
		if (i == n-1) {
			printf("]\n");
		}
		else {
			printf("\n");
		}
	}
}
