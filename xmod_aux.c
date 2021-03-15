#include "xmod_aux.h"

double timeElapsed(){

    clock_t midTime = clock();
    double time_spent =((double)(midTime-begin) / CLOCKS_PER_SEC)*1000;
	return time_spent;
}