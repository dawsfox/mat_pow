/**
 * @file e_darts_print_server.h
 * @author Jose M Monsalve
 * @date 8 July 2017
 * @brief Listener printer server for the host to print from the device
 * @todo Add copyright
 * @todo add printing of variables
 *
 * This will spawn a PThread that will be looping over the printing buffer that
 * is created on the device. After filling the print buffer, the device will
 * set a flag to 1 when it wants to print something. The host will restart the
 * flag to 0 after printing the buffer
 */

#include "e-hal.h"
#include <stdio.h>
#include <string.h>

// Changing this should also be changed on the device side
#define MAX_NUM_CHARACTERS 200
#define MAX_NUM_ARGUMENTS 20
#define ARGUMENTS_SIZE 96

#define TRANSFORM_PARAMETER(letter,tempBuffer,argument)            \
         {                                                              \
            char myChar[3];                                             \
            myChar[0]='%'; myChar[1]=letter; myChar[3]=0x0;             \
            if (letter == 'd' || letter == 'i') {                       \
                sprintf(tempBuffer, myChar,*((int*)argument));       \
                argument += sizeof(int);                               \
            } else if (letter == 'o' || letter == 'u' ||                \
                letter == 'x' || letter == 'X') {                       \
                sprintf(tempBuffer, myChar,*((unsigned*)argument));   \
                argument += sizeof(int);                              \
            } else if (letter == 'e' || letter == 'E' ||                \
                letter == 'f' || letter == 'g' ||                       \
                letter == 'G' || letter == 'a' ||                       \
                letter == 'A') {                                        \
                sprintf(tempBuffer, myChar, *((double*)argument));      \
                argument += sizeof (double);                            \
            } else if (letter == 'c' || letter == 'C') {                \
                sprintf(tempBuffer, myChar,*((char *)argument));        \
                argument += sizeof (char);                              \
            } else if (letter == 's' || letter == 'S') {                \
                /*strcpy(tempBuffer, argument);*/                       \
                tempBuffer[0] = 0; printf("\nSTRINGS NOT SUPPORTED YET\n");\
                argument += strlen(tempBuffer);                         \
            } else {                                                    \
                tempBuffer[0]= 0x0;                                     \
            }                                                           \
         }                                                              \



typedef struct __attribute__ ((__packed__)) printBuffer_s
{
    unsigned sendPrintInstruction;
    unsigned padding;
    char printingBufferHead[MAX_NUM_CHARACTERS];
    char arguments[ARGUMENTS_SIZE];
} printBuffer_t;

/**
 * @brief wrapper for the pthred initialization
 *
 * starts the printing server and comes back
 *
 * @returns Error by pthreads library
 */
int start_printing_server();


/**
 * @brief The actual printing function
 *
 * This function is async.
 *
 */

void* printing_server(void *tid);

/**
 * @brief This functions signals the server for safe termination
 */
void stop_printing_server();


