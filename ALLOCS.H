#ifndef ALLOCS_HEADER
#define ALLOCS_HEADER

#include <stdlib.h>

#ifndef ALLOCS_IMPL

#undef malloc
#undef free

#define malloc malloc_stat
#define calloc calloc_stat
#define free   free_stat

#endif

extern unsigned long malloc_count;
extern unsigned long calloc_count;
extern unsigned long realloc_count;
extern unsigned long free_count;

extern unsigned long alloc_bytes;
extern unsigned long alloc_max_bytes;

extern unsigned long free_count;

void *malloc_stat( size_t size );
void *calloc_stat( size_t n, size_t size );
void *realloc_stat( void *old_blk, size_t size );
void free_stat( void *p );

void print_mem_stat();

#endif
