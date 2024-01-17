
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#define ALLOCS_IMPL

unsigned long malloc_count = 0;
unsigned long calloc_count = 0;
unsigned long realloc_count = 0;
unsigned long alloc_bytes = 0;
unsigned long alloc_max_bytes = 0;

unsigned long free_count = 0;

static void update_stats( size_t size )
{
   alloc_bytes += size;

   if ( alloc_bytes > alloc_max_bytes ) {
      alloc_max_bytes = alloc_bytes;
   }
}

void *calloc_stat( size_t n, size_t size )
{
   void *res = calloc( n, size );
   calloc_count++;
   if ( !res ) {
      return res;
   }

   update_stats( _msize( res ) );
   return res;
}

void *malloc_stat( size_t size )
{
   void *res = malloc( size );
   malloc_count++;
   if ( !res ) {
      return res;
   }

   update_stats( _msize( res ) );
   return res;
}

void *realloc_stat( void *old_blk, size_t size )
{
   size_t old_size;
   void *res;

   old_size = old_blk ? _msize( old_blk ) : 0;
   realloc_count++;
   alloc_bytes -= old_size;
   res = realloc( old_blk, size );
   if ( !res ) {
      return res;
   }

   update_stats( _msize( res ) );
   return res;
}

void free_stat( void *p )
{
   free_count++;
   alloc_bytes -= _msize( p );
   free( p );
}

void print_mem_stat()
{
   printf( "--- MALLOC STATISTICS -----------\n" );
   printf( "malloc calls     : %lu\n", malloc_count );
   printf( "calloc calls     : %lu\n", calloc_count );
   printf( "realloc calls    : %lu\n", realloc_count );
   printf( "free calls       : %lu\n", free_count );
   printf( "allocated mem now: %lu bytes\n", alloc_bytes );
   printf( "max allocated mem: %lu bytes\n", alloc_max_bytes );
}
