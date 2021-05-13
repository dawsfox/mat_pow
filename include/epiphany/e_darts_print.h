/**
 * @file e_darts_print.h
 * @author Jose M Monsalve
 * @date 8 July 2017
 * @brief printing service from the epiphany to darts
 * @todo Add copyright
 *
 * Given that it is not possible to print from the device, it is necessary to
 * have a method that allows to pass information from the device to the host
 * so that the host can print it.
 */

#ifndef E_DARTS_PRINT_H
#define E_DARTS_PRINT_H

//#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "e_darts_mutex.h"

// Changing this should also be changed on the host side
#define MAX_NUM_CHARACTERS 200
#define MAX_NUM_ARGUMENTS 20
#define ARGUMENTS_SIZE 96
#define TYPE_SELECTOR(letter)                               \
        ((letter == 'd' || letter == 'i' ||                  \
         letter == 'o' || letter == 'u' ||                  \
         letter == 'x' || letter == 'X')? sizeof(unsigned):(      \
        (letter == 'c' || letter == 'C')? sizeof(char):     \
        (letter == 'e' || letter == 'E' ||                  \
         letter == 'f' || letter == 'F' ||                  \
         letter == 'g' || letter == 'G' ||                  \
         letter == 'a' || letter == 'A' ||                  \
         letter == 'n' || letter == 'N')? sizeof(double):0))   \


typedef struct __attribute__ ((__packed__)) printBuffer_s
{
    unsigned sendPrintInstruction;
    darts_mutex_t *mutex;
    char printingBufferHead[MAX_NUM_CHARACTERS];
    char arguments[ARGUMENTS_SIZE];
} printBuffer_t;

typedef union {
    double fArg;
    int iArg;
    unsigned uArg;
    char cArg;
} arg_receive_t;

/**
 * @brief printing function
 * @todo allow printing variables
 *
 * Send a message to this function and the message will be send to
 * the host by copying it to the printBuffer. There will be a thread
 * on the host that will be spinning over the sendPrintInstruction flag
 * and whenver there is a 1, it will copy the information over
 *
 */
void e_darts_print(const char * message, ...);

#endif /* E_DARTS_PRINT_H */
