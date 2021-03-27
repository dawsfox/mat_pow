#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
//#include "e-hal.h"
//#include "e-loader.h"

#define N 2
#define POW 3

//takes input square matrix in_mat with size nxn, raises it to power pow, outputs to out_mat
int mat_pow(int *in_mat, int size, int *out_mat, int pow);
// prints a matrix with pretty formatting of size nxn
void print_mat(int *mat, int n);
//copies in matrix into out matrix element by element with size of nxn
void copy_mat(int *in, int *out, int n);
// multiplies x by y and stores in out, all square with size nxn
void mult_mat(int *x, int *y, int *out, int n);

//first arg: file name to input matrix
//second: n size (nxn of input matrix)
//third: pow to raise matrix to
int main(int argc, char *argv[]){
	int start[N*N];
	int end[N*N];
	// for now
	start[0] = 2;	// start: [2 -1
	start[1] = -1;  //	   0  3]
	start[2] = 0;
	start[3] = 3;

	printf("power: %d, input matrix:\n", POW);
	print_mat(start, N);
        printf("starting exponentiation\n");
	int result = mat_pow(start, N, end, POW);
	printf("exponentiation complete\n");
	if (result == 0) {
		print_mat(end, N);
	}
	else if (result == -1) {
		printf("***** no calculation *****\n");
	}
	else if (result == -2) {
		printf("***** pow=1, out=in *****\n");
		print_mat(end, N);
	}

	return EXIT_SUCCESS;
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
			printf("executing multiplication %d\n", pow - l + 1);
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
