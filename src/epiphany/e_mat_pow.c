#include <stdio.h>
#include <stdlib.h>
#include "e-lib.h"
#include "e_mat_pow.h"
#include "e_darts_print.h"


// all three are in DRAM and placed by the linker script
params_t _matPowParams __attribute__ ((section(".matPowParams")));
matrix_space_t _matSpace __attribute__ ((section(".matSpace")));
flag_t _startFlag __attribute__ ((section(".startFlag")));

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
	*inter_bar = 0;

	while(*start_flag != 1); //wait for start signal

	//e_darts_print("Gathering params...\n");

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

	//e_darts_print("Running computations\n");
	if (e_group_config.core_row == 2 && e_group_config.core_col == 1) {
		*start_flag = 0;
	}

	if (pow == 0) {
		// a debug case
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
		for (int l=pow; l>1; l--) { //top level -- number of multiplies
			// if handles intermediate copies
			if (l == pow) {
				// on first run, multiplies in by itself, stores in out
				mult_mat_block(start, start, end, size);
				if (e_group_config.core_row == 2 && e_group_config.core_col == 1) {
					*start_flag = 0;
				}
			}
			else {
				// on every other run, multiplies in by inter
				// stores in out
				//mult_mat(start, inter, end, size);
				if (e_group_config.core_row == 2 && e_group_config.core_col == 1) {
					*start_flag = 0;
				}
				mult_mat_block(start, inter, end, size);
			}
			if (l > 2) {
				// copies out back to inter except for last run
				copy_mat_block(end, inter, size);
				//copy_mat(end, inter, size);
				*inter_bar = 1;
				if (e_group_config.core_row == 2 && e_group_config.core_col == 1) {
					//e_darts_print("(2,1) waiting for interBarrier\n");
					while(check_inter_bar(&(_interBarrier)) != 1U);
					//e_darts_print("interBarrier met\n");
					*start_flag = 1;
					*inter_bar = 0;
				}
				else {
					while(*start_flag != 1);
					*inter_bar = 0;
				}
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
}

void copy_mat(int *in, int *out, int n) {
	int row = e_group_config.core_row;
	int col = e_group_config.core_col;
	out[row*n + col] = in[row*n + col];
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
