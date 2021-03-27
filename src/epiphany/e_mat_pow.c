#include <stdio.h>
#include <stdlib.h>
#include "e-lib.h"
#include "e_mat_pow.h"

#define SUM_RESULT 0x2228
#define INTER_BARRIER 0x2008 //barrier for each multiplication
#define FINAL_BARRIER 0x200c //barrier for end of computation
#define STATES_BASE_ADDR 0x3000  /// Chosen randomly
#define START_SIGNAL STATES_BASE_ADDR // consumer->producer(ready to receieve)
#define DONE_SIGNAL STATES_BASE_ADDR // producer->consumer (done pushing codelets)
#define CODQUEUE_ADDR (STATES_BASE_ADDR + 0x4) // Metadata and handler

typedef void (*codeletFunction)();

params_t _matPowParams __attribute__ ((section(".matPowParams")));
matrix_space_t _matSpace __attribute__ ((section(".matSpace")));


int main(void)
{
	//declare barriers in each core's local storage
	unsigned this_core_id;
	GETCOREID(this_core_id);
	
	unsigned *inter_bar = (unsigned *) APPEND_COREID(this_core_id, INTER_BARRIER);
	unsigned *final_bar = (unsigned *) APPEND_COREID(this_core_id, FINAL_BARRIER);

	int pow = _matPowParams.pow;
	int size = _matPowParams.size;
	int row = e_group_config.core_row;
	int col = e_group_config.core_col;

	//unsigned start_i = 0; //index in matSpace where start begins
	//unsigned inter_i = size*size; //index where inter begins
	//unsigned end_i = size*size*2; //index where end begins

	int *start = (int *) &(_matSpace.matrix[0]);
	int *inter = (int *) &(_matSpace.matrix[size*size]);
	int *end = (int *) &(_matSpace.matrix[size*size*2]);

	if (pow == 0) {
		if (row == 0 && col == 0) {
			int sum = size * 2;
			_matSpace.matrix[0] = sum;
		}
		*final = 1; //set barrier locally
	}
	else if (pow == 1) {
		// copy in parallel based on core ID
		// for this basic test case assume its just a 4x4 matrix
		//_matSpace.matrix[end_i + row*size + col] = _matSpace.matrix[row*size + col];
		copy_mat(start, end, size);
	}
	else if (pow > 1) {
		for (int l=pow; l>1; l--) { //top level -- number of multiplies
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
	}

	return 0;
}

void copy_mat(int *in, int *out, int n) {
	int row = e_group_config.core_row;
	int col = e_group_config.core_col;
	out[row*n + col] = in[row*n + col];
	/*
	for (int i=0; i<n; i++) {
		for (int j=0; j<n; j++) {
			out[i*n + j] = in[i*n + j];
		}
	}
	*/
}

// x * y = out all with size nxn
// for now basically assumes 4x4
void mult_mat(int *x, int *y, int *out, int n) {
	int row = e_group_config.core_row;
	int col = e_group_config.core_col;
	int acc =0;
	for (int k=0; k<n; k++) {
		acc += x[row*n + k] * y[k*n + col];
	}
	out[row*n + col] = acc;
}
