#include "diskio.h"

struct disk_info dinfo;

static void lba_to_chs(struct disk_addr *laddr, struct disk_addr_chs *chs)
{
	chs->disk = laddr->disk;
    chs->cyl  = CYL(laddr->sect);
    chs->head = HEAD(laddr->sect);
    chs->sect = SECT(laddr->sect);

}
int disk_read(struct disk_addr *daddr, void *buf, int num_sect)
{
	struct disk_addr_chs chs;
	
    lba_to_chs(daddr, &chs);

    return disk_read_chs(&chs, buf, num_sect);
}

int disk_write(struct disk_addr *daddr, void *buf, int num_sect)
{
	struct disk_addr_chs chs;
	
    lba_to_chs(daddr, &chs);

    return disk_write_chs(&chs, buf, num_sect);

}

int disk_format(struct disk_addr *daddr, void *ftable)
{
	struct disk_addr_chs chs;
	
    lba_to_chs(daddr, &chs);

    return disk_format_chs(&chs, ftable);

}

int disk_verify(struct disk_addr *daddr, int num_sect)
{
	struct disk_addr_chs chs;
	
    lba_to_chs(daddr, &chs);

    return disk_verify_chs(&chs, num_sect);
}

