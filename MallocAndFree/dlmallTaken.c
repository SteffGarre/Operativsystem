#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h> 

struct taken{
  uint16_t bfree;       // 2 bytes, the status of block before
  uint16_t bsize;       // 2 bytes, the size of block before
  uint16_t free;        // 2 bytes, the status of the block
  uint16_t size;        // 2 bytes, the size (max 2^16 ie 64 Kbyte)
};

struct head{
  uint16_t bfree;       // 2 bytes, the status of block before
  uint16_t bsize;       // 2 bytes, the size of block before
  uint16_t free;        // 2 bytes, the status of the block
  uint16_t size;        // 2 bytes, the size (max 2^16 ie 64 Kbyte)
  struct head *next;    // 8 bytes pointer
  struct head *prev;    // 8 bytes pointer
};

#define TRUE 1
#define FALSE 0


#define HEAD (sizeof(struct taken)) //is the size of the head structure, 8 bytes.
#define MIN(size) (((size)> (16))?(size):(16)) //The minimum size that we will hand out is 16 bytes
#define MAGIC(memory) ((struct taken*)memory - 1)
#define HIDE(block) (void*)((struct taken*)block + 1)


#define LIMIT(size) (MIN(0) + HEAD + size)
#define ALIGN 8
#define ARENA (64*1024)


//before and after section 
struct head *after(struct head *block){
  return (struct head*)((char*) block + block->size + HEAD);
}

struct head *before (struct head *block){
  return (struct head*)((char*) block - block->bsize - HEAD);
}

//split a block
struct head *split(struct head *block, int size){
  int rsize = block->size - size - HEAD;
  block->size = rsize;

  struct head *splt = after(block);
  splt->bsize = block->size;
  splt->bfree = block->free;
  splt->size = size;
  splt->free = FALSE;

  struct head *aft = after(splt);
  aft->bsize = splt->size;

  return splt;
}

//create a new block using mmap(), called once.
struct head *arena = NULL;

struct head *new(){

  if(arena != NULL){
    printf("one arena already allocated \n");
    return NULL;
  }

  // using mmap,
  struct head *new = mmap(NULL, ARENA, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                      
  if(new == MAP_FAILED){
    printf("mmap failed: error %d\n", errno);
    return NULL;
  }

  //make room for a head dummy

  unsigned int size = ARENA - 2*HEAD;

  /*Before we return the new block we set it up to prevent our algorithm to fall outside of the arena. 
  We set the bfree flag of the block to false to prevent anyone from trying to merge it with a non existing 
  block before. We also set up a sentinel block in the end of the arena and mark the free flag to false. 
  This will prevent anyone from trying to merge the block with the sentinel block.
*/

  new->bfree = FALSE;
  new->bsize = 0;
  new->free = TRUE;
  new->size = size;

  struct head *sentinal = after(new);

  //only touch the status fields
  sentinal->bfree = new->free;
  sentinal->bsize = size;
  sentinal->free = FALSE;
  sentinal->size = 0;

  //this is the only arena we have
  arena = (struct head*)new;
  return new;
}


/*
All free blocks will be linked in a double linked list. 
We define a procedures to detach() a block from the list and one to insert() a new block in the list. 
Since the list is so far unordered we will always insert a block to the front of the list.
*/
struct head* flist;

void detach(struct head *block){

  if(block->next != NULL){
    block->next->prev = block->prev;
  }
  
  if(block->prev != NULL){
    block->prev->next = block->next;
  } else{
    flist = block->next;
  }

}

void insert (struct head *block){
  block->next = flist;
  block->prev = NULL;

  if(flist != NULL){
    flist->prev= block;
  }

  flist = block;

}


// *** Function bellow implement Malloc and Free ***

//adjust a requested size to be a of min size of 8 bytes and a multiple of 8.
int adjust (size_t size){

  int minsize = MIN(size);

  if(minsize%ALIGN == 0){
    return size;
  }else{
    int adj = minsize%ALIGN;
    return (minsize + ALIGN -adj);
  }
}

//function tries to find an available block in flist
struct head *find(size_t size){
  struct head *temp = flist;

  while(temp != NULL){

    if(temp->size >= size){
      detach(temp);

      if(temp->size >= LIMIT(size)){

        struct head *block = split(temp,size);
        insert(temp);
        struct head *aft = after(block);
				aft->bfree = FALSE;
				block->free = FALSE;
				return block;

      }else{

        temp->free = FALSE;
        struct head *aft = after(temp);
        aft->bfree = FALSE;
        return temp;

      }
    }else{  
      temp = temp->next;
    }
    
  }
  return NULL;
}

//function tries to merge with any adjecent blocks.
struct head *merge(struct head *block){

  struct head *aft = after(block);
	
	
	if(block->bfree == TRUE){
	
		struct head *bef = before(block);
		detach(bef);
		
		bef->size = bef->size + block->size + HEAD;
		aft->bsize = bef->size;
		block = bef;
	}

	if(aft->free == TRUE){

		detach(aft);

		block->size = block->size + aft->size + HEAD;
		aft = after(block);
		aft->bsize = block->size;
	}

	return block;
}

// allocates a block of memory of a requested size.
//adjust() adjustes the size to be a min of 8 bytes or a multiple of 8
//find() looks in flist to see if there is a available block.
//If we find a block, the macro HIDE returns a ptr to the data segment: ie hides its header.
void *dalloc(size_t request){

  if(request <= 0){
    return NULL;
  }

  int size = adjust(request);
  struct taken *taken =(struct taken*) find(size);

  if(taken == NULL){
    return NULL;
  } else {
    return HIDE(taken);
  }
}

//free a previously allocated block of memory
//if merge is implemented, the block tries to merge with any adjecent blocks.
void dfree (void *memory){

  if(memory != NULL){
    struct head *block =  (struct head*) MAGIC(memory);

    // call merge function
   //block = merge(block);

    struct head *aft = after(block);
    block->free = TRUE;
    aft->bfree = block->free;
    insert(block); 
  }
  
  return;
}

//goes through the arena and all the blocks and prints their header values.
void traverseblocks(){
	struct head *temp = arena;
	
	while(temp->size != 0){
		printf("address: %p\t, free: %d\t, size: %d\t, bfree: %d\t, bsize: %d\t\n", 
    temp, temp->free, temp->size, temp->bfree, temp->bsize);
		temp = after(temp);
	}
}

//checks if the freelist and arena looks ok
void sanity(){

	struct head *sanity = flist;
	struct head *prev = sanity->prev;
	
	int size = sanity->size;
	
	while(sanity != NULL && sanity->size != 0){

		//Check so that block in the freelist actually is free
		if(sanity->free != TRUE){
			printf("Error - found a block that is not free");
			exit(1);
		}
		
		// Check so that block in the freelist actually is aligned
		if(sanity->size % ALIGN != 0){
			printf("Error - found a block that is not aligned");
			exit(1);
		}
		
		if(sanity->prev != prev){
			printf("Error - found a block with incorrect prev");
			exit(1);
		}
		
		prev = sanity;
		sanity = sanity->next;
	}
	printf("Sanity check OK, no problems found! \n");
}

//goes through flist and counts the # of free blocks.
int freelistlength(int numOfAllocs){
  int count = 0;
  int average, sum = 0;
  struct head *next = flist;
  while(next != NULL){
    count++;
    sum += next->size;
    next = next->next;
  }
  average = (int)ceil(sum / count);
  //printf("%d\t\t%d\t\t%d bytes\n", numOfAllocs, count, average);
  printf("%d\t%d\t%d\n", numOfAllocs, count, average);
  return count;
}

//initializes the arena and inserts it to the flist.
void init(){
	struct head *first = new();
	insert(first);
}

void terminate(){
  munmap(arena, ARENA);
  arena = NULL;
  flist = NULL;
}


