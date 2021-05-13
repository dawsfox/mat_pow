
#define START_FLAG
#define INTER_BARRIER 0x2008 //barrier for intermediate multiplies
#define FINAL_BARRIER 0x200c //barrier for end of computation

#define MAX_SIZE 12 //max nxn size for in matrix

#define GETCOREID(coreIdVar) __asm__ __volatile__ ("movfs %0, coreid" : "=r" (coreIdVar))
#define APPEND_COREID(coreID, address) ((((unsigned)coreID) << 20) + (((unsigned)address) & 0x000FFFFF))


typedef struct params_s {
	int size;
	int pow;
} params_t;

typedef struct matrix_space_s {
	int matrix[MAX_SIZE*MAX_SIZE*3];
} matrix_space_t;

typedef unsigned flag_t;

unsigned check_inter_bar(unsigned *inter_bar);
void full_copy_mat(int *in, int *out, int n);
void copy_mat(int *in, int *out, int n);
void copy_mat_block(int *in, int *out, int n);
void full_mult_mat(int *x, int *y, int *out, int n);
void full_mult_mat_ext(int *x, int *y, int *out, int n, unsigned coreID);
void mult_mat(int *x, int *y, int *out, int n);
void mult_mat_block(int *x, int *y, int *out, int n);
