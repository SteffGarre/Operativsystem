#include <stdlib.h>
#include <stdio.h>
#include "dlmall.h"

void testfunct(){
	
	init();
	
	/* Traverse all memory blocks from arena until end */
	printf("\nFIRST TRAVERSE \n");
	traverseblocks();
	printf("\n\n");
	
	struct head *var1 = dalloc(100);
	
	printf("\nSECOND TRAVERSE \n");
	traverseblocks();
	printf("\n\n");
	
	printf("memory allocated at %p \n", &var1);
	dfree(var1);
	printf("memory freed at %p \n", &var1);
	
	/* Check for abnormalities */
	sanity();
	
	/* Traverse all memory blocks from arena until end */
	printf("\nTHIRD TRAVERSE \n");
	traverseblocks();
	printf("\n\n");
	
}

int main(){
	
	testfunct();
}