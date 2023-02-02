#ifndef CACHE_H
#define CACHE_H

typedef struct cache_block {
	unsigned char data[SECT_SIZE];
	struct part_long *part;
	unsigned long sector;
	unsigned char dirty;
} cache_block_t;


void cache_mark_dirty(cache_block_t *blk);
int cache_flush(cache_block_t *blk);
int in_cache(struct part_long *p, cache_block_t *blk, unsigned long sector);
void * cache_read(struct part_long *p, cache_block_t *blk, unsigned long sector);

#endif
