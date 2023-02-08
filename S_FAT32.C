#include "part.h"
#include "fat.h"
#include <time.h>
#include <ctype.h>


#define BBT_SIZE 128

#define F_NORM  0
#define F_VERIFY 1
#define F_DESTR 2

#define MBR_MAGIC_NUM32 0xAA55

/* according to Microsoft FAT specification */
unsigned fat32_cluster_size(unsigned long sectors)
{
    if (sectors <= 66600) return 0;
    else if (sectors <= 532480) return 1;
    else if (sectors <= 16777216) return 8;
    else if (sectors <= 33554432) return 16;
    else if (sectors <= 67108864) return 32;
    else return 64;
}

unsigned long fat32_clust_to_sect(struct boot_fat32 *b, unsigned long clust)
{
    return b->res_sects + b->fat_copies * b->fat_size + (clust-2) * b->clust_size;
}

int fat32_read_clust(unsigned char * buf,
    struct part_long *p, 
    struct boot_fat32 *b,
    unsigned long clust)
{
    int i;
    unsigned long sect = fat32_clust_to_sect(b, clust);
    if (clust < 2) return FAILED;

    for (i = 0; i < b->clust_size; i++) {
        if (disk_read_rel(p, sect+i, buf, 1) == FAILED) return FAILED;
        buf += SECT_SIZE;
    }
    return OK;
}

int fat32_write_clust(unsigned char * buf,
    struct part_long *p, 
    struct boot_fat32 *b,
    unsigned long clust)
{
    int i;
    unsigned long sect = fat32_clust_to_sect(b, clust);
    if (clust < 2) return FAILED;

    for (i = 0; i < b->clust_size; i++) {
        if (disk_write_rel(p, sect+i, buf, 1) == FAILED) return FAILED;
        buf += SECT_SIZE;
    }
    return OK;
}


int fat32_update_label_file(struct part_long *p, struct boot_fat32 *b)
{
    unsigned char *buf;
    struct dirent *dirent;
    unsigned long cluster, start_sect, sect;
    int j;

    buf = malloc(SECT_SIZE);
    if (buf == NULL) return FAILED;

    cluster = b->root_clust;
    start_sect = fat32_clust_to_sect(b, cluster);

    /* for now we only search the first cluster for a volume id */
    for (sect = start_sect; sect < start_sect + b->clust_size; sect++)
    {
        if (disk_read_rel(p, sect, buf, 1) == FAILED) goto failed;
        dirent = (struct dirent *)buf;

        for (j = 0; j < DIRENT_PER_SECT; j++) {
            if (dirent->name[0] == 0 || 
                    (dirent->attr & DIRENT_LONG_NAME_MASK) != DIRENT_LONG_NAME_MASK &&
                    ((dirent->attr) & (DIRENT_ATTR_VOL | DIRENT_ATTR_DIR))  == DIRENT_ATTR_VOL) {

                if (memcmp(b->label, NO_NAME_LABEL, sizeof(b->label))) {
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
           sprintf_long(tmp2, fat_calc_hidden_sectors(p)));
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

     unsigned short fs_info_sect_num; /* FS Sector number (1) ???		*/
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
    unsigned long n, l, lc;
    unsigned long min_clust, max_clust, min_num_sect, max_num_sect;

    if ((tmp = malloc(7 * SECT_SIZE)) == 0) {
        show_error(ERROR_MALLOC);
        return FAILED;
    }

    tmp1 = tmp + 100;
    b      = (struct boot_fat32 *)(tmp + SECT_SIZE);
    b_orig = b + 1;

    if (disk_read_rel(p, 0, b, 3) == -1) {
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

    memmove(b_orig, b, 3 * SECT_SIZE);

    clear_window(TEXT_COLOR, 2, 5, 78, 19);
    write_string(HINT_COLOR, 56, 24, "F5");
    write_string(MENU_COLOR, 58, 24, " - SetExp");

    min_clust = 0;
    max_clust = ((unsigned long)SECT_SIZE / 4 * b->fat_size - 2);
    min_num_sect = 0;
    n = b->res_sects + b->fat_copies * b->fat_size;
    max_num_sect = n + max_clust * b->clust_size;

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
    write_string(TEXT_COLOR,
                 StX,
                 StY + 13,
                 "         Number of clusters:");

    
    write_string(TEXT_COLOR, StX, StY + 15, "     Minimum partition size:  press    to calculate");
    write_string(HINT_COLOR, StX + 36, StY + 15, "F7");
    write_string(TEXT_COLOR, StX, StY + 16, "     Current partition size:  ?");
    write_string(TEXT_COLOR, StX, StY + 17, "     Maximum partition size:  ?");
    
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
        tmp, "%-7lu", b->fat_size);

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

    sprintf(tmp, " %-9lu", fat_calc_hidden_sectors(p));
    write_string(DATA_COLOR, StX2 + 12, StY + 11, tmp);

    sprintf(tmp, " %-9lu", p->num_sect);
    write_string(DATA_COLOR, StX2 + 12, StY + 12, tmp);

    sprintf(tmp,
            "%lu sectors = %s KiB",
            max_num_sect,
            sprintf_long(tmp1, (max_num_sect) / 2));
    write_string(DATA_COLOR, StX2, StY + 17, tmp);

    n = b->num_sects;
    sprintf(tmp, "%lu sectors = %s KiB", n, sprintf_long(tmp1, n / 2));
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
        sprintf(tmp, "%-11lu", b->hid_sects);
        write_string((b->hid_sects == fat_calc_hidden_sectors(p)) ? DATA_COLOR : INVAL_COLOR,
                     StX2,
                     StY + 11,
                     tmp);
        sprintf(tmp, "%-11lu", b->num_sects);
        write_string((b->num_sects == p->num_sect) ? DATA_COLOR : INVAL_COLOR,
                     StX2,
                     StY + 12,
                     tmp);

        sprintf(tmp, "%-11lu  max %-11lu", 
            (b->num_sects - b->res_sects - b->fat_copies * b->fat_size)
             / b->clust_size, max_clust);
        write_string(DATA_COLOR, StX2, StY + 13, tmp);

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
                           -11,
                           &b->hid_sects,
                           -1);
        if (act == 5)
            edit_int_field(&ev,
                           0,
                           EDIT_COLOR,
                           StX2,
                           StY + 12,
                           -11,
                           &b->num_sects,
                           -1);
        /*   get_event(&ev,EV_KEY); */

        
        n = b->num_sects;
        
        sprintf(tmp, "%lu sectors = %s KiB", n, sprintf_long(tmp1, n / 2));
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

            if (disk_write_rel(p, 0, b, 3) == FAILED ||
                disk_write_rel(p, 6, b, 3) == FAILED ||
                fat32_update_label_file(p, b) == FAILED) {
                show_error("Error saving boot sector or volume label.");
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
            b->hid_sects = fat_calc_hidden_sectors(p);
            b->num_sects = p->num_sect;
            b->magic_num = MBR_MAGIC_NUM32;
        } else if (ev.scan == 0x4100) /* F7 - Calculate size */
        {
            write_string(HINT_COLOR, StX2, StY + 15, "Reading FAT...               ");
            move_cursor(StX2 + 21, StY + 15);
            
            l = lc = 0;
            for (n = b->res_sects; n < b->res_sects + b->fat_size; n++) {
                get_event(&ev, EV_KEY | EV_NONBLOCK);
                if (ev.key == 27) {
                    write_string(TEXT_COLOR, StX, StY + 15, 
                        "     Minimum partition size:  press    to calculate");
                     write_string(HINT_COLOR, StX + 36, StY + 15, "F7");
                    goto calc_aborted;
                }

                if (disk_read_rel(p, n, tmp, 1) == FAILED) {
                    show_error("Error reading FAT table");
                    break;
                } else {
                    write_int(HINT_COLOR, StX2 + 16, StY + 15, 5, n - b->res_sects);
                    for (i = 0; i < SECT_SIZE; i++, l++)
                        if (tmp[i] != 0)
                           lc = l;
                }                
            }
            
            min_clust = (lc + 3) / 4 - 2;
            min_num_sect = b->res_sects + b->fat_copies * b->fat_size + min_clust * b->clust_size;
                
            sprintf(tmp,
                "%lu sectors = %s KiB",
                min_num_sect,
                sprintf_long(tmp1, (min_num_sect) / 2u));
            write_string(DATA_COLOR, StX2, StY + 15, tmp);

        calc_aborted: (void)0;
        }

    } /* while(1) */

    free(tmp);
    return OK;
} /* setup_fat32 */
