#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "random.h"
#include "dlmall.h"
#define BUFFER 100 
#define MAX_SIZE 100

void allocate(int allocs){

  void *buffer[BUFFER];
  for(int i = 0; i < BUFFER; i++){
    buffer[i] = NULL;
  }

  for(int i = 0; i < allocs; i++){
    int index = rand() % BUFFER;

    if(buffer[index] != NULL){
      dfree(buffer[index]);
    }

    size_t size = (size_t)request();
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
    allocate(i);
    sum+= printCountLengthOfFlist(i);
    average = sum/100;
    terminate();
  }
  printf("Average length of free list is: %d \n", average);
}

void BenchmarkPalloc(int allocs){
  printf("# Evaluation of time performance\n# Allocs\tTime(ms)\n");
  clock_t time_start, time_stop;
  double timeAlloc = 0;
  for(int i = BUFFER; i < allocs+1; i+= 10){
    init();
    time_start = clock();
    allocate(i);
    time_stop = clock();
    terminate();
    timeAlloc = ((double)(time_stop - time_start)) / ((double)CLOCKS_PER_SEC/1000);
    printf("%d\t%f ms\n", i, timeAlloc);
  }
}

void checkSanity(int allocs){
  init();
  allocate(allocs);
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
  checkFreeListLength(allocs);
  //checkSanity(allocs);
  
  return 0;
}