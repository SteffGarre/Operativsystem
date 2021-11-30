void *dalloc(size_t requested);
void dfree(void *memory);
void sanity();
void traverseblocks();
void init();
int freelistlength();
void sizes(int *buffer, int max);
void terminate();
int printCountLengthOfFlist(int numOfAllocs);