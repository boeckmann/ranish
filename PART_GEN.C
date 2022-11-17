#include "part.h"

/*

bbt - Bad Blocks Table - will have numbers of bad sectors found
bbt_size - size of the table, if  0 halt on first bad sector,
                              if -1 return number of bad sectors.
generic_verify and
generic_format will return number of bad sectors found, or
               CANCEL, if user pressed ESC, or
               FAILED, if there was unrecoverable error ot bbt is too small

genric_clean

*/

int generic_verify(struct part_long *p, int bbt_size, unsigned long *bbt)
{
    char tmp[90];
    struct disk_addr daddr;
    unsigned long base_sect = 0;
    int s_sect, e_sect;
    int cyl, head, sect, num_sect, x, num_bad = 0, hd = dinfo.disk;

    disk_lock(hd);

    progress(MESG_VERIFYING);

    /* TODO: convert to LBA
    for (cyl = p->start_cyl; cyl <= p->end_cyl; cyl++) {
        for (head = ((cyl == p->start_cyl) ? p->start_head : 0);
             head < ((cyl == p->end_cyl) ? p->end_head + 1 : dinfo.num_heads);
             head++) {
            daddr.disk = hd;
            daddr.cyl  = cyl;
            daddr.head = head;

            s_sect = (cyl == p->start_cyl && head == p->start_head)
                         ? p->start_sect
                         : 1;
            e_sect = (cyl == p->end_cyl && head == p->end_head)
                         ? p->end_sect
                         : dinfo.num_sects;

            daddr.sect = s_sect;
            num_sect   = e_sect - s_sect + 1;

            if (disk_verify(&daddr, num_sect) == -1) {
                for (sect = s_sect; sect <= e_sect; sect++) {
                    daddr.sect = sect;
                    if (disk_verify(&daddr, 1) == -1) {
                        if (bbt_size != -1) {
                            if (num_bad == bbt_size) {
                                disk_unlock(hd);
                                return FAILED;
                            }
                            bbt[num_bad] = base_sect + sect;
                        }
                        num_bad++;
                    }
                }
            }

            base_sect += num_sect;

            x = base_sect * 100 / p->num_sect;

            sprintf(tmp, "%% %3d%%  Cylinder: %3d", x, cyl);
            if (progress(tmp) == CANCEL) {
                disk_unlock(hd);
                return CANCEL;
            }
        }
    }
    */

    disk_unlock(hd);
    return num_bad;
} /* generic_verify */

/*
   Some BIOSes have problems with mapping logical cylinders into physical
on the large hard disks. If you use destructive format on such systems it
may cause corruption of the several sectors at the beginning of the next
partition. To avoid it generic_format will not format first and last side
of the partition, but will verify it and clear with zeros.

*/

int generic_format(struct part_long *p, int bbt_size, unsigned long *bbt)
{
    char tmp[90];
    struct disk_addr daddr;
    unsigned long base_sect = 0;
    static unsigned char ftab[512];
    int cyl, head, sect, num_sect, x, num_bad = 0, hd = dinfo.disk;

    progress(MESG_FORMATTING);

    if (p->start_sect != 1 || p->end_sect != dinfo.num_sects) {
        progress(ERROR_FORMAT_FRACTION);
        return FAILED;
    }

    if (detected_os == SYS_WIN95) {
        progress(ERROR_FORMAT_WIN95);
        return FAILED;
    }

    disk_lock(hd);

    num_sect = dinfo.num_sects;

    /* -----> now we handle special case for the first side */

    daddr.disk = hd;
    daddr.sect = QUICK_BASE(p);
    /* TODO convert to LBA
    daddr.disk = hd;
    daddr.cyl  = p->start_cyl;
    daddr.head = p->start_head;
    daddr.sect = 0;

    memset(ftab, 0, 512);

    for (sect = 0; sect < num_sect; sect++) {
        daddr.sect = sect + 1;
        if (disk_verify(&daddr, 1) == -1) {
            if (bbt_size != -1) {
                if (num_bad == bbt_size) {
                    disk_unlock(hd);
                    return FAILED;
                }
                bbt[num_bad] = base_sect;
            }
            num_bad++;
        }
        base_sect++;
    }

    for (sect = 0; sect < num_sect; sect++) {
        daddr.sect = sect + 1;
        disk_write(&daddr, ftab, 1);
    } */

    /* -----> end special case */

    /*for (cyl = p->start_cyl; cyl <= p->end_cyl; cyl++) {
        for (head = ((cyl == p->start_cyl) ? p->start_head + 1 : 0);
             head < ((cyl == p->end_cyl) ? p->end_head : dinfo.num_heads);
             head++) {
            daddr.disk = hd;
            daddr.cyl  = cyl;
            daddr.head = head;
            daddr.sect = 0;

            for (sect = 0; sect < num_sect; sect++) {
                ftab[sect * 2 + 1] = sect + 1;
            }

            if (disk_format(&daddr, ftab) == -1) {
                progress(ERROR_FORMAT_GEN);
                disk_unlock(hd);
                return FAILED;
            }

            for (sect = 0; sect < num_sect; sect++)
                if (ftab[sect * 2] != 0)*/ /* is it a bad sector? */
              /*  {
                    if (bbt_size != -1) {
                        if (num_bad == bbt_size) {
                            disk_unlock(hd);
                            return FAILED;
                        }
                        bbt[num_bad] = base_sect + sect;
                    }
                    num_bad++;
                }

            base_sect += num_sect;

            x = base_sect * 100 / p->num_sect;

            sprintf(tmp, "%% %3d%%  Cylinder: %3d", x, cyl);
            if (progress(tmp) == CANCEL) {
                disk_unlock(hd);
                return CANCEL;
            }
        }
    } */

    /* -----> now we handle special case for the last side */

    /*daddr.disk = hd;
    daddr.cyl  = p->end_cyl;
    daddr.head = p->end_head;

    memset(ftab, 0, 512);

    for (sect = 0; sect < num_sect; sect++) {
        daddr.sect = sect + 1;
        if (disk_verify(&daddr, 1) == -1) {
            if (bbt_size != -1) {
                if (num_bad == bbt_size) {
                    disk_unlock(hd);
                    return FAILED;
                }
                bbt[num_bad] = base_sect + sect;
            }
            num_bad++;
        }
    } 

    for (sect = 0; sect < num_sect; sect++) {
        daddr.sect = sect + 1;
        disk_write(&daddr, ftab, 1);
    } 

    sprintf(tmp, "%% 100%%  Cylinder: %3d", p->end_cyl);
    progress(tmp);*/

    /* -----> end special case */

    disk_unlock(hd);
    return num_bad;
} /* generic_format */

int format_unused(struct part_long *p, char **argv)
{
    int x;

    if (argv == 0 || *argv == 0 ||
        _stricmp(*argv, "/clean") != 0 &&
            _stricmp(*argv, "/destructive") != 0) {
        progress("^I will not do that.");
        progress(
            "To enforce operation use option \"/clean\" or \"/destructive\" ");
        return FAILED;
    }

    flush_caches();

    if (_stricmp(*argv, "/clean") == 0) {
        x = generic_clean(p);
    } else /* /destructive */
    {
        x = generic_format(p, -1, 0);
    }

    if (x < 0)
        return x;

    return OK;
} /* format_unused */

int generic_clean(struct part_long *p)
{
    char *z, tmp[90];
    struct disk_addr daddr;
    unsigned long total_done = 0;
    int cyl, head, s_sect, e_sect, num_sect, x, hd = dinfo.disk;

    if ((z = (char *)malloc(63 * SECT_SIZE)) == 0) {
        show_error(ERROR_MALLOC);
        return FAILED;
    }

    memset(z, 0, 63 * SECT_SIZE);

    progress(MESG_CLEANING);

    disk_lock(hd);

    /* TODO: convert to LBA
    for (cyl = p->start_cyl; cyl <= p->end_cyl; cyl++) {
        for (head = ((cyl == p->start_cyl) ? p->start_head : 0);
             head < ((cyl == p->end_cyl) ? p->end_head + 1 : dinfo.num_heads);
             head++) {
            daddr.disk = hd;
            daddr.cyl  = cyl;
            daddr.head = head;

            s_sect = (cyl == p->start_cyl && head == p->start_head)
                         ? p->start_sect
                         : 1;
            e_sect = (cyl == p->end_cyl && head == p->end_head)
                         ? p->end_sect
                         : dinfo.num_sects;

            daddr.sect = s_sect;
            num_sect   = e_sect - s_sect + 1;

            disk_write(&daddr, z, num_sect);

            total_done += num_sect;

            x = total_done * 100 / p->num_sect;

            sprintf(tmp, "%% %3d%%  Cylinder: %3d", x, cyl);
            if (progress(tmp) == CANCEL) {
                disk_unlock(hd);
                free(z);
                return CANCEL;
            }
        }
    }
    */
    disk_unlock(hd);
    free(z);
    return OK;
} /* generic_clean */
