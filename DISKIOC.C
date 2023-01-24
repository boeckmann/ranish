#include "diskio.h"

struct disk_info dinfo;

unsigned long force_num_cyls = 0;
unsigned short force_num_heads = 0;
unsigned short force_num_sects = 0;
unsigned long reported_total_sectors = 0;

#ifdef __cplusplus
extern "C"
{
#endif

    int __cdecl _get_disk_info(int hd, struct disk_info *, char *buf_4096);

    int __cdecl disk_op_lba(struct disk_addr *, void *, int num_sect, int operation);

    int __cdecl disk_read_chs(struct disk_addr_chs *, void *, int num_sect);
    int __cdecl disk_write_chs(struct disk_addr_chs *, void *, int num_sect);
    int __cdecl disk_format_chs(struct disk_addr_chs *, void *ftable);
    int __cdecl disk_verify_chs(struct disk_addr_chs *, void *, int num_sect);


#ifdef __cplusplus
}
#endif

static void lba_to_chs(struct disk_addr *laddr, struct disk_addr_chs *chs)
{
	chs->disk = laddr->disk;
    chs->cyl  = CYL(laddr->sect);
    chs->head = HEAD(laddr->sect);
    chs->sect = SECT(laddr->sect);

}

unsigned long test_sectors[] = { 0x10000000, 0xfffffff, 16450559, 16434494, 0 };

int get_disk_info(int hd, struct disk_info *di, char *buf_4096)
{
	int result, i;
	struct disk_addr addr;

	if ((result = _get_disk_info(hd, di, buf_4096)) == -1) return result;
    reported_total_sectors = dinfo.total_sects;
	
	if (di->lba && lba_enabled) {
		/* comment out to force disk to 2TB size for testing */
		/* di->total_sects = 0xffffffff; */

		if ((di->num_heads <= 16) && 
			(di->total_sects > 63ul * 16ul * 1024ul) &&
			(force_num_heads == 0)) {
			/* calculate virtual CHS value */
			if (di->total_sects > 63ul * 128ul * 1024ul) {
				di->num_heads = 255;
			}
			else if (di->total_sects > 63ul * 128ul * 1024ul) {
				di->num_heads = 128;
			}
			else if (di->total_sects > 63ul * 32ul * 1024ul) {
				di->num_heads = 64;
			}
			else if (di->total_sects > 63ul * 16ul * 1024ul) {
				di->num_heads = 32;
			}
			else {
				di->num_heads = 16;			
			}			
		}

        addr.disk = dinfo.disk;
        addr.sect = dinfo.total_sects - 1;

        /* test if last sector is accessable */
        if (disk_read(&addr, buf_4096, 1) != 0) {
        	i = 0;
        	while(test_sectors[i]) {
        		addr.sect = test_sectors[i];
        		if (disk_read(&addr, buf_4096, 1) == 0) {
					di->total_sects = test_sectors[i] + 1;
					break;
				}
        	}
	        if (test_sectors[i] == 0) return -1;
        }
	}
	
	if (force_num_sects) {
		di->num_sects = force_num_sects;
	}
	if (force_num_heads) {
		di->num_heads = force_num_heads;		
	}
	if (force_num_cyls) {
		di->num_cyls = force_num_cyls;
	}

	if ((di->lba && lba_enabled) || force_num_heads || force_num_sects) {
		di->sect_per_cyl = di->num_heads * di->num_sects;
		if (!force_num_cyls) {
		/* recalculate cylinder count based on total sectors and
		   sectors per cylinder */
			di->num_cyls = di->total_sects / di->sect_per_cyl;
			if (di->total_sects % di->sect_per_cyl) {
				di->num_cyls++;
			}
		}	
		else {
			di->total_sects = ((unsigned long) di->num_cyls) * di->num_heads * di->num_sects;
			reported_total_sectors = di->total_sects;
		}
	}
	
	return result;
}

int disk_read(struct disk_addr *daddr, void *buf, int num_sect)
{
	struct disk_addr_chs chs;
	int result;

    if (dinfo.lba) {
    	result = disk_op_lba(daddr, buf, num_sect, INT13_READ_EXT);
    } else {
	    lba_to_chs(daddr, &chs);
	    result = disk_read_chs(&chs, buf, num_sect);
    }

    return result;
}


int disk_write(struct disk_addr *daddr, void *buf, int num_sect)
{
	struct disk_addr_chs chs;
	int result;

    if (dinfo.lba) {
    	result = disk_op_lba(daddr, buf, num_sect, INT13_WRITE_EXT);
    } else {
	    lba_to_chs(daddr, &chs);
	    result = disk_write_chs(&chs, buf, num_sect);
    }

    return result;
}


int disk_buffered_read(struct disk_addr *daddr, void *buf, int num_sect)
{
	return disk_read(daddr, buf, num_sect);
}


int disk_buffered_write(struct disk_addr *daddr, void *buf, int num_sect)
{
	return disk_write(daddr, buf, num_sect);
}


int disk_format(struct disk_addr *daddr, void *ftable)
{
	struct disk_addr_chs chs;
	
    lba_to_chs(daddr, &chs);

    return disk_format_chs(&chs, ftable);

}

int disk_verify(struct disk_addr *daddr, void *buf, int num_sect)
{
	struct disk_addr_chs chs;
	int result;

    if (dinfo.lba) {
    	result = disk_op_lba(daddr, buf, num_sect, INT13_VERIFY_EXT);
    } else {
	    lba_to_chs(daddr, &chs);
	    result = disk_verify_chs(&chs, buf, num_sect);
    }

    return result;
}

