#include "e_darts_mutex.h"

int darts_mutex_lock( volatile darts_mutex_t *mutex )
{
    // If the mutex does not contain the core ID in the address
    if (!((unsigned)mutex & 0x000FFFFF))
        return -1;

    const register unsigned offset = 0x0;

    // Obtaining core ID from special register COREID
    register unsigned coreid, val, i, wait = 1;
    //register unsigned addition = ((unsigned)mutex & 0x00300000) >> 20;
    __asm__ __volatile__ ("movfs %0, coreid" : "=r" (coreid));

    // Loop until lock is locked
    do {
        val = coreid;
        __asm__ __volatile__(
                "testset    %[val], [%[mutex], %[offset]]"
                : [val] "+r" (val)
                : [mutex] "r" (mutex), [offset] "r" (offset)
                : "memory");
        // Exponential backoff
        //for (i = 0; val != 0 && i < wait; i++);
        //wait += addition;
    } while (val != 0);

    return val;
}

int darts_mutex_unlock( volatile darts_mutex_t *mutex )
{
    // If the mutex does not contain the core ID in the address
    if (!((unsigned)mutex & 0x000FFFFF))
        return -1;

    const register unsigned zero = 0;

    __asm__ __volatile__(
            "str    %[zero], [%[mutex]]"
            : /* no outputs */
            : [zero] "r" (zero), [mutex] "r" (mutex)
            : "memory");

    return 0;
}
