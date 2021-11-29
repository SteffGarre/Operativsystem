#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "random.h"
#include "dlmall.h"
#define BUFFER 100 
#define MAX_SIZE 128 

void bench(int allocs){

  void *buffer[BUFFER];
  for(int i = 0; i < BUFFER; i++){
    buffer[i] = NULL;
  }

  for(int i = 0; i < allocs; i++){
    int index = rand() % BUFFER;
    if(buffer[index] != NULL){
      dfree(buffer[index]);
    }

    size_t size = (size_t)request(MAX_SIZE);
    int *memory;
    memory = dalloc(size);

    if(memory == NULL){
      fprintf(stderr, "allocation failed\n");
      return;
    }
    buffer[index] = memory;
    /* writing to the memory so we know it exists */
    *memory = 123;
  }
}

void checkFreeListLength(int allocs){
  printf("# Checking length of flist\n# Allocs\tflistLength\n");
  int sum, average = 0;
  for(int i = BUFFER; i < allocs+1; i+= 10){
    init();
    bench(i);
    sum+= printCountLengthOfFlist(i);
    average = sum/100;
    terminate();
  }
  printf("Average length of free list is: %d \n", average);
}

void checkFreeListDist(int allocs){
  printf("# Checking distribution of the block sizes in flist\n# Average\tflistLength\n");
  init();
  bench(allocs);
  printAverageSizeDistributionOfFlist();
  terminate();
}



void testSanity(int allocs){
  init();
  bench(allocs);
  sanity();
  terminate();
}

int main(int argc, char const *argv[]) {

  if(argc < 2){
    printf("Please enter:\tNumberOfAllocs\n");
    exit(1);
  }

  int allocs = atoi(argv[1]);
  BenchmarkPalloc(allocs);
  //sleep(1);
  checkFreeListLength(allocs);
  //sleep(1);
  checkFreeListDist(allocs);
  //sleep(0.1);
  //testSanity(allocs);

  return 0;
}