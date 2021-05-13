#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "e-hal.h"
#include "e-loader.h"
#include "darts_print_server.h"

#define INTER_BARRIER 0x6c //barrier are stored locally in each core
#define FINAL_BARRIER 0x70
#define START_FLAG 0x007f8 //flag is in DRAM, offset from symbol table of elf

typedef struct params_s {
	int size;
	int pow;
} params_t;

typedef unsigned flag_t;


//first arg: file name to input matrix
//second: n size (nxn of input matrix)
//third: pow to raise matrix to
int main(int argc, char *argv[]){
	e_platform_t platform;
	e_epiphany_t dev;
	e_mem_t e_mat_begin;
	e_mem_t e_start_flag;

	clock_t start_time, end_time;
	double cpu_time;

	//Initalize Epiphany device
	e_init(NULL);
	e_reset_system();//reset Epiphany
	e_get_platform_info(&platform);
	start_printing_server();
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
	
	start_time = clock();

	e_alloc(&e_mat_begin, 0x130, total_data_size);
	e_alloc(&e_start_flag, START_FLAG, sizeof(flag_t));
	e_load_group("e_mat_split_pow.elf", &dev, 0, 0, 4, 4, E_FALSE);
	printf("Group loaded \n");
	e_start_group(&dev);
	usleep(1e5);

	e_write(&e_mat_begin, 0, 0, 0x0, &mat_pow_params, sizeof(mat_pow_params));
	//e_write(&e_mat_begin, 0, 0, 0x8, start, sizeof(int) * size * size);
	for (int i=0; i<16; i++) {
		//offset taken from symbol table of elf file / linker script
		e_write(&dev, i/4, i%4, 0x78, start, sizeof(int)*size*size);
	}
	flag_t start_flag = 1;
	e_write(&e_start_flag, 0, 0, 0x0, &start_flag, sizeof(flag_t)); //signal start of computation

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
    
	printf("Waiting for final barrier...\n");
	int number;
	for(int i=0; i<16; i++) { //wait for all cores to have darts_rt_alive = 0
		number = 0;
		while(number != 1) {
	        	e_read(&dev, i/4, i%4, FINAL_BARRIER, &number, sizeof(number));
		}
	}
	int *result = (int *) malloc(size * size * sizeof(int));
	e_read(&dev, 0, 0, 0x78 + size*size*sizeof(int)*2, result, size*size*sizeof(int)); //offset params, start, and inter

	end_time = clock();

	printf("[");
	for (int i=0; i<size; i++) {
		for (int j=0; j<size; j++) {
			printf("\t%d", result[i*size+j]);
		}
		printf("\n");
	}
	printf("]\n");

	cpu_time = ((double) (end_time-start_time)) / CLOCKS_PER_SEC;
	printf("mat_pow took %lf seconds\n", cpu_time);

	free(start);
	free(result);

	usleep(1e5);
	stop_printing_server();
	e_close(&dev);
	e_finalize();
	return EXIT_SUCCESS;
}

