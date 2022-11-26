#include "diskio.h"

struct disk_info dinfo;

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

int get_disk_info(int hd, struct disk_info *di, char *buf_4096)
{
	int result;

	if ((result = _get_disk_info(hd, di, buf_4096)) == -1) return result;

	if (di->lba) {
		/* comment out to force disk to 2TB size for testing */
		/* di->total_sects = 0xffffffff; */

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

		di->sect_per_cyl = di->num_heads * di->num_sects;

		di->num_cyls = di->total_sects / di->sect_per_cyl;
		if (di->total_sects % di->sect_per_cyl) {
			di->num_cyls++;
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

