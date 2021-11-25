#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


struct head{
  uint16_t bfree;       // 2 bytes, the status of block before
  uint16_t bsize;       // 2 bytes, the size of block before
  uint16_t free;        // 2 bytes, the status of the block
  uint16_t size;        // 2 bytes, the size (max 2^16 ie 64 Kbyte)
  struct head *next;    // 8 bytes pointer
  struct head *prev;    // 8 bytes pointer
};

//macros
#define TRUE 1
#define FALSE 0
#define HEAD (sizeof(struct head)) //is the size of the head structure, 24 bytes.
#define MIN(size) (((size)> (8))?(size):(8)) //The minimum size that we will hand out is 8 bytes

//The limit is the size a block has to be larger than in order to split it.
//EX: If we want to split a block to accommodate a block of 32 bytes it has
//to be 62 (8 + 24 + 32) or larger. The smallest block we will split is a block 
//of size 40 that could be divided up into two 8 byte blocks (24 + 40 = 24 + 8 + 24 + 8).
#define LIMIT(size) (MIN(0) + HEAD + size)

#define MAGIC(memory) ((struct head*)memory - 1)
#define HIDE(block) (void*)((struct head*)block + 1)

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

  struct head *aft = after(split);
  aft->bsize = splt->size;

  return splt;
}