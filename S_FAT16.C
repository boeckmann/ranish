#include "part.h"
#include "fat.h"
#include <time.h>
#include <ctype.h>

/* defined in S_FAT32.C */
unsigned long fat_calc_hidden_sect(struct part_long *p);


#define BBT_SIZE 128

#define F_NORM  0
#define F_VERIFY 1
#define F_DESTR 2

#define ROOT_ENTR   (512)
#define ROOT_SIZE   (ROOT_ENTR / DIRENT_PER_SECT)
#define MAX_CLUST12 (4084)  /* Maximum number of clusters in FAT12 system */
#define MAX_CLUST16 (65524) /* Maximum number of clusters in FAT16 system */

#define FAT12_MAX_SIZE 6144   /* 4096 12-bit entries */

/* according to Microsoft FAT specification */
unsigned fat16_cluster_size(unsigned long sectors)
{
    if (sectors <= 8400) return 0;
    else if (sectors <= 32680) return 2;
    else if (sectors <= 262144) return 4;
    else if (sectors <= 524288) return 8;
    else if (sectors <= 1048576) return 16;
    else if (sectors <= 2097152) return 32;
    else if (sectors <= 4194304) return 64;
    else return 128;
}

int fat16_update_label_file(struct part_long *p, struct boot_ms_dos *b)
{
    unsigned char *buf;
    struct dirent *dirent;
    unsigned root_sect_cnt, j;
    unsigned long start_sect, sect;

    if ((buf = malloc(SECT_SIZE)) == NULL) return FAILED;

    root_sect_cnt = b->root_entr / DIRENT_PER_SECT;
    start_sect = b->res_sects + b->num_fats * b->fat_size;

    for (sect = start_sect; sect < start_sect + root_sect_cnt; sect++)
    {
        if (disk_read_rel(p, sect, buf, 1) == FAILED) goto failed;
        dirent = (struct dirent *)buf;

        for (j = 0; j < DIRENT_PER_SECT; j++) {
            if (dirent->name[0] == 0 || 
                    (dirent->attr & DIRENT_LONG_NAME_MASK) != DIRENT_LONG_NAME_MASK &&
                    ((dirent->attr) & (DIRENT_ATTR_VOL | DIRENT_ATTR_DIR))  == DIRENT_ATTR_VOL) {  

                if (memcmp(b->label, NO_NAME_LABEL, 11)) {
                    /* update label or create new one if not found */
                    memset(dirent, 0, sizeof(struct dirent));
                    dirent->attr |= DIRENT_ATTR_VOL;
                    memcpy(dirent->name, b->label, sizeof(b->label));
                }
                else if (dirent->name[0] != 0) {
                    /* delete label file if it exists and label is "NO NAME" */
                    dirent->name[0] = 0xe5;
                }
                if (disk_write_rel(p, sect, buf, 1) == FAILED) goto failed;
                goto success;
            }

            dirent++;
        }
    }

success:
    free(buf);
    return OK;

failed:
    free(buf);
    return FAILED;
}

/*   0x01, "DOS FAT-12"			*/
/*   0x04, "DOS FAT-16 (<=32Mb)"	*/
/*   0x06, "BIGDOS FAT-16 (>=32Mb)"	*/

int format_fat(struct part_long *p, char **argv)
{
    char *data_pool;
    struct boot_ms_dos *b;
    unsigned short int *fat;
    int i, j, k, wr_sect, ret_code, sys_type, next_bad;
    unsigned long num_clust, num_sect, base_sect, *bbt;
    long fat_size;

    unsigned int num_bad    = 0;
    unsigned int clust_size = 4;
    unsigned int form_type  = F_NORM;

    if ((data_pool = malloc(SECT_SIZE + FAT12_MAX_SIZE + BBT_SIZE * sizeof(long))) ==
        0) {
        show_error(ERROR_MALLOC);
        return FAILED;
    }

    b   = (struct boot_ms_dos *)(data_pool);
    fat = (unsigned short *)(data_pool + SECT_SIZE);
    bbt = (unsigned long *)(data_pool + SECT_SIZE + FAT12_MAX_SIZE);

    memmove(b, FAT_BOOT, SECT_SIZE);
    memmove(b->sys_id, "MSDOS5.0", 8);
    b->sect_size  = SECT_SIZE;
    b->res_sects  = 1;
    b->num_fats   = 2;
    b->root_entr  = ROOT_ENTR;
    b->media_desc = 0xF8;
    b->ext_signat = 0x29;
    memmove(b->label, "NO NAME    ", 11);
    b->magic_num = MBR_MAGIC_NUM;

    num_sect = p->num_sect;

    while (*argv != 0) {
        if (_stricmp(*argv, "/destructive") == 0)
            form_type = F_DESTR;
        else if (_stricmp(*argv, "/verify") == 0)
            form_type = F_VERIFY;
        else if (_strnicmp(*argv, "/c:", 3) == 0) {
            k = atoi((*argv) + 3);
            for (i = k / 2, j = 0; i != 0 && j < 7; j++, i /= 2)
                ;
            clust_size = 1 << j;
            if (clust_size != k) {
                progress("^Invalid cluster size.");
                goto failed;
            }
        } else if (_strnicmp(*argv, "/l:", 3) == 0) {
            strncpy(tmp, (*argv) + 3, 11);
            tmp[11] = 0;
            for (i = 0; tmp[i] != 0 && i < 11; i++)
                b->label[i] = tmp[i];
            for (; i < 11; i++)
                b->label[i] = ' ';
        } else {
            progress("^Unknown option:");
            progress(*argv);
            goto failed;
        }
        argv++;
    }

    if (QUICK_BASE(p) > QUICK_BASE(p) + num_sect) {
        progress("^Partition crosses 2TiB boundary. Refusing to format.");
        goto failed;
    }

    if (p->os_id == 0x0400 || p->os_id == 0x0600 || /* FAT16 */
        p->os_id == 0x0e00 || p->os_id == 0x1e00 ||
        p->os_id == 0x1400 || p->os_id == 0x1600) {

        clust_size = fat16_cluster_size(num_sect);

        if (clust_size == 0) {
            progress("^FAT16 partition too small. Use FAT12 instead!");
            goto failed;
        }

        fat_size = ((unsigned long long)(num_sect - ROOT_SIZE - 1) + (clust_size * 256 + 2 - 1)) /
                       (clust_size * 256 + 2);

        num_clust = (num_sect - ROOT_SIZE - 1 - 2 * fat_size) / (clust_size);

        if ((clust_size == 128) || (num_clust > MAX_CLUST16)) {
            progress("^FAT16 partition too large. Use FAT32 instead!");
            goto failed;
        }
        
        memmove(b->fs_id, "FAT16   ", 8);

        sys_type = 16;
    }

    if (p->os_id == 0x0100 || p->os_id == 0x1100) /* FAT12 */
    {
        while (clust_size < 64) {
            if (1 + 24 + ROOT_SIZE + (unsigned long)clust_size * MAX_CLUST12 >
                num_sect)
                break;
            clust_size *= 2;
        }

        if (clust_size > 8) {
            progress("^Partition is too big. Use FAT16 instead!");
            goto failed;
        }

        fat_size = (num_sect - ROOT_SIZE - 1 + 2 * clust_size) /
                       ((long)clust_size * 512 * 2 / 3 + 2) +
                   1;
        num_clust = (num_sect - ROOT_SIZE - 1 - 2 * fat_size) / (clust_size);

        memmove(b->fs_id, "FAT12   ", 8);

        sys_type = 12;
    }

    if (fat_size < 0 ||
        1 + 2 * fat_size + ROOT_SIZE + clust_size > num_sect) {
        progress("^Partition is too small.");
        goto failed;
    }

    b->fat_size   = fat_size;
    b->clust_size = clust_size;

    b->drive_num = dinfo.disk;
    b->num_sects = dinfo.num_sects;
    b->num_sides = dinfo.num_heads;

    b->hid_sects  = fat_calc_hidden_sect(p);
    b->total_sect = (num_sect < 65536L) ? num_sect : 0;
    b->big_total  = (num_sect < 65536L) ? 0 : num_sect;

    b->serial_num =
        ((p->rel_sect << 16) + (p->num_sect * ((long)b % 451))) +
        ((dinfo.total_sects % 12345L) ^ (dinfo.total_sects * 67891L)) +
        ((dinfo.disk * 123L) ^ (dinfo.num_heads % 7)) + clock();

    flush_caches();

    if (form_type == F_DESTR)
        ret_code = generic_format(p, BBT_SIZE, bbt);
    else if (form_type == F_VERIFY)
        ret_code = generic_verify(p, BBT_SIZE, bbt);
    else
        ret_code = 0;

    if (ret_code < 0) /* format failed or canceled */
    {
        free(data_pool);
        return ret_code;
    }

    num_bad = ret_code;

    disk_lock(dinfo.disk);

    progress("^Initializing file system ...");

    if (num_bad != 0 && bbt[0] < 1 + 2 * fat_size + ROOT_SIZE) {
        progress(
            "Beginning of the partition is unusable. Try to move it forward.");
        goto failed;
    }

    progress("~Writing boot sector ...");

    if (disk_write_rel(p, 0, b, 1) == FAILED) /*  Writing boot sector  */
    {
        progress("Error writing boot sector.");
        goto failed;
    }

    progress("~Writing FAT tables ...");

    wr_sect = 1;

    if (sys_type == 16) {
        for (k = 0; k < 2; k++) /* Writing two copies of FAT16 */
        {
            next_bad  = 0;
            base_sect = 1 + 2 * fat_size + ROOT_SIZE;

            for (i = 0; i < fat_size; i++) {
                memset(fat, 0, 512);
                if (i == 0) {
                    fat[0] = 0xFFF8;
                    fat[1] = 0xFFFF;
                    while (next_bad != num_bad &&
                           bbt[next_bad] < base_sect + clust_size * 254)
                        fat[(bbt[next_bad++] - base_sect) / clust_size + 2] =
                            0xFFF7;
                    base_sect += clust_size * 254;
                } else {
                    while (next_bad != num_bad &&
                           bbt[next_bad] < base_sect + clust_size * 256)
                        fat[(bbt[next_bad++] - base_sect) / clust_size] =
                            0xFFF7;
                    base_sect += clust_size * 256;
                }

                if (disk_write_rel(p, wr_sect++, fat, 1) == FAILED) {
                    progress("Error writing FAT.");
                    goto failed;
                }
            }
        }
    }    /* fat16 */
    else /* Writing two copies of FAT12 */
    {
        struct fat12
        {
            unsigned c0 : 12;
            unsigned c1 : 12;
        } *fat12 = (struct fat12 *)fat;

        memset(fat, 0, 6144);

        fat12[0].c0 = 0xFF8;
        fat12[0].c1 = 0xFFF;

        next_bad  = 0;
        base_sect = 1 + 2 * fat_size + ROOT_SIZE;

        while (next_bad != num_bad) {
            j = (bbt[next_bad++] - base_sect) / clust_size + 2;

            if (j % 2 == 0)
                fat12[j / 2].c0 = 0xFF7;
            else
                fat12[j / 2].c1 = 0xFF7;
        }

        for (k = 0; k < 2; k++)
            for (i = 0; i < fat_size; i++)
                if (disk_write_rel(p, wr_sect++, fat + i * 256, 1) == FAILED) {
                    progress("Error writing FAT.");
                    goto failed;
                }
    } /* fat12 */

    memset(fat, 0, 512);

    progress("~Writing root directory ...");

    for (i = 0; i < ROOT_SIZE; i++)
        if (disk_write_rel(p, wr_sect++, fat, 1) == FAILED) {
            progress("Error writing root directory.");
            goto failed;
        }

    if (fat16_update_label_file(p, b) == FAILED) {
        progress("Error writing volume label.");
        goto failed;
    }

    disk_unlock(dinfo.disk);
    free(data_pool);
    return OK;

failed:

    disk_unlock(dinfo.disk);
    free(data_pool);
    return FAILED;
} /* format_fat */

int format_embr(struct part_long *p, char **argv)
{
    struct mbr *mbr = (struct mbr *)&tmp;

    flush_caches();

    disk_lock(dinfo.disk);

    progress("^Initializing Extended DOS partition ...");

    progress("~Writing Extended Master Boot Record ...");

    memset(mbr, 0, SECT_SIZE);
    memmove(mbr->x.std.code, EMP_IPL, EMP_IPL_SIZE);
    strncpy(mbr->x.std.code + EMP_IPL_SIZE,
            MESG_EXT_NONBOOT,
            sizeof(mbr->x.std.code) - EMP_IPL_SIZE);
    mbr->magic_num = MBR_MAGIC_NUM;

    if (disk_write_rel(p, 0, mbr, 1) == FAILED) {
        progress("Error Writing Extended Master Boot Record.");
        disk_unlock(dinfo.disk);
        return FAILED;
    }

    argv++; /* so compiler will not show warning */

    disk_unlock(dinfo.disk);
    return OK;
} /* format_embr */

int print_fat(struct part_long *p)
{
    struct boot_ms_dos *b = (struct boot_ms_dos *)tmp;
    char tmp1[20], tmp2[20];

    if (disk_read_rel(p, 0, b, 1) == -1) {
        fprintf(stderr, "Error reading boot sector.\n");
        return FAILED;
    }

    printf("\n              DOS Boot Sector Data              Expected "
           "Value\n\n");
    printf("                        System id:  %-.8s\n", b->sys_id);
    printf("                      Sector size:  %-3d         512\n",
           b->sect_size);
    printf("              Sectors per cluster:  %d\n", b->clust_size);
    printf("Reserved sectors at the beginning:  %d\n", b->res_sects);
    printf("             Number of FAT copies:  %d\n", b->num_fats);
    printf("           Root directory entries:  %d\n", b->root_entr);
    printf("            Total sectors on disk:  %-5u       %u\n",
           b->total_sect,
           (b->total_sect == 0 || p->num_sect > 65535L) ? 0 : p->num_sect);
    printf("            Media descriptor byte:  %02Xh         F8h\n",
           b->media_desc);
    printf("                  Sectors per FAT:  %d\n", b->fat_size);
    printf("                 Sectors per side:  %-2d          %d\n",
           b->num_sects,
           dinfo.num_sects);
    printf("                  Number of sides:  %-3d         %d\n",
           b->num_sides,
           dinfo.num_heads);
    printf("Hidden sectors prior to partition:  %-10s  %-10s\n",
           sprintf_long(tmp1, b->hid_sects),
           sprintf_long(tmp2, fat_calc_hidden_sect(p)));
    printf("      Big total number of sectors:  %-10s  %-10s\n",
           b->big_total == 0 ? "0" : sprintf_long(tmp1, b->big_total),
           (b->total_sect == 0 || p->num_sect > 65535L)
               ? sprintf_long(tmp2, p->num_sect)
               : "0");
    printf("                     Drive number:  %-3d         %d\n",
           b->drive_num,
           dinfo.disk);
    printf("   Extended boot record signature:  %02Xh\n", b->ext_signat);
    printf("             Volume serial number:  %08lX\n", b->serial_num);
    printf("                     Volume label:  %-.11s\n", b->label);
    printf("                 File system type:  %-.8s    %s\n",
           b->fs_id,
           (b->clust_size == 0)
               ? ("?")
               : (p->num_sect / b->clust_size < 4096 ? "FAT12" : "FAT16"));
    printf("         Boot sector magic number:  0%04Xh      0AA55h\n",
           b->magic_num);
    return 0;
} /* print_fat */

#define StX 5
#define StY 5
#define StW 66
#define StH 14

#define StX2 (StX + 30)
#define StX3 (StX + 69)

#define TEXT_COLOR (BrCyan + BakBlue)
#define DATA_COLOR (BrWhite + BakBlue)
#define EDIT_COLOR (BrWhite + BakBlack)

int setup_fat(struct part_long *p)
{
    struct event ev;
    int i, act, pos;
    char *tmp, *tmp1;
    unsigned long n, min_clust, max_clust, min_num_sect, max_num_sect, l, lc;
    unsigned fatsz, syst;
    unsigned long num_sect;

    struct boot_ms_dos *b, *b_orig,
        *fat_boot_code = (struct boot_ms_dos *)FAT_BOOT;

    if ((tmp = malloc(3 * SECT_SIZE)) == 0) {
        show_error(ERROR_MALLOC);
        return FAILED;
    }

    tmp1 = tmp + 100;
    /* tmp2=tmp+120; */
    b      = (struct boot_ms_dos *)(tmp + SECT_SIZE);
    b_orig = b + 1;

    if (disk_read_rel(p, 0, b, 1) == -1) {
        show_error("Error reading boot sector");
        free(tmp);
        return FAILED;
    }

    if ((b->clust_size < 1) ||
        (b->clust_size > 128) ||
    (b->sect_size != 512) ||
    (b->res_sects == 0)) {
       show_error("Partition seems not to be FAT formatted");
       free(tmp);
       return FAILED;
    }

    memmove(b_orig, b, SECT_SIZE);

    clear_window(TEXT_COLOR, 2, 5, 78, 19);
    write_string(HINT_COLOR, 45, 24, "         ESC");
    write_string(MENU_COLOR, 57, 24, " - Return to MBR view");

    write_string(HINT_COLOR, 64, 13, "F5");
    write_string(MENU_COLOR, 66, 13, " - SetExpVal");
    write_string(HINT_COLOR, 64, 14, "F6");
    write_string(MENU_COLOR, 66, 14, " - FixBoot2G");

    syst = 0;
    if (strncmp(b->fs_id, "FAT12", 5) == 0)
        syst = 12;
    if (strncmp(b->fs_id, "FAT16", 5) == 0)
        syst = 16;

    max_clust =
        (syst == 0) ? (0) : ((long)SECT_SIZE * 8u * b->fat_size / syst - 2u);

    write_string(TEXT_COLOR, StX, StY + 1, "                  System id:");
    write_string(TEXT_COLOR,
                 StX,
                 StY + 2,
                 "           File system type:                             "
                 "Media desc:");
    write_string(TEXT_COLOR,
                 StX,
                 StY + 3,
                 "        Sectors per cluster:                            "
                 "Sector size:");
    write_string(TEXT_COLOR,
                 StX,
                 StY + 4,
                 "            Sectors per FAT:                             "
                 "FAT copies:");
    write_string(TEXT_COLOR,
                 StX,
                 StY + 5,
                 "     Root directory entries:                               "
                 "Ext sign:");
    write_string(TEXT_COLOR,
                 StX,
                 StY + 6,
                 "       Volume serial number:                                "
                 "  Magic:");
    write_string(TEXT_COLOR,
                 StX,
                 StY + 7,
                 "               Volume label:               Expected Value");
    write_string(TEXT_COLOR, StX, StY + 8, "               Drive number:");
    write_string(TEXT_COLOR, StX, StY + 9, "            Number of sides:");
    write_string(TEXT_COLOR, StX, StY + 10, "           Sectors per side:");
    write_string(TEXT_COLOR,
                 StX,
                 StY + 11,
                 " Sectors prior to partition:               123456789");
    write_string(TEXT_COLOR,
                 StX,
                 StY + 12,
                 "    Total number of sectors:               12345");
    write_string(TEXT_COLOR,
                 StX,
                 StY + 13,
                 "Big total number of sectors:               123456789");
    write_string(TEXT_COLOR,
                 StX,
                 StY + 14,
                 "         Number of clusters:");
    
    
    write_string(TEXT_COLOR, StX, StY + 15, "     Minimum partition size:  press    to calculate");
    write_string(HINT_COLOR, StX + 36, StY + 15, "F7");
    write_string(TEXT_COLOR, StX, StY + 16, "     Current partition size:  ?");
    write_string(TEXT_COLOR, StX, StY + 17, "     Maximum partition size:  ?");
    
    sprintf(tmp, "%-.8s", b->sys_id);
    write_string(DATA_COLOR, StX2, StY + 1, tmp);

    sprintf(tmp, "%-.8s", b->fs_id);
    write_string(DATA_COLOR, StX2, StY + 2, tmp);

    sprintf(tmp, "%Xh", b->media_desc);
    write_string(DATA_COLOR, StX3, StY + 2, tmp);

    sprintf(tmp,
            "%-3u %s bytes/cluster",
            b->clust_size,
            sprintf_long(tmp1, (unsigned long)b->clust_size * SECT_SIZE));
    write_string(DATA_COLOR, StX2, StY + 3, tmp);

    sprintf(tmp, "%d", b->sect_size);
    write_string(DATA_COLOR, StX3, StY + 3, tmp);

    
    sprintf(tmp,
            "%-3d",
            b->fat_size);
    write_string(DATA_COLOR, StX2, StY + 4, tmp);
    
    sprintf(tmp, "%d", b->num_fats);
    write_string(DATA_COLOR, StX3, StY + 4, tmp);

    sprintf(tmp, "%d", b->root_entr);
    write_string(DATA_COLOR, StX2, StY + 5, tmp);

    sprintf(tmp, "%Xh", b->ext_signat);
    write_string(DATA_COLOR, StX3, StY + 5, tmp);

    sprintf(tmp, "%08lX", b->serial_num);
    write_string(DATA_COLOR, StX2, StY + 6, tmp);

    sprintf(tmp, " %u", dinfo.disk);
    write_string(DATA_COLOR, StX2 + 12, StY + 8, tmp);

    sprintf(tmp, " %u", dinfo.num_heads);
    write_string(DATA_COLOR, StX2 + 12, StY + 9, tmp);

    sprintf(tmp, " %u", dinfo.num_sects);
    write_string(DATA_COLOR, StX2 + 12, StY + 10, tmp);

    sprintf(tmp, " %-9lu", fat_calc_hidden_sect(p));
    write_string(DATA_COLOR, StX2 + 12, StY + 11, tmp);

    sprintf(tmp, " %-5u", (p->num_sect > 65535L) ? 0 : p->num_sect);
    write_string(DATA_COLOR, StX2 + 12, StY + 12, tmp);

    sprintf(tmp, " %-9lu", (p->num_sect > 65535L) ? p->num_sect : 0);
    write_string(DATA_COLOR, StX2 + 12, StY + 13, tmp);

    num_sect = b->total_sect != 0 ? b->total_sect : b->big_total;

    n = b->res_sects + b->num_fats * b->fat_size +
        b->root_entr * 32 / SECT_SIZE;
    max_num_sect = n + max_clust * b->clust_size;

    sprintf(tmp,
            "%lu sectors = %s kbytes",
            max_num_sect,
            sprintf_long(tmp1, (max_num_sect) / 2));
    write_string(DATA_COLOR, StX2, StY + 17, tmp);

    n = num_sect;
    sprintf(tmp, "%lu sectors = %s kbytes", n, sprintf_long(tmp1, n / 2));
    write_string(DATA_COLOR, StX2, StY + 16, tmp);

    pos = 0;
    act = 0;

    while (1) {
        for (i = 0; i < 11; i++)
            tmp[i] = (b->label[i] == 0) ? ' ' : b->label[i];
        tmp[i] = 0;
        write_string(DATA_COLOR, StX2, StY + 7, tmp);
        sprintf(tmp, "%-5d", b->drive_num);
        write_string((b->drive_num == dinfo.disk) ? DATA_COLOR : INVAL_COLOR,
                     StX2,
                     StY + 8,
                     tmp);
        sprintf(tmp, "%-5d", b->num_sides);
        write_string((b->num_sides == dinfo.num_heads) ? DATA_COLOR
                                                       : INVAL_COLOR,
                     StX2,
                     StY + 9,
                     tmp);
        sprintf(tmp, "%-5d", b->num_sects);
        write_string((b->num_sects == dinfo.num_sects) ? DATA_COLOR
                                                       : INVAL_COLOR,
                     StX2,
                     StY + 10,
                     tmp);
        sprintf(tmp, "%-9lu", b->hid_sects);
        write_string((b->hid_sects == fat_calc_hidden_sect(p)) ? DATA_COLOR : INVAL_COLOR,
                     StX2,
                     StY + 11,
                     tmp);
        sprintf(tmp, "%-6u", b->total_sect);
        write_string((b->total_sect == 0 || b->total_sect == p->num_sect)
                         ? DATA_COLOR
                         : INVAL_COLOR,
                     StX2,
                     StY + 12,
                     tmp);
        sprintf(tmp, "%-9lu", b->big_total);
        write_string((b->big_total == 0 || b->big_total == p->num_sect)
                         ? DATA_COLOR
                         : INVAL_COLOR,
                     StX2,
                     StY + 13,
                     tmp);
        sprintf(tmp, "%-11lu  max %-11lu", 
            (num_sect - b->res_sects - b->num_fats * b->fat_size)
             / b->clust_size, max_clust);

        write_string(DATA_COLOR, StX2, StY + 14, tmp);
        
        sprintf(tmp, "%04X", b->magic_num);
        write_string((b->magic_num == MBR_MAGIC_NUM) ? DATA_COLOR
                                                     : INVAL_COLOR,
                     StX3,
                     StY + 6,
                     tmp);

        p->changed = (memcmp(b, b_orig, SECT_SIZE) == 0) ? 0 : 1;
        if (p->changed == 0)
            write_string(HINT_COLOR, 15, 24, "F2");
        if (p->changed == 1)
            write_string(Blink + HINT_COLOR, 15, 24, "F2");

        if (act == 0) {
            memmove(tmp, b->label, 11);
            tmp[11] = 0;
            for (i = 10; i >= 0 && tmp[i] == ' '; i--)
                tmp[i] = 0;
            memmove(tmp1, tmp, 12);
            edit_str_field(&ev, 0, EDIT_COLOR, StX2, StY + 7, 12, tmp, &pos);
            if (memcmp(tmp, tmp1, 12) != 0) {
                for (i = 0; tmp[i] != 0 && i < 11; i++)
                    b->label[i] = toupper(tmp[i]);
                for (; i < 11; i++)
                    b->label[i] = ' ';
                if (!memcmp(b->label, "           ", 11)) {
                    memcpy(b->label, NO_NAME_LABEL, 11);
                }
            }
        }

        if (act == 1) {
            n = b->drive_num;
            edit_int_field(&ev, 0, EDIT_COLOR, StX2, StY + 8, -5, &n, 255L);
            b->drive_num = n;
        }

        if (act == 2) {
            n = b->num_sides;
            edit_int_field(&ev, 0, EDIT_COLOR, StX2, StY + 9, -5, &n, 255L);
            b->num_sides = n;
        }

        if (act == 3) {
            n = b->num_sects;
            edit_int_field(&ev, 0, EDIT_COLOR, StX2, StY + 10, -5, &n, 63L);
            b->num_sects = n;
        }

        if (act == 4)
            edit_int_field(&ev,
                           0,
                           EDIT_COLOR,
                           StX2,
                           StY + 11,
                           -9,
                           &b->hid_sects,
                           999999999L);
        if (act == 5) {
            n = b->total_sect;
            edit_int_field(&ev, 0, EDIT_COLOR, StX2, StY + 12, -6, &n, 65535L);
            b->total_sect = n;
        }

        if (act == 6)
            edit_int_field(&ev,
                           0,
                           EDIT_COLOR,
                           StX2,
                           StY + 13,
                           -9,
                           &b->big_total,
                           999999999L);
        /*   get_event(&ev,EV_KEY); */

        /*
        n = b->total_sect != 0 ? b->total_sect : b->big_total;
        sprintf(tmp, "%ld sectors = %s kbytes", n, sprintf_long(tmp1, n / 2));
        clear_window(DATA_COLOR, StX2, StY + 16, 78 - StX2, 1);
        write_string((n >= min_num_sect && n <= max_num_sect) ? DATA_COLOR
                                                              : INVAL_COLOR,
                     StX2,
                     StY + 16,
                     tmp);
        */
        p->changed = (memcmp(b, b_orig, SECT_SIZE) == 0) ? 0 : 1;
        if (p->changed == 0)
            write_string(HINT_COLOR, 15, 24, "F2");
        if (p->changed == 1)
            write_string(Blink + HINT_COLOR, 15, 24, "F2");

        if (ev.key == 27) /* ESC */
        {
            if (p->changed)
                continue;
            break;
        }
        if (ev.key == 13 || ev.key == 9) /* Enter or Tab */
        {
            act = (act + 1) % 7;
        } else if (ev.scan == 0x3B00) /* F1 - Help */
        {
            sprintf(tmp, "#setup_fat");
            html_view(tmp);
        } else if (ev.scan == 0x50E0 || ev.scan == 0x5000) /* down */
        {
            if (act < 6)
                act++;
        } else if (ev.scan == 0x48E0 || ev.scan == 0x4800) /* up */
        {
            if (act > 0)
                act--;
        } else if (ev.scan == 0x3C00) /* F2 - Save */
        {
            if (p->inh_invalid || p->inh_changed) {
                show_error(ERROR_INH_INVAL);
                continue;
            }

            flush_caches();

            disk_lock(dinfo.disk);

            if (disk_write_rel(p, 0, b, 1) == FAILED ||
                fat16_update_label_file(p, b) == FAILED) {
                show_error("Error saving boot sector or volume label");
            } else {
                memmove(b_orig, b, SECT_SIZE);
            }

            disk_unlock(dinfo.disk);
        } else if (ev.scan == 0x3D00) /* F3 - Undo */
        {
            memmove(b, b_orig, SECT_SIZE);
        } else if (ev.scan == 0x3F00) /* F5 - Set */
        {
            b->drive_num  = dinfo.disk;
            b->num_sides  = dinfo.num_heads;
            b->num_sects  = dinfo.num_sects;
            b->hid_sects  = fat_calc_hidden_sect(p);
            b->total_sect = (p->num_sect < 65536L) ? p->num_sect : 0;
            b->big_total  = (p->num_sect < 65536L) ? 0 : p->num_sect;
            b->magic_num  = MBR_MAGIC_NUM;
        } else if (ev.scan == 0x4000) /* F6 - Code */
        {
            memmove(b->xcode, fat_boot_code->xcode, sizeof(b->xcode));
        } else if (ev.scan == 0x4100) /* F7 - calculate min size */
        {
            write_string(HINT_COLOR, StX2, StY + 15, "Reading FAT...                 ");
            move_cursor(StX2 + 14, StY + 15);
        
            fatsz = min(256, b->fat_size);
            l = lc = 0;
            for (n = b->res_sects; n < b->res_sects + fatsz; n++) {
                get_event(&ev, EV_KEY | EV_NONBLOCK);
                if (ev.key == 27) {
                    write_string(TEXT_COLOR, StX, StY + 15, 
                        "     Minimum partition size:  press    to calculate");
                     write_string(HINT_COLOR, StX + 36, StY + 15, "F7");
                    goto calc_aborted;
                }

                if (disk_read_rel(p, n, tmp, 1) == -1) {
                    show_error("Error reading FAT table");
                    break;
                } else {
                    write_int(HINT_COLOR, StX2 + 16, StY + 15, 5, n - b->res_sects);

                    for (i = 0; i < SECT_SIZE; i++, l++)
                        if (tmp[i] != 0)
                            lc = l;
                }
            }
            min_clust =
                (syst == 0u) ? (0u) : (lc * 8u / syst + (lc * 8u % syst == 0u ? 0u : 1u) - 2u);
        
        
            min_num_sect = b->res_sects + 
                (unsigned long) b->num_fats * b->fat_size + 
                (unsigned long) min_clust * b->clust_size;
            
            sprintf(tmp,
                    "%lu sectors = %s kbytes",
                    min_num_sect,
                    sprintf_long(tmp1, (min_num_sect) / 2));
            write_string(DATA_COLOR, StX2, StY + 15, tmp);

        calc_aborted: (void)0;
        }

    } /* while(1) */

    free(tmp);
    return OK;
} /* setup_fat */
