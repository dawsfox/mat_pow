
#define SUM_RESULT 0x2228
#define FINAL_BARRIER 0x2008

#define MAX_SIZE 10 //max nxn size for in matrix

#define GETCOREID(coreIdVar) __asm__ __volatile__ ("movfs %0, coreid" : "=r" (coreIdVar))
#define APPEND_COREID(coreID, address) ((((unsigned)coreID) << 20) + (((unsigned)address) & 0x000FFFFF))


typedef struct params_s {
	int size;
	int pow;
} params_t;

typedef struct matrix_space_s {
	int matrix[MAX_SIZE*MAX_SIZE*3];
} matrix_space_t;
