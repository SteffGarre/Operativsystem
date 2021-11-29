#include <math.h>
#include <stdlib.h>

#define MIN 8

/* generates random sequence of size for bench to use */
int request(int max){

	/* k is log(MAX/MIN) */
	double k = log(((double) max) / MIN);
	
	/* r is [0 .. k] */
	double r = ((double)( rand() % (int)(k*10000))) / 10000;
	
	/* size is [0 .. MAX] */
	int size = (int)(((double) max) / exp(r));
	
	return size;
}