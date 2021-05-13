#include "e_darts_print.h"

darts_mutex_t __printingMutex __attribute__ ((section("printingMutex"))) = DARTS_MUTEX_NULL; // Located in a particular location in core 0,0 in linker script
printBuffer_t __printBuffer __attribute__ ((section(".printBuffer"))) = {0, &__printingMutex, ""};

void e_darts_print(const char * message, ...){

    darts_mutex_lock(__printBuffer.mutex);
    // if sendPrintInstruction is not 0, it means the host
    // has not finished printing
    while (__printBuffer.sendPrintInstruction != 0);
    char argTypes[MAX_NUM_ARGUMENTS]; // up to 20 arguments more
    va_list args_list;
    int charCount = 0;
    int argCount = 0;

    // We copy the message to the buffer up to 200 characters
    while (message[charCount] != 0x0 && charCount < MAX_NUM_CHARACTERS)
    {
    	// Counting the number of arguments we should have
    	if (message[charCount] == '%' && argCount < MAX_NUM_ARGUMENTS)
    	{
    		argTypes[argCount] = message[charCount + 1];
    		argCount++;
    	}
    	__printBuffer.printingBufferHead[charCount] = message[charCount];
    	charCount++;
    }
    // If we reach 200 characters, we want to truncate the message
    if(charCount == MAX_NUM_CHARACTERS) {
    	__printBuffer.printingBufferHead[charCount - 1] = 0x0;
    } else {
    	__printBuffer.printingBufferHead[charCount] = 0x0;
    }

    // Now, let's add all the arguments to the arguments buffer
    va_start(args_list, message);
    charCount = 0;
    for (; argCount >= 0 && charCount < ARGUMENTS_SIZE; argCount--) {
    	// TODO Fix %s. It seems like the characters are not sent in the argument args_list
    	if (argTypes[argCount] == 's' || argTypes[argCount] == 'S') {
    		//__printBuffer.arguments[charCount++] = 0;
    		//while (argStart[charCount] != 0x0 && charCount < ARGUMENTS_SIZE) {
    		//    __printBuffer.arguments[charCount] = argStart[charCount];
    		//    charCount++;
    		//}
    	}
    	else {
    		//int size = TYPE_SELECTOR(argTypes[argCount]);
		arg_receive_t arg;
		char current = argTypes[argCount];
		int size;
		if (current == 'f' || current == 'e' || current == 'E' || current == 'g' || current == 'G' || 
		    current == 'F' || current == 'a' || current == 'A') {
			size = 8;
			arg.fArg = va_arg(args_list, double);
			char *charConv = (char *) &(arg.fArg);
			while (size > 0) {
				__printBuffer.arguments[charCount] = charConv[8-size];
				charCount++;
				size --;
			}
		}
		else if (current == 'd' || current == 'i') {
			size = 4;
			arg.iArg = va_arg(args_list, int);
			while (size > 0) {
				__printBuffer.arguments[charCount] = arg.iArg & 255;
				arg.iArg = arg.iArg >> 8;
				charCount++;
				size --;
			}
		}
		else if (current == 'x' || current == 'X' || current == 'o' || current == 'u') {
			size = 4;
			arg.uArg = va_arg(args_list, unsigned);
			while (size > 0) {
				__printBuffer.arguments[charCount] = arg.uArg & 255;
				arg.uArg = arg.uArg >> 8;
				charCount++;
				size --;
			}
		}
		//what about char? issues with char being promoted to int through ...
    	}
    }
    va_end(args_list);

    int i = 0;
    // Wait for the whole buffer to reach memory...
    __printBuffer.sendPrintInstruction = 1;
    darts_mutex_unlock(__printBuffer.mutex);

}
