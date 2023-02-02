#include "part.h"
#include "cache.h"

void cache_mark_dirty(cache_block_t *blk)
{
    blk->dirty = 1;
}

int cache_flush(cache_block_t *blk)
{
    if (!blk->part) return OK;

    if (blk->dirty) {
        if (disk_write_rel(blk->part, blk->sector, blk->data, 1) == FAILED) return FAILED;
        blk->dirty = 0;
    }

    return OK;
}

int in_cache(struct part_long *p, cache_block_t *blk, unsigned long sector)
{
	return blk->part == p && blk->sector == sector;
}

void * cache_read(struct part_long *p, cache_block_t *blk, unsigned long sector)
{
	if (in_cache(p, blk, sector)) return blk->data;

    cache_flush(blk);
    blk->part = p;
    if (disk_read_rel(blk->part, sector, blk->data, 1) == FAILED) return NULL;

    blk->sector = sector;
    blk->part = p;

    return blk->data;
}

