void *dalloc(size_t requested);
void dfree(void *memory);
void sanity();
void traverseblocks();
void init();
void terminate();
int freelistlength(int numOfAllocs);