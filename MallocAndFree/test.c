#include <stdlib.h>
#include <stdio.h>
#include "dlmall.h"

void testfunct(){
	
	init();
	
	/* Traverse all memory blocks from arena until end */
	printf("\nFIRST TRAVERSE \n");
	traverseblocks();
	printf("\n\n");
	
	struct head *block1 = dalloc(10);
	struct head *block2 = dalloc(64);
	struct head *block3 = dalloc(512);
	struct head *block4 = dalloc(10000);
	struct head *block5 = dalloc(1006);
	
	printf("\nSECOND TRAVERSE \n");
	traverseblocks();
	printf("\n\n");
	
	printf("memory allocated at %p for block1\n", &block1);
	printf("memory allocated at %p for block2\n", &block2);
	printf("memory allocated at %p for block3\n", &block3);
	printf("memory allocated at %p for block4\n", &block4);
	printf("memory allocated at %p for block5\n", &block5);
	printf("\n");
	dfree(block1);
	dfree(block2);
	dfree(block3);
	dfree(block4);
	dfree(block5);
	printf("memory freed at %p for block1\n", &block1);
	printf("memory freed at %p for block2\n", &block2);
	printf("memory freed at %p for block3\n", &block3);
	printf("memory freed at %p for block4\n", &block4);
	printf("memory freed at %p for block5\n", &block5);
	printf("\n");
	
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