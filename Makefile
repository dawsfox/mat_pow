sequential: sequential_mat_pow.c
	gcc -o sequential sequential_mat_pow.c

file_sequential: file_sequential_mat_pow.c
	gcc -o file_sequential file_sequential_mat_pow.c

pthread: pthread_mat_pow.c
	gcc -o pthread_mat_pow pthread_mat_pow.c -lpthread

clean:
	rm -f sequential file_sequential pthread_mat_pow
