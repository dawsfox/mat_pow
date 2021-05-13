#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "darts_print_server.h"

struct {
    e_mem_t printBuffer;
    pthread_t printThread;
    int stillRunning;

} pthread_vars;

int start_printing_server()
{
    // Getting memory from host (relative to 0x8e000000)
    e_alloc(&(pthread_vars.printBuffer), 0x00000000, sizeof(printBuffer_t));
    printBuffer_t myBuffer;
    myBuffer.printingBufferHead[0]=0;
    myBuffer.arguments[0]=0;
    myBuffer.sendPrintInstruction = 0;
    e_write(&(pthread_vars.printBuffer), 0, 0, 0x0, &myBuffer, sizeof(myBuffer));
    pthread_vars.stillRunning = 1;
    int rc = pthread_create(&(pthread_vars.printThread), NULL, printing_server, 0);
    if (rc) {
        printf("Error starting printer server pthread... \n");
        printf("-- Return code = %d",rc);
    }
    return rc;
}


void* printing_server(void *tid)
{
    printBuffer_t myBuffer;
    printf("Starting the printer server ...\n");
    printf("printBuffer server side goes from %x to %x, size = %d\n", &myBuffer, myBuffer.arguments, ((unsigned)(myBuffer.arguments) - (unsigned)(&myBuffer)));
    printf("debug print\n");
    while (pthread_vars.stillRunning == 1) {
        // Do server stuff
        e_read(&(pthread_vars.printBuffer), 0, 0, 0x0, &myBuffer, sizeof(printBuffer_t));
        if (myBuffer.sendPrintInstruction == 1) {
            // Wait for a little, read again. to avoid problems
            // with the processor writting to external memory
            int charCount = 0;
            char * argPointer = myBuffer.arguments;
            char tempBuffer[80];
            char processedBuffer[MAX_NUM_CHARACTERS*2];
            char * currentStr = processedBuffer;
	    char floatError = 0x0; // used to protect against float first printing errors
            while (myBuffer.printingBufferHead[charCount] != 0x0 && charCount < MAX_NUM_CHARACTERS) {
                if (myBuffer.printingBufferHead[charCount] == '%') {
		    /*
                    int it = 0;
                    while (it < 64) {
                        printf("%x%x%x%x ", argPointer[it], argPointer[it+1], argPointer[it+2], argPointer[it+3]);
            	        it+=4;
	            }
                    printf("DONE ARG DUMP\n");
		    */
                    TRANSFORM_PARAMETER(myBuffer.printingBufferHead[charCount + 1],
                                        tempBuffer,
                                        argPointer);
                    int counter = 0;
                    //printf("temp buffer contents: ");
                    while (tempBuffer[counter] != 0x0) {
		        //printf("%c", tempBuffer[counter]);
                        *currentStr = tempBuffer[counter];
                        counter++;
                        currentStr++;
                    }
		    //printf("\n");
                    charCount++; // to skip the token %
                } else {
                    *currentStr = myBuffer.printingBufferHead[charCount];
                    currentStr++;
                }
                charCount++;

            }
            // Add end of string
            *currentStr = 0x0;
            printf("--> %s",processedBuffer);
            fflush(stdout);
            myBuffer.printingBufferHead[0]=0;
            myBuffer.arguments[0]=0;
            myBuffer.sendPrintInstruction = 0;
            e_write(&(pthread_vars.printBuffer), 0, 0, 0x0, &myBuffer, sizeof(myBuffer));
        }
    }
    pthread_exit(NULL);
}

void stop_printing_server()
{
    pthread_vars.stillRunning = 0;
}
