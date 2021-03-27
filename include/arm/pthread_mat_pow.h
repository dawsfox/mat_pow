
//takes input square matrix in_mat with size nxn, raises it to power pow, outputs to out_mat
int mat_pow(int *in_mat, int size, int *out_mat, int pow);
// prints a matrix with pretty formatting of size nxn
void print_mat(int *mat, int n);
//copies in matrix into out matrix element by element with size of nxn
void copy_mat(int *in, int *out, int n);
// multiplies x by y and stores in out, all square with size nxn
void mult_mat(int *x, int *y, int *out, int n);
// function for pthread to use for half the multiplications
void *mat_pow_thread(void *arguments);
// struct to hold all arguments pthread will need for its own mat_pow_thread
typedef struct pthread_args_s {
	int *in_mat;
	int size;
	int *out_mat;
	int pow;
} pthread_args_t;
