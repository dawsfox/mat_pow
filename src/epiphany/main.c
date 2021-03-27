#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "e-hal.h"
#include "e-loader.h"

#define INTER_BARRIER 0x2008
#define FINAL_BARRIER 0x200c
#define DARTS_RT_ALIVE 0x6c

typedef struct params_s {
	int size;
	int pow;
} params_t;


//first arg: file name to input matrix
//second: n size (nxn of input matrix)
//third: pow to raise matrix to
int main(int argc, char *argv[]){
	e_platform_t platform;
	e_epiphany_t dev;
	e_mem_t e_mat_begin;

	//Initalize Epiphany device
	e_init(NULL);
	e_reset_system();//reset Epiphany
	e_get_platform_info(&platform);
	e_open(&dev, 0, 0, 4, 4);


	if (argc != 4) {
		printf("incorrect number of args given, exiting\n");
		return 1;
	}
	FILE *file_ptr;
	char in_buf[128];
	int size = atoi(argv[2]);
	int pow = atoi(argv[3]);
	printf("opening file %s\n", argv[1]);
	file_ptr = fopen(argv[1], "r");
	if (file_ptr == NULL) {
                printf("error opening file %s\n", argv[1]);
                return 2;
        }
        int *start = malloc(sizeof(int) * size * size);
        //int *end = malloc(sizeof(int) * size * size);
        for (int i=0; i<size; i++) {
                for (int j=0; j<size; j++) {
                        if (fscanf(file_ptr, "%d", &(start[i*size + j])) != 1) {
                                return 3;
                        }
                }
        }
        fclose(file_ptr);

	// 4 + 4 for size and pow
	// size * size * 4 for one matrix of ints
	// * 3 for in, inter, and out
	unsigned total_data_size = 4 + 4 + size * size * 4 * 3;
	params_t mat_pow_params;
	mat_pow_params.size = size;
	mat_pow_params.pow = pow;
	
	e_alloc(&e_mat_begin, 0x0, total_data_size);
	e_load_group("e_darts_rt_init.elf", &dev, 0, 0, 4, 4, E_FALSE);
	printf("Group loaded \n");
	e_start_group(&dev);
	usleep(1e5);

	e_write(&e_mat_begin, 0, 0, 0x0, &mat_pow_params, sizeof(mat_pow_params));

	//read file into dynamically allocated array
	//e_write size n to first location
	//e_write pow to second location
	//e_write whole matrix
	//mem organization will be
	/*
	 * size=4 bytes
	 * pow=4 bytes
	 * in=size x size bytes
	 * inter=size x size bytes
	 * out=size x size bytes
	 */
    

	for(int i=0; i<16; i++) { //wait for all cores to have darts_rt_alive = 0
		number = 0;
		while(number != 1) {
	        	e_read(&dev, i/4, i%4, FINAL_BARRIER, &number, sizeof(number));
		}
	}
	int sum = 0;
	e_read(&e_mat_begin, 0, 0, 0x8, &sum, sizeof(int));
	printf("sum received: %d\n", sum);
	usleep(1e5);

	e_close(&dev);
	e_finalize();
	return EXIT_SUCCESS;
}

