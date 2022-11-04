#include "part.h"
#include <time.h>

_Packed struct boot_fat32
{
    /* Sector 1 */

    unsigned char jmp[3];    /* Must be 0xEB, 0x58, 0x90 = jmp 5A	*/
    unsigned char sys_id[8]; /* Probably:   "MSWIN4.1"		*/

    unsigned short sect_size; /* Sector size in bytes (512)		*/
    unsigned char clust_size; /* Sectors per cluster (1,2,4,...,128)	*/
    unsigned short res_sects; /* Reserved sectors at the beginning (33)*/
    unsigned char fat_copies; /* Number of FAT copies (2)		*/

    unsigned char resrvd1[4]; /* Reserved				*/
    unsigned char media_desc; /* Media descriptor byte (F8h)		*/
    unsigned short sfat_size; /* Sectors per FAT			*/

    unsigned short track_size; /* Sectors per track			*/
    unsigned short num_sides;  /* Sides				*/

    unsigned long hid_sects; /* Special hidden sectors		*/
    unsigned long num_sects; /* Big total number of sectors  	*/
    unsigned long fat_size;  /* Sectors per FAT (big)		*/
    unsigned char fat_attr;  /* FAT attributes (I guess)		*/

    unsigned char fs_ver_maj;  /* File System Version (major)		*/
    unsigned short fs_ver_min; /* File System Version (minor)		*/

    unsigned long root_clust; /* First cluster in root		*/

    unsigned short fs_sect_num; /* FS Sector number (1) ???		*/
    unsigned short bs_bak_sect; /* Boot sector backup (6)		*/

    unsigned char resrvd2[12]; /* Reserved				*/
    unsigned char drive_num;   /* Physical drive number (80h)		*/
    unsigned char resrvd3[1];  /* Reserved				*/

    unsigned char ext_signat; /* Extended Boot Record signature (29h)	*/
    unsigned long serial_num; /* Volume serial number			*/
    unsigned char label[11];  /* Volume label				*/
    unsigned char fs_id[8];   /* File system id ("FAT32   ")		*/
    unsigned char xcode[418]; /* Boot loader code (first part)	*/
    unsigned long magic_num;  /* Magic number (Must be 0xAA550000) 	*/

    /* Sector 2 */

    unsigned long ext_sign2;    /* Ext Boot Record Sign (0x41615252)	*/
    unsigned char resrvd4[480]; /* Reserved				*/
    unsigned long ext_sign3;    /* FS Info Signature    (0x61417272)	*/
    unsigned long free_clust;   /* Number of free clusters		*/
    unsigned long next_free;    /* Next free cluster			*/
    unsigned char resrvd5[12];  /* Reserved				*/
    unsigned long magic_num2;   /* Ext Boot Record Sign (0xAA550000)	*/

    /* Sector 3 */

    unsigned char resrvd6[508]; /* Reserved				*/
    unsigned long magic_num3;   /* Ext Boot Record Sign (0xAA550000)	*/
};

#define BBT_SIZE 128

#define F_NORM  0
#define F_QUICK 1
#define F_DESTR 2

#define MBR_MAGIC_NUM32 0xAA550000L

int format_fat32(struct part_long *p, char **argv)
{
    char *data_pool;
    struct boot_fat32 *b;
    unsigned long *fat;
    int i, j, k, wr_sect, ret_code, fat_size, next_bad;
    unsigned long l, num_clust, num_sect, base_sect, *bbt;

    unsigned int num_bad    = 0;
    unsigned int clust_size = 8;
    unsigned int form_type  = F_NORM;

    if ((data_pool = malloc(SECT_SIZE * 3 + BBT_SIZE * sizeof(long))) == 0) {
        show_error(ERROR_MALLOC);
        return FAILED;
    }

    b   = (struct boot_fat32 *)(data_pool);
    fat = (unsigned long int *)(data_pool);
    bbt = (unsigned long int *)(data_pool + SECT_SIZE * 3);

    memset(b, 0, SECT_SIZE * 3);

    memmove(b->jmp, "\xEB\x58\x90", 3);
    memmove(b->sys_id, "MSWIN4.1", 8);
    b->sect_size   = SECT_SIZE;
    b->res_sects   = 33;
    b->fat_copies  = 2;
    b->fs_sect_num = 1;
    b->bs_bak_sect = 6;
    b->media_desc  = 0xF8;
    b->ext_signat  = 0x29;
    memmove(b->label, "NO NAME    ", 11);
    memmove(b->fs_id, "FAT32   ", 8);
    memmove(b->xcode, EMP_IPL, EMP_SIZE);
    strncpy(b->xcode + EMP_SIZE, MESG_NON_SYSTEM, sizeof(b->xcode) - EMP_SIZE);

    b->ext_sign2 = 0x41615252L;
    b->ext_sign3 = 0x61417272L;

    b->magic_num  = 0xAA550000L;
    b->magic_num2 = 0xAA550000L;
    b->magic_num3 = 0xAA550000L;

    num_sect = p->num_sect;

    while (*argv != 0) {
        if (_stricmp(*argv, "/destructive") == 0)
            form_type = F_DESTR;
        else if (_stricmp(*argv, "/quick") == 0)
            form_type = F_QUICK;
        else if (_strnicmp(*argv, "/c:", 3) == 0) {
            k = atoi((*argv) + 3);
            for (i = k / 2, j = 0; i != 0 && j < 7; j++, i /= 2)
                ;
            clust_size = 1 << j;
            if (clust_size != k) {
                progress("^Invalid cluster size.");
                goto failed;
            }
        } else if (_strnicmp(*argv, "/x:", 3) == 0) {
            if (_stricmp(*argv, "/x:disk") == 0)
                l = dinfo.total_sects;
            else
                l = atol((*argv) + 3);
            if (l > num_sect)
                num_sect = l;
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

    while (1) {
        fat_size  = (num_sect) / (clust_size * 128) + 1;
        num_clust = (num_sect - 33 - 2 * fat_size) / (clust_size);

        if (num_clust >= 65536L)
            break;

        clust_size /= 2;

        if (clust_size == 0) {
            progress("^Oops! Cluster size is 0...");
            progress("You have to use FAT-16 file system for such a small "
                     "partition.");
            goto failed;
        }
    }

    if (33 + 2 * fat_size + 2 * clust_size > p->num_sect) {
        progress("^Partition is too small.");
        goto failed;
    }

    b->fat_size   = fat_size;
    b->clust_size = clust_size;

    b->root_clust = 2;
    b->next_free  = 3;
    b->free_clust = num_clust - 1;

    b->drive_num  = dinfo.disk;
    b->track_size = dinfo.num_sects;
    b->num_sides  = dinfo.num_heads;

    b->hid_sects = p->rel_sect;
    b->num_sects = p->num_sect;

    b->serial_num =
        ((p->rel_sect << 16) + (p->num_sect * ((long)b % 451))) +
        ((dinfo.total_sects % 12345L) ^ (dinfo.total_sects * 67891L)) +
        ((dinfo.disk * 123L) ^ (dinfo.num_heads % 7)) + clock();

    flush_caches();

    if (form_type == F_QUICK)
        ret_code = 0;
    else if (form_type == F_DESTR)
        ret_code = generic_format(p, BBT_SIZE, bbt);
    else
        ret_code = generic_verify(p, BBT_SIZE, bbt);

    if (ret_code < 0) /* format failed or canceled */
    {
        free(data_pool);
        return ret_code;
    }

    num_bad = ret_code;

    disk_lock(dinfo.disk);

    progress("^Initializing file system ...");

    if (num_bad != 0 && bbt[0] < 33 + 2 * fat_size + 2 * clust_size) {
        progress(
            "Beginning of the partition is unusable. Try to move it forward.");
        goto failed;
    }

    progress("~Writing boot sector ...");

    if (disk_write_rel(p, 0, b, 3) == -1) /*  Writing boot sector  */
    {
        progress("Error writing boot sector.");
        goto failed;
    }

    if (disk_write_rel(p, 6, b, 3) == -1) /*  Writing boot sector (backup) */
    {
        progress("Error writing boot sector.");
        goto failed;
    }

    progress("~Writing FAT tables ...");

    wr_sect = 33;

    for (k = 0; k < 2; k++) /* Writing two copies of FAT16 */
    {
        next_bad  = 0;
        base_sect = 33 + 2 * fat_size + clust_size /*root*/;

        for (i = 0; i < fat_size; i++) {
            memset(fat, 0, 512);
            if (i == 0) {
                fat[0] = 0x0FFFFFF8;
                fat[1] = 0x0FFFFFFF;
                fat[2] = 0x0FFFFFFF; /* root */
                while (next_bad != num_bad &&
                       bbt[next_bad] < base_sect + clust_size * 125)
                    fat[(bbt[next_bad++] - base_sect) / clust_size + 3] =
                        0x0FFFFFF7;
                base_sect += clust_size * 125;
            } else {
                while (next_bad != num_bad &&
                       bbt[next_bad] < base_sect + clust_size * 128)
                    fat[(bbt[next_bad++] - base_sect) / clust_size] =
                        0x0FFFFFF7;
                base_sect += clust_size * 128;
            }

            if (disk_write_rel(p, wr_sect++, fat, 1) == -1) {
                progress("Error writing FAT.");
                goto failed;
            }
        }
    }

    memset(fat, 0, 512);

    progress("~Writing root directory ...");

    for (i = 0; i < clust_size; i++)
        if (disk_write_rel(p, wr_sect++, fat, 1) == -1) {
            progress("Error writing root directory.");
            goto failed;
        }

    disk_unlock(dinfo.disk);
    free(data_pool);
    return OK;

failed:

    disk_unlock(dinfo.disk);
    free(data_pool);
    return FAILED;
} /* format_fat32 */

int print_fat32(struct part_long *p)
{
    char tmp1[20], tmp2[20];
    struct boot_fat32 *b = (struct boot_fat32 *)tmp;

    if (disk_read_rel(p, 0, b, 3) == -1) {
        fprintf(stderr, "Error reading boot sector.\n");
        return FAILED;
    }

    printf("\n                  Boot Sector Data              Expected "
           "Value\n\n");
    printf("                        System id:  %-.8s\n", b->sys_id);
    printf("                      Sector size:  %-3d         512\n",
           b->sect_size);
    printf("              Sectors per cluster:  %d\n", b->clust_size);
    printf("Reserved sectors at the beginning:  %d\n", b->res_sects);
    printf("             Number of FAT copies:  %d\n", b->fat_copies);
    printf("            Media descriptor byte:  %02Xh         F8h\n",
           b->media_desc);

    printf("                     Drive number:  %-3d         %d\n",
           b->drive_num,
           dinfo.disk);
    printf("                  Number of sides:  %-3d         %d\n",
           b->num_sides,
           dinfo.num_heads);
    printf("                 Sectors per side:  %-2d          %d\n",
           b->track_size,
           dinfo.num_sects);

    printf("Hidden sectors prior to partition:  %-10s  %-10s\n",
           sprintf_long(tmp1, b->hid_sects),
           sprintf_long(tmp2, p->rel_sect));
    printf("          Total number of sectors:  %-10s  %-10s\n",
           sprintf_long(tmp1, b->num_sects),
           sprintf_long(tmp2, p->num_sect));
    printf("                  Sectors per FAT:  %u\n", b->sfat_size);
    printf("              Big Sectors per FAT:  %lu\n", b->fat_size);
    printf("               FAT Attributes (?):  %02Xh\n", b->fat_attr);
    printf("            First cluster in root:  %lu\n", b->root_clust);
    printf("                    Free clusters:  %lu\n", b->free_clust);
    printf("                        Next free:  %lu\n", b->next_free);

    printf("   Extended boot record signature:  %02Xh\n", b->ext_signat);
    printf("             Volume serial number:  %08lX\n", b->serial_num);
    printf("                     Volume label:  %-.11s\n", b->label);
    printf("                 File system type:  %-.8s    FAT32\n", b->fs_id);
    printf("         Boot sector magic number:  0x%08lX  0xAA550000\n",
           b->magic_num);

#if 0
     unsigned char  fs_ver_maj;	/* File System Version (major)		*/
     unsigned short fs_ver_min	/* File System Version (minor)		*/

     unsigned short fs_sect_num; /* FS Sector number (1) ???		*/
     unsigned short bs_bak_sect; /* Boot sector backup (6)		*/

     unsigned long  ext_sign2;	 /* Ext Boot Record Sign (0x41615252)	*/
     unsigned long  ext_sign3;	 /* FS Info Signature    (0x61417272)	*/

#endif

    printf("  Second Boot sector magic number:  0x%08lX  0xAA550000\n",
           b->magic_num2);
    printf("   Third Boot sector magic number:  0x%08lX  0xAA550000\n",
           b->magic_num3);

    return 0;
} /* print_fat32 */

#define StX 5
#define StY 5
#define StW 66
#define StH 14

#define StX2 (StX + 30)
#define StX3 (StX + 69)

#define TEXT_COLOR (BrCyan + BakBlue)
#define DATA_COLOR (BrWhite + BakBlue)
#define EDIT_COLOR (BrWhite + BakBlack)

int setup_fat32(struct part_long *p)
{
    struct event ev;
    int i, act, pos;
    char *tmp, *tmp1;
    struct boot_fat32 *b, *b_orig;
    unsigned long n, l, lc, max_clust, min_clust, min_num_sect, max_num_sect,
        fatsz;

    if ((tmp = malloc(7 * SECT_SIZE)) == 0) {
        show_error(ERROR_MALLOC);
        return FAILED;
    }

    tmp1 = tmp + 100;
    /* tmp2=tmp+120; */
    b      = (struct boot_fat32 *)(tmp + SECT_SIZE);
    b_orig = b + 1;

    if (disk_read_rel(p, 0, b, 3) == -1) {
        show_error("Error reading boot sector");
        free(tmp);
        return FAILED;
    }

    memmove(b_orig, b, 3 * SECT_SIZE);

    clear_window(TEXT_COLOR, 2, 5, 78, 19);
    write_string(HINT_COLOR, 56, 24, "F5");
    write_string(MENU_COLOR, 58, 24, " - SetExp");

    max_clust = ((long)SECT_SIZE * 8 * b->fat_size / 32 - 2);

    write_string(TEXT_COLOR,
                 StX,
                 StY + 1,
                 "                  System id:                             FS "
                 "Version:");
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
                 "      First cluster in Root:                               "
                 "Ext sign:");
    write_string(
        TEXT_COLOR,
        StX,
        StY + 6,
        "       Volume serial number:                             Magic:");
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
                 "    Total number of sectors:               123456789");
    write_string(TEXT_COLOR, StX, StY + 15, "     Minimum partition size:");
    write_string(TEXT_COLOR, StX, StY + 16, "     Current partition size:");
    write_string(TEXT_COLOR, StX, StY + 17, "     Maximum partition size:");

    sprintf(tmp, "%-.8s", b->sys_id);
    write_string(DATA_COLOR, StX2, StY + 1, tmp);

    sprintf(tmp, "%d.%02d", b->fs_ver_maj, b->fs_ver_min);
    write_string(DATA_COLOR, StX3, StY + 1, tmp);

    if (b->fs_ver_maj > 0 || b->fs_ver_maj == 0 && b->fs_ver_min > 0) {
        show_error("Warning: I wasn't tested with this FileSystem Version!!!");
    }

    sprintf(tmp, "%-.8s", b->fs_id);
    write_string(DATA_COLOR, StX2, StY + 2, tmp);

    sprintf(tmp, "%Xh", b->media_desc);
    write_string(DATA_COLOR, StX3, StY + 2, tmp);

    sprintf(tmp,
            "%-5u %s bytes/clust",
            b->clust_size,
            sprintf_long(tmp1, (unsigned long)b->clust_size * SECT_SIZE));
    write_string(DATA_COLOR, StX2, StY + 3, tmp);

    sprintf(tmp, "%d", b->sect_size);
    write_string(DATA_COLOR, StX3, StY + 3, tmp);

    sprintf(
        tmp, "%-5lu %s max clust", b->fat_size, sprintf_long(tmp1, max_clust));
    write_string(DATA_COLOR, StX2, StY + 4, tmp);

    sprintf(tmp, "%d", b->fat_copies);
    write_string(DATA_COLOR, StX3, StY + 4, tmp);

    sprintf(tmp, "%lu", b->root_clust);
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

    sprintf(tmp, " %-9lu", p->rel_sect);
    write_string(DATA_COLOR, StX2 + 12, StY + 11, tmp);

    sprintf(tmp, " %-9lu", p->num_sect);
    write_string(DATA_COLOR, StX2 + 12, StY + 12, tmp);

    write_string(HINT_COLOR, StX2, StY + 16, "Reading FAT... ");
    move_cursor(StX2 + 21, StY + 16);

    fatsz = min(65536L, b->fat_size);
    l = lc = 0;
    for (n = b->res_sects; n < b->res_sects + fatsz; n++)
        if (disk_read_rel(p, n, tmp, 1) == -1) {
            show_error("Error reading FAT table");
            break;
        } else {
            write_int(HINT_COLOR, StX2 + 16, StY + 16, 5, n - b->res_sects);
            for (i = 0; i < SECT_SIZE; i++, l++)
                if (tmp[i] != 0)
                    lc = l;
        }

    min_clust = (lc * 8 / 32 + (lc * 8 % 32 == 0 ? 0 : 1) - 2);

    n = b->res_sects + b->fat_copies * b->fat_size;

    min_num_sect = n + min_clust * b->clust_size;
    max_num_sect = n + max_clust * b->clust_size;

    sprintf(tmp,
            "%ld sectors = %s kbytes",
            min_num_sect,
            sprintf_long(tmp1, (min_num_sect) / 2));
    write_string(DATA_COLOR, StX2, StY + 15, tmp);

    sprintf(tmp,
            "%ld sectors = %s kbytes",
            max_num_sect,
            sprintf_long(tmp1, (max_num_sect) / 2));
    write_string(DATA_COLOR, StX2, StY + 17, tmp);

    n = b->num_sects;
    sprintf(tmp, "%ld sectors = %s kbytes", n, sprintf_long(tmp1, n / 2));
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
        sprintf(tmp, "%-5d", b->track_size);
        write_string((b->track_size == dinfo.num_sects) ? DATA_COLOR
                                                        : INVAL_COLOR,
                     StX2,
                     StY + 10,
                     tmp);
        sprintf(tmp, "%-9lu", b->hid_sects);
        write_string((b->hid_sects == p->rel_sect) ? DATA_COLOR : INVAL_COLOR,
                     StX2,
                     StY + 11,
                     tmp);
        sprintf(tmp, "%-9lu", b->num_sects);
        write_string((b->num_sects == p->num_sect) ? DATA_COLOR : INVAL_COLOR,
                     StX2,
                     StY + 12,
                     tmp);
        sprintf(tmp, "%08lX", b->magic_num);
        write_string((b->magic_num == MBR_MAGIC_NUM32) ? DATA_COLOR
                                                       : INVAL_COLOR,
                     StX3 - 5,
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
                    b->label[i] = tmp[i];
                for (; i < 11; i++)
                    b->label[i] = ' ';
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
            n = b->track_size;
            edit_int_field(&ev, 0, EDIT_COLOR, StX2, StY + 10, -5, &n, 63L);
            b->track_size = n;
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
        if (act == 5)
            edit_int_field(&ev,
                           0,
                           EDIT_COLOR,
                           StX2,
                           StY + 12,
                           -9,
                           &b->num_sects,
                           999999999L);
        /*   get_event(&ev,EV_KEY); */

        n = b->num_sects;
        sprintf(tmp, "%ld sectors = %s kbytes", n, sprintf_long(tmp1, n / 2));
        clear_window(DATA_COLOR, StX2, StY + 16, 78 - StX2, 1);
        write_string((n >= min_num_sect && n <= max_num_sect) ? DATA_COLOR
                                                              : INVAL_COLOR,
                     StX2,
                     StY + 16,
                     tmp);

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
            act = (act + 1) % 6;
        } else if (ev.scan == 0x3B00) /* F1 - Help */
        {
            sprintf(tmp, "#setup_fat");
            html_view(tmp);
        } else if (ev.scan == 0x50E0 || ev.scan == 0x5000) /* down */
        {
            if (act < 5)
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

            if (disk_write_rel(p, 0, b, 3) == -1 ||
                disk_write_rel(p, 6, b, 3) == -1) {
                show_error("Error saving boot sector");
            } else {
                memmove(b_orig, b, 3 * SECT_SIZE);
            }

            disk_unlock(dinfo.disk);
        } else if (ev.scan == 0x3D00) /* F3 - Undo */
        {
            memmove(b, b_orig, 3 * SECT_SIZE);
        } else if (ev.scan == 0x3F00) /* F5 - Set */
        {
            b->drive_num = dinfo.disk;
            b->num_sides = dinfo.num_heads;
            b->num_sects = dinfo.num_sects;
            b->hid_sects = p->rel_sect;
            b->num_sects = p->num_sect;
            b->magic_num = MBR_MAGIC_NUM32;
        }

    } /* while(1) */

    free(tmp);
    return OK;
} /* setup_fat32 */
