#include <stdio.h>
#include <stdlib.h>
#include "e-lib.h"
#include "e_mat_pow.h"
#include "e_darts_print.h"


// all three are in DRAM and placed by the linker script
params_t _matPowParams __attribute__ ((section(".matPowParams")));
matrix_space_t _matSpace __attribute__ ((section(".matSpace")));
flag_t _startFlag __attribute__ ((section(".startFlag")));
matrix_space_t _locMatSpace __attribute__ ((section(".locMatSpace")));

// both in local core memory so different copies for each core
flag_t _interBarrier __attribute__ ((section(".interBarrier")));
flag_t _finalBarrier __attribute__ ((section(".finalBarrier")));

unsigned core_ids[16] = {0x808, 0x809, 0x80a, 0x80b, \
	                 0x848, 0x849, 0x84a, 0x84b, \
			 0x888, 0x889, 0x88a, 0x88b, \
			 0x8c8, 0x8c9, 0x8ca, 0x8cb};

int main(void)
{
	//declare barriers in each core's local storage
	unsigned this_core_id;
	GETCOREID(this_core_id);
	
	flag_t *start_flag = (flag_t *) &(_startFlag);
	unsigned *inter_bar = (unsigned *) APPEND_COREID(this_core_id, &(_interBarrier));
	unsigned *final_bar = (unsigned *) APPEND_COREID(this_core_id, &(_finalBarrier));

	while(*start_flag != 1); //wait for start signal

	int pow = _matPowParams.pow;
	int size = _matPowParams.size;
	int row = e_group_config.core_row;
	int col = e_group_config.core_col;

	int *start = (int *) &(_locMatSpace.matrix[0]);
	int *inter = (int *) &(_locMatSpace.matrix[size*size]);
	int *end = (int *) &(_locMatSpace.matrix[size*size*2]);

	if (pow == 0) {
		if (row == 0 && col == 0) {
			int sum = size * 2;
			_matSpace.matrix[0] = sum;
		}
		*final_bar = 1; //set barrier locally
	}
	else if (pow == 1) {
		// copy in parallel based on core ID
		// for this basic test case assume its just a 4x4 matrix
		//_matSpace.matrix[end_i + row*size + col] = _matSpace.matrix[row*size + col];
		copy_mat(start, end, size);
	}
	else if (pow > 1) {
		int pow_block = (int) pow / 16; //split exponent into submultiplications
		int pow_rem = pow % 16;
		if (e_group_config.core_row == 0 && e_group_config.core_col == 0) {
			e_darts_print("pow_block = %d, pow_rem = %d\n", pow_block, pow_rem);
		}
		for (int l=pow_block; l>1; l--) { //top level -- number of multiplies
			// if handles intermediate copies
			if (l == pow_block) {
				// on first run, multiplies in by itself, stores in out
				full_mult_mat(start, start, end, size);
			}
			else {
				// on every other run, multiplies in by inter
				// stores in out
				//mult_mat(start, inter, end, size);
				full_mult_mat(start, inter, end, size);
			}
				// copies out back to inter except for last run
			full_copy_mat(end, inter, size);
		}
		*inter_bar = 1U;
		if (e_group_config.core_row == 0 && e_group_config.core_col == 0) {
			while (check_inter_bar(inter_bar) == 0);
			e_darts_print("interBarrier passed\n");
			/*
			e_darts_print("'end' address: %x\n", end);
			e_darts_print("(0,0) has end:\n");
			for (int i=0; i<size; i++) {
				for (int j=0; j<size; j++) {
					e_darts_print("%d ", end[i*size+j]);
				}
				e_darts_print("\n");
			}
			e_darts_print("(0,0) has start:\n");
			for (int i=0; i<size; i++) {
				for (int j=0; j<size; j++) {
					e_darts_print("%d ", start[i*size+j]);
				}
				e_darts_print("\n");
			}
			e_darts_print("(0,0) has inter:\n"); //at this point inter is all 0s and I don't know why
			for (int i=0; i<size; i++) {
				for (int j=0; j<size; j++) {
					e_darts_print("%d ", inter[i*size+j]);
				}
				e_darts_print("\n");
			}
			*/
			// do remainder multiplications
			for (int i=0; i<pow_rem; i++) {
				full_mult_mat(start, inter, end, size);
				full_copy_mat(end, inter, size);
			}
			/*
			e_darts_print("After remainder multiplication\n");
			e_darts_print("(0,0) has end:\n");
			for (int i=0; i<size; i++) {
				for (int j=0; j<size; j++) {
					e_darts_print("%d ", end[i*size+j]);
				}
				e_darts_print("\n");
			}
			*/
			// perform joining multiplications
			for (int i=1; i<16; i++) {
				//e_darts_print("'end' address: %x\n", end);
				int *ext_out = (int *) APPEND_COREID(core_ids[i], end);
				//e_darts_print("'ext_out' address: %x\n", ext_out);
				// multiply with result from each other core
				full_copy_mat(ext_out, start, size); //copy external to start
				/*
				e_darts_print("ext_out (as start, post-copy) has:\n");
				for (int i=0; i<size; i++) {
					e_darts_print("%d %d %d %d ", start[i*size], start[i*size+1], start[i*size+2], start[i*size+3]);
					e_darts_print("%d %d %d %d ", start[i*size+4], start[i*size+5], start[i*size+6], start[i*size+7]);
					e_darts_print("%d %d %d %d\n", start[i*size+8], start[i*size+9], start[i*size+10], start[i*size+11]);
				}
				*/
				//full_mult_mat_ext(inter, ext_out, end, size, core_ids[i]);
				//full_mult_mat(inter, ext_out, end, size);
				full_mult_mat(inter, start, end, size); //multiply external (start) by inter (prior result)
				/*
				e_darts_print("end after mat mult:\n");
				for (int i=0; i<size; i++) {
					e_darts_print("%d %d %d %d ", end[i*size], end[i*size+1], end[i*size+2], end[i*size+3]);
					e_darts_print("%d %d %d %d ", end[i*size+4], end[i*size+5], end[i*size+6], end[i*size+7]);
					e_darts_print("%d %d %d %d\n", end[i*size+8], end[i*size+9], end[i*size+10], end[i*size+11]);
				}
				*/
				// copy result back to inter
				full_copy_mat(end, inter, size);
			}
		}

	}
	*final_bar = 1; //set final signal
	return 0;
}

unsigned check_inter_bar(unsigned *inter_bar) {
	// bitwise and all the inter flags, if all 1 -> result is 1, if not -> 0
	unsigned result = 1U & (*((unsigned *)APPEND_COREID(core_ids[0], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[1], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[2], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[3], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[4], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[5], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[6], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[7], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[8], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[9], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[10], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[11], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[12], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[13], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[14], inter_bar)));
	result = result & (*((unsigned *)APPEND_COREID(core_ids[15], inter_bar)));
	return(result);
	/*
	unsigned result = 0;
	for (int i=1; i<16; i++) {
		unsigned flag = *((unsigned *)APPEND_COREID(core_ids[i], inter_bar));
		result += flag;
	}
	if (result == 15) {
		return(1);
	}
	else {
		return(0);	
	}
	*/
}

void copy_mat(int *in, int *out, int n) {
	int row = e_group_config.core_row;
	int col = e_group_config.core_col;
	out[row*n + col] = in[row*n + col];
}

void full_copy_mat(int *in, int *out, int n) {
	for (int i=0; i<n*n; i++) {
		out[i] = in[i];
	}
}

void copy_mat_block(int *in, int *out, int n) {
	int block_size = 1;
	int extra = 0;
	if (n*n > 16) {
		block_size = (int) (n*n) / 16;
		extra = (n*n) % 16;
	}
	int e_row = e_group_config.core_row;
	int e_col = e_group_config.core_col;
	int e_id = e_row * 4 + e_col;
	int id = (e_row * 4 + e_col) * block_size;
	for (int l=0; l<block_size; l++) {
		int row = (int) id / n;
		int col = (int) id % n;
		out[row*n + col] = in[row*n + col];
		id++;
	}
	if (e_id < extra) {
		id = 16 * block_size + e_id;
		int row = (int) id / n;
		int col = (int) id % n;
		out[row*n + col] = in[row*n + col];
	}
}

// x * y = out all with size nxn
// for now basically assumes 4x4
void mult_mat_block(int *x, int *y, int *out, int n) {
	int block_size = 1;
	int extra = 0;
	if (n*n > 16) {
		block_size = (int) (n*n) / 16;
		extra = (n*n) % 16;
	}
	int e_row = e_group_config.core_row;
	int e_col = e_group_config.core_col;
	int e_id = e_row * 4 + e_col;
	int id = (e_row * 4 + e_col) * block_size;
	for (int l=0; l<block_size; l++) {
		int acc = 0;
		int row = (int) id / n;
		int col = (int) id % n;
		for (int k=0; k<n; k++) {
			acc += x[row*n + k] * y[k*n + col];
		}
		out[row*n + col] = acc;
		id++;
	}
	if (e_id < extra) {
		id = 16 * block_size + e_id;
		int row = (int) id / n;
		int col = (int) id % n;
		int acc = 0;
		for (int k=0; k<n; k++) {
			acc += x[row*n + k] * y[k*n + col];
		}
		out[row*n + col] = acc;
	}
}

void mult_mat(int *x, int *y, int *out, int n) {
	int row = e_group_config.core_row;
	int col = e_group_config.core_col;
	int acc =0;
	for (int k=0; k<n; k++) {
		acc += x[row*n + k] * y[k*n + col];
	}
	out[row*n + col] = acc;
}

void full_mult_mat(int *x, int *y, int *out, int n) {
	for (int i=0; i<n; i++) {
		for (int j=0; j<n; j++) {
			int acc = 0;
			for (int k=0; k<n; k++) {
				acc += x[i*n+k] * y[k*n+j];
			}
			out[i*n+j] = acc;
		}
	}
}

void full_mult_mat_ext(int *x, int *y, int *out, int n, unsigned coreID) {
	int *ext = (int *) APPEND_COREID(coreID, y);
	for (int i=0; i<n; i++) {
		for (int j=0; j<n; j++) {
			int acc = 0;
			for (int k=0; k<n; k++) {
				acc += x[i*n+k] * ext[k*n+j];
			}
			out[i*n+j] = acc;
		}
	}
}
