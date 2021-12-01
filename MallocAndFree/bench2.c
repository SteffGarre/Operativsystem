#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "random.h"
#include "dlmallTaken.h"
#define TEST_CONST 100 
#define BLOCKS 1000
#define LOOP 1000
#define ROUNDS 10000

void allocate(int allocs){
  
  // create an array of void ptrs and set them all to null.
  //This will allow a sort of randomizer between calling dfree() and dalloc()
  //Note: there is a higher probalistic that the dalloc() func is called.
  void *test_const[TEST_CONST];
  for(int i = 0; i < TEST_CONST; i++){
    test_const[i] = NULL;
  }

  //allocs will initially be = TEST_CONST, but will increase in size until
  //it reaches the requested # of allocation from main!
  for(int i = 0; i < allocs; i++){

    //randomize a number between 0 and TEST_CONST
    int index = rand() % TEST_CONST;
    
    if(test_const[index] != NULL){
      dfree(test_const[index]);
    }

    size_t size = (size_t)request();
    int *memory;
    memory = dalloc(size);

    if(memory == NULL){
      fprintf(stderr, "allocation failed\n");
      return;
    }

    //write to array, increasing the chance that dfree() is called
    test_const[index] = memory;

    // writing to the memory to make sure that it exists
    *memory = 123;
  }
}


//Starts with TEST_CONST number of allocations until we reach the requested # of allocation from main! 
//Increases by +20 allocations for every iteration
//Checks length of flist for every iteration
void check_flist_length(int allocs){

  //printf("** Checking length of flist and average byte-size **\n#Allocations\tflistLength\tAverage size in flist\n");
  for(int i = TEST_CONST; i < allocs+1; i+= 20){
    init();
    allocate(i);
    freelistlength(i);
    terminate();
  }
  printf("\n");
}


//Starts with TEST_CONST number of allocations until we reach the requested # of allocation from main! 
//Increases by +20 allocations for every iteration
//checks time performane vs # of allocations for every iteration
void checkDalloc(int allocs){

  //printf("** Testing of time performance vs number of allocations **\n#Allocations\tTime(ms)\n");
  clock_t start, stop;
  double timeAlloc = 0;

  for(int i = TEST_CONST; i < allocs+1; i+= 20){
    init();
    start = clock();
    allocate(i);
    stop = clock();
    terminate();
    timeAlloc = ((double)(stop - start)) / ((double)CLOCKS_PER_SEC/1000);
    //printf("%d\t\t%f ms\n", i, timeAlloc);
    printf("%d\t%f\n", i, timeAlloc);
  }
  printf("\n");
}

void checkTakenOpt(){

  printf("** Testing taken optimization **\n");
  int *array[BLOCKS];
    init();
    clock_t start, stop;
  double timeAlloc = 0;
    start = clock();
    
    for(int i = 0; i < BLOCKS; i++){
        array[i] = dalloc(16);
    }
    
    for(int i = 0; i < ROUNDS; i++){
        for(int j = 0; j < LOOP; j++){
            *array[j] = 100;
        }
    }

    stop = clock();
  terminate();
  timeAlloc = ((double)(stop - start)) / ((double)CLOCKS_PER_SEC/1000);
    printf("Time elapsed: %f ms\n\n", timeAlloc);
}


int main(int argc, char const *argv[]) {

  if(argc < 2){
    printf("Error: Please enter a number of allocations\n");
    exit(1);
  }

  int allocs = atoi(argv[1]);
  //checkDalloc(allocs);
  //check_flist_length(allocs);
  checkTakenOpt();
  return 0;
}