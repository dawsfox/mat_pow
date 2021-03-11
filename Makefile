sequential: mat_pow_sequential.c
	gcc -o sequential mat_pow_sequential.c

clean:
	rm -f sequential
