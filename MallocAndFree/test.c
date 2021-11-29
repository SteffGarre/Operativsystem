#include <stdlib.h>
#include <stdio.h>
#include "dlmall.h"

void testfunct(){
	
	init();
	
	/* Traverse all memory blocks from arena until end */
	printf("\nFIRST TRAVERSE OF ARENA \n");
	traverseblocks();
	printf("\n\n");
	
	struct head *block1 = dalloc(10);
	struct head *block2 = dalloc(64);
	struct head *block3 = dalloc(500);
	struct head *block4 = dalloc(1100);
	struct head *block5 = dalloc(10000);
	
	printf("\nSECOND TRAVERSE OF ARENA \n");
	traverseblocks();
	printf("\n\n");
	
	printf("memory allocated for block1 at %p \n", &block1);
	printf("memory allocated for block2 at %p \n", &block2);
	printf("memory allocated for block3 at %p \n", &block3);
	printf("memory allocated for block4 at %p \n", &block4);
	printf("memory allocated for block5 at %p \n", &block5);
	printf("\n");
	dfree(block1); 
	dfree(block2);
	dfree(block3);
	dfree(block4);
	dfree(block5);
	printf("memory freed for block1 at %p \n", &block1);
	printf("memory freed for block2 at %p \n", &block2);
	printf("memory freed for block3 at %p \n", &block3);
	printf("memory freed for block4 at %p \n", &block4);
	printf("memory freed for block5 at %p \n", &block5);
	printf("\n");
	
	/* Check for abnormalities */
	sanity();
	
	/* Traverse all memory blocks from arena until end */
	printf("\nTHIRD TRAVERSE OF ARENA \n");
	traverseblocks();
	printf("\n\n");
	
}

int main(){
	
	testfunct();
}