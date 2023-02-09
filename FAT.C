#include "part.h"
#include "fat.h"
#include "cache.h"
#include "dosext.h"
#include <stdio.h>
#include <ctype.h>
#include <dos.h>
#include <string.h>
#include <time.h>

#define FAT16_ENTRY_SIZE 2
#define FAT32_ENTRY_SIZE 4

/* one block cache for two FAT tables to speed up things */
static cache_block_t fat_table_cache[2];


int fat32_find_logical_drive(struct part_long *p)
{
    struct boot_ms_dos *b;
    struct dos_disk_info di;
    unsigned curr_drv, last_drv;
    int result = 0;
    
    b = malloc(sizeof(struct boot_ms_dos));
    if (!b) return FAILED;

    if (disk_read_rel(p, 0, b, 1) != OK) {
        result = FAILED;
        goto done;
    }

    _dos_getdrive(&curr_drv);
    _dos_setdrive(curr_drv, &last_drv);

    for (curr_drv = 3; curr_drv <= last_drv; curr_drv++) {
        if (!dos_get_serial(curr_drv, &di)) {
            printf("drive %c: %08lX\n", 'A' + curr_drv - 1, di.serial_number);
            /* found valid logical drive */
            if (b->x.f32.serial_num == di.serial_number)
            {
                result = curr_drv;
                goto done;
            }
        }
    }
        
done:
    free(b);
    return result;
}

/* according to Microsoft FAT specification */
static unsigned short fat16_cluster_size(unsigned long sectors)
{
    if (sectors <= 8400) return 0;
    else if (sectors <= 32680) return 2;
    else if (sectors <= 262144) return 4;
    else if (sectors <= 524288) return 8;
    else if (sectors <= 1048576) return 16;
    else if (sectors <= 2097152) return 32;
    /* WRONG?!? : else if (sectors <= 4194304) return 64;*/
    else if (sectors <= 4194081) return 64;
    else return 0xffff;
}


/* returns cluster size for FAT-12 paritions so that total */
/* clusters does not exceed 4084 */
static unsigned short fat12_cluster_size(unsigned long sectors)
{
    if (sectors < 39) return 0;
    else if (sectors <= 4123) return 1;
    else if (sectors <= 8207) return 2;
    else if (sectors <= 16375) return 4;
    else if (sectors <= 32711) return 8;
    else return 0xffff;
}


/* according to Microsoft FAT specification */
static unsigned fat32_cluster_size(unsigned long sectors)
{
    if (sectors <= 66600) return 0;
    else if (sectors <= 532480) return 1;
    else if (sectors <= 16777216) return 8;
    else if (sectors <= 33554432) return 16;
    else if (sectors <= 67108864) return 32;
    else return 64;
}

static unsigned int fat32_is_end_of_chain(unsigned long cluster)
{
    return  cluster >= 0x0FFFFFF8;
}

unsigned long fat_sector_count(struct boot_ms_dos *b)
{
    return (b->total_sect) ? b->total_sect : b->big_total;
}


static unsigned long fat_size(struct boot_ms_dos *b)
{
    return (b->fat_size16 != 0) ? b->fat_size16 : b->x.f32.fat_size;
}


static int is_fat32(struct boot_ms_dos *b)
{
    return b->root_entr == 0 && b->total_sect == 0 && b->fat_size16 == 0;
}


unsigned long fat_calc_hidden_sectors(struct part_long *p)
{
    struct part_long *q = p;
    while (q) {
        if (q->os_id == 0x0C00 ||
            q->os_id == 0x0E00 ||
            q->os_id == 0x0F00 ||
            q->os_id == 0x1C00 ||
            q->os_id == 0x1E00 ||
            q->os_id == 0x1F00)
        {
            return p->rel_sect + p->container_base;
        }

        q = q->container;
    }
    return p->rel_sect;
}


/* returns the number of root sectors for FAT-16 or 0 for FAT-32 */
static unsigned long fat_root_sector_count(struct boot_ms_dos *b)
{
    if (is_fat32(b)) return 0;

    return ((unsigned long)b->root_entr * sizeof(struct dirent) + b->sect_size - 1)
        / b->sect_size;
}


/* calculates the number of sectors preceeding the data area */
unsigned long fat_non_data_sector_count(struct boot_ms_dos *b)
{
    return b->res_sects + b->num_fats * fat_size(b) + fat_root_sector_count(b);
}


/* calculates number of clusters from filesystem size */
unsigned long fat_cluster_count(struct boot_ms_dos *b)
{
    return (fat_sector_count(b) - fat_non_data_sector_count(b)) / b->clust_size;
}


/* determines FAT type by number of clusters */
static int fat_type(struct boot_ms_dos *b)
{
    unsigned long clusters = fat_cluster_count(b);

    if (clusters > FAT16_MAX_DATA_CLUST) return FAT_32;
    else if (clusters > FAT12_MAX_DATA_CLUST) return FAT_16;
    else return FAT_12;
}


/* calculates the maximum cluster count (highest number-2) based on FAT table size */
static unsigned long fat_max_cluster_entries(struct boot_ms_dos *b, int fat_type)
{
    unsigned long fat_sz;

    fat_sz = fat_size(b);
    if (fat_sz == 0) return 0;

    if (fat_type == FAT_32) {
        return fat_sz * (b->sect_size / FAT32_ENTRY_SIZE) - 2;
    }
    else if (fat_type == FAT_16) {
        return fat_sz * (b->sect_size / FAT16_ENTRY_SIZE) - 2;
    } 
    else if (fat_type == FAT_12) {
        return fat_sz * b->sect_size * 2 / 3 - 2;
    }

    return 0;
}


/*--- FAT table routines ---------------------------------------------------*/

/* calculate sector and offset for cluster into FAT num_fat */
static void fat_calc_table_pos(struct boot_ms_dos *b, unsigned table_num, 
    unsigned long cluster, unsigned long *fat_sector, unsigned long *fat_offset)
{
    int fat_typ = fat_type(b);
    unsigned long base = b->res_sects + fat_size(b) * table_num;

    if (fat_typ == FAT_32) {
        *fat_sector = base + (FAT32_ENTRY_SIZE * cluster) / b->sect_size;
        *fat_offset = (FAT32_ENTRY_SIZE * cluster) % b->sect_size;
    } else if (fat_typ == FAT_16) {
        *fat_sector = base + (FAT16_ENTRY_SIZE * cluster) / b->sect_size;
        *fat_offset = (FAT16_ENTRY_SIZE * cluster) % b->sect_size;        
    } else if (fat_typ == FAT_12) {
        *fat_sector = base + (3 * cluster) / (b->sect_size * 2);
        *fat_offset = (3 * cluster / 2) % b->sect_size;                
    }
    /*printf("%3lu : %5lu : %5lu\n", cluster, *fat_sector, *fat_offset);*/
}


/* UNTESTED FOR FAT-12 !!! */
/* find FAT table entry for cluster in specified table */
int fat_get_table_entry(struct part_long *p, struct boot_ms_dos *b, 
    unsigned short table_num, unsigned long cluster, unsigned long *value)
{
    unsigned long fat_sector;
    unsigned long fat_offset;
    unsigned short v;
    int fat_typ;
    char *buf;

    if (fat_cluster_count(b) + 2 < cluster) return FAILED;
    fat_calc_table_pos(b, table_num, cluster, &fat_sector, &fat_offset);

    buf = cache_read(p, &fat_table_cache[table_num], fat_sector);
    if (!buf) return FAILED;

    fat_typ = fat_type(b);
    if (fat_typ == FAT_32) {
        *value = (*(unsigned long *)(buf + fat_offset));
    } 
    else if (fat_typ == FAT_16) {
        *value = (*(unsigned short *)(buf + fat_offset));        
    }
    else if (fat_typ == FAT_12) {        
        if (fat_offset < b->sect_size - 1) {
            v = *(unsigned short *)(buf + fat_offset);
        }
        else {
            v = *(unsigned char *)(buf + fat_offset);

            buf = cache_read(p, &fat_table_cache[table_num], fat_sector + 1);
            if (!buf) return FAILED;
            v |= *(unsigned char *)(buf) << 8;
        }
        if (cluster % 2) {
            *value = v >> 4;
        }
        else {
            *value = v & 0xfff;
        }
    }

    return OK;
}


/* set FAT table entry for cluster  in specified table */
int fat_set_table_entry(struct part_long *p, struct boot_ms_dos *b,
    unsigned short table_num, unsigned long cluster, unsigned long value)
{
    unsigned long fat_sector;
    unsigned long fat_offset;
    int fat_typ;
    unsigned char v1, v2;
    unsigned char mask1, mask2;
    unsigned char *v;
    char *buf;

    if (fat_cluster_count(b) + 2 < cluster) return FAILED;
    fat_calc_table_pos(b, table_num, cluster, &fat_sector, &fat_offset);

    buf = cache_read(p, &fat_table_cache[table_num], fat_sector);
    if (!buf) return FAILED;

    fat_typ = fat_type(b);
    if (fat_typ == FAT_32) {
        (*(unsigned long *)(buf + fat_offset)) = value;
    } else if (fat_typ == FAT_16) {
        (*(unsigned short *)(buf + fat_offset)) = value;        
    } else if (fat_typ == FAT_12) {
        if (cluster % 2) {
            v1 = value & 0x0f; mask1 = 0xf0;
            v2 = value >> 4; mask2 = 0x00;

        } else {
            v1 = value; mask1 = 0x00;
            v2 = (value >> 4) & 0xf0; mask2 = 0x0f;
        }

        v = (unsigned char *)(buf + fat_offset);
        *v = (*v & mask1) | v1; v++;
        
        /* cross sector border? */
        if (fat_offset >= b->sect_size - 1) {
            cache_mark_dirty(&fat_table_cache[table_num]);
            
            buf = cache_read(p, &fat_table_cache[table_num], fat_sector + 1);
            if (!buf) return FAILED;
            v = buf;
        }
        *v = (*v & mask2) | v2;
    }

    cache_mark_dirty(&fat_table_cache[table_num]);

    return OK;
}


/*--- cluster routines -----------------------------------------------------*/

/* calculates sector for given cluster */
unsigned long fat_cluster_to_sector(struct boot_ms_dos *b, unsigned long cluster)
{
    return fat_non_data_sector_count(b) + (cluster - 2) * b->clust_size;
}


/* calculates cluster number for given sector */
unsigned long fat_sector_to_cluster(struct boot_ms_dos *b, unsigned long sector)
{
    return (sector - fat_non_data_sector_count(b)) / b->clust_size + 2;
}


/* CURRENTLY UNUSED

static int fat_read_cluster(unsigned char * buf,
    struct part_long *p, 
    struct boot_ms_dos *b,
    unsigned long clust)
{
    int i;
    unsigned long sect = fat_cluster_to_sector(b, clust);
    if (clust < 2) return FAILED;

    for (i = 0; i < b->clust_size; i++) {
        if (disk_read_rel(p, sect+i, buf, 1) == FAILED) return FAILED;
        buf += SECT_SIZE;
    }
    return OK;
}


static int fat_write_cluster(unsigned char * buf,
    struct part_long *p, 
    struct boot_ms_dos *b,
    unsigned long clust)
{
    int i;
    unsigned long sect = fat_cluster_to_sector(b, clust);
    if (clust < 2) return FAILED;

    for (i = 0; i < b->clust_size; i++) {
        if (disk_write_rel(p, sect+i, buf, 1) == FAILED) return FAILED;
        buf += SECT_SIZE;
    }
    return OK;
}
*/

/* prepares a string to use as FAT volume label */
/* converts character to uppercase and replaces forbidden chars with space */
/* expects the input buffer to be no less than 12 characters */
void fat_canonicalize_label(char *label)
{
    int i;
    label[FAT_LABEL_LEN] = 0;

    if (*label) {
        for (i = 0; label[i] != 0 && i < FAT_LABEL_LEN; i++) {
            label[i] = toupper(label[i]);
            if (strchr("*?.,;:/\\|+=<>[]", label[i]))
                label[i] = ' ';      
        }
        for (; i < FAT_LABEL_LEN; i++)
            label[i] = ' ';    
    } else {
        strcpy(label, "NO NAME    ");
    }
}


/* returns pointer to label stored in boot record depending on FAT type */
char * fat_label(struct boot_ms_dos *b)
{
    if (is_fat32(b)) return b->x.f32.label;
    else return b->x.f16.label;
}


/* updates volume label file to match the label stored in boot record */
/* creates a new volume label if it does not exists */
/* in case of FAT-32 traverses root dir cluster chain */
/* currently DOES NOT create label file if new cluster must be allocated */
/* but this is not needed when formating */
int fat_update_label_file(struct part_long *p, struct boot_ms_dos *b)
{
    unsigned char *buf;
    struct dirent *dirent;
    unsigned long sector_count, j;
    unsigned long start_sect, sect;
    unsigned long data_cluster;
    unsigned long next_data_cluster = 0;



    int fat_typ = fat_type(b);
    if ((buf = malloc(SECT_SIZE)) == NULL) return FAILED;

    if (fat_typ == FAT_32) {
        data_cluster = b->x.f32.root_clust;
        if (fat_get_table_entry(p, b, 0, data_cluster, &next_data_cluster) == FAILED)
            goto failed;
        start_sect = fat_cluster_to_sector(b, data_cluster);
        sector_count = b->clust_size;
    } else {
        start_sect = b->res_sects + b->num_fats * fat_size(b);
        sector_count = fat_root_sector_count(b);
    }

    while (1) {
        for (sect = start_sect; sect < start_sect + sector_count; sect++)
        {
            if (disk_read_rel(p, sect, buf, 1) == FAILED) goto failed;
            dirent = (struct dirent *)buf;
    
            for (j = 0; j < DIRENT_PER_SECT; j++) {
                if (dirent->name[0] == 0 || 
                        (dirent->attr & DIRENT_LONG_NAME_MASK) != DIRENT_LONG_NAME_MASK &&
                        ((dirent->attr) & (DIRENT_ATTR_VOL | DIRENT_ATTR_DIR))  == DIRENT_ATTR_VOL) {  
    
                    if (memcmp(fat_label(b), NO_NAME_LABEL, sizeof(b->x.f16.label))) {
                        /* update label or create new one if not found */
                        memset(dirent, 0, sizeof(struct dirent));
                        dirent->attr |= DIRENT_ATTR_VOL;
                        memcpy(dirent->name, fat_label(b), sizeof(b->x.f16.label));
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

        if (fat_typ == FAT_32) {
            if (fat32_is_end_of_chain(next_data_cluster)) break;
            else {
                data_cluster = next_data_cluster;
                if (fat_get_table_entry(p, b, 0, data_cluster, &next_data_cluster) == FAILED)
                    goto failed;
                start_sect = fat_cluster_to_sector(b, data_cluster);
            }
        } else {
            break;
        }    
    }


success:
    free(buf);
    return OK;

failed:
    free(buf);
    return FAILED;
}


/* calculate and set FAT table size for FAT-12/16/32 so that the */
/* tables have enough but not more sectors than needed to store all clusters */
/* make sure to initialize boot_ms_dos struct before calling this */
static int fat_calculate_table_size(struct boot_ms_dos *b, int fat_type)
{
    unsigned long cluster_bound_high;

    /* calculate upper bound of FAT size */
    cluster_bound_high = (fat_sector_count(b) - b->res_sects) / b->clust_size;

    if (fat_type == FAT_32) {
        b->x.f32.fat_size = FAT32_ENTRY_SIZE * cluster_bound_high / b->sect_size; 
        b->fat_size16 = 0;       
    } else if (fat_type == FAT_16) {
        b->fat_size16 = FAT16_ENTRY_SIZE * cluster_bound_high / b->sect_size;
    } else {
        b->fat_size16 = 3 * cluster_bound_high / (2 * b->sect_size);
    }
        
    /* decrease FAT size until it does not fit all clusters */
    while (fat_max_cluster_entries(b, fat_type) > fat_cluster_count(b)) {
        if (fat_type == FAT_32) {
            b->x.f32.fat_size--;
        } else {
            b->fat_size16--;
        }
    }

    if (fat_type == FAT_32) {
        b->x.f32.fat_size++;
    } else {
        b->fat_size16++;
    }

    return OK;
}


static unsigned long fat_calculate_serial(struct part_long *p, struct boot_ms_dos *b)
{
    return ((p->rel_sect << 16) + (p->num_sect * ((long)b % 451))) +
        ((dinfo.total_sects % 12345L) ^ (dinfo.total_sects * 67891L)) +
        ((dinfo.disk * 123L) ^ (dinfo.num_heads % 7)) + clock();
}


/* initializes a new BPB in memory based on */
/*    sector count, root dir entries and fat type */
int fat_initialize_bootrec(struct part_long *p, struct boot_ms_dos *b,
                           unsigned short root_entr,int fat_type,
                           char label[])
{
    unsigned short cluster_size;

    memset(b, 0, sizeof(struct boot_ms_dos));

    /* BPB fields shared between FAT12/16/32 */
    memmove(b->sys_id, "MSWIN4.1", sizeof b->sys_id);
    b->sect_size  = SECT_SIZE;
    b->res_sects  = (fat_type == FAT_32) ? 32 : 1;
    b->root_entr  = (fat_type == FAT_32) ? 0 : root_entr;
    b->num_fats   = 2;
    b->media_desc = 0xF8;
    b->magic_num = MBR_MAGIC_NUM;
    b->track_size = dinfo.num_sects;
    b->num_sides = dinfo.num_heads;

    b->hid_sects  = fat_calc_hidden_sectors(p);
    b->total_sect = (p->num_sect < 65536L) ? p->num_sect : 0;
    b->big_total  = (p->num_sect < 65536L) ? 0 : p->num_sect;

    fat_canonicalize_label(label);

    if (fat_type == FAT_32) {
        memmove(b->x.f32.fs_id, "FAT32   ", sizeof b->x.f32.fs_id);      
        cluster_size = fat32_cluster_size(fat_sector_count(b));

        b->x.f32.fs_info_sect_num = 1;
        b->x.f32.root_clust = 2;
        b->x.f32.bs_bak_sect = FAT32_BACKUP_SECTOR;
        b->x.f32.drive_num = dinfo.disk;
        b->x.f32.ext_signat = 0x29;
        b->x.f32.serial_num = fat_calculate_serial(p, b);

        memmove(b->x.f32.label, label, sizeof(b->x.f32.label));

        memmove(b->jmp, "\xEB\x58\x90", 3);
        memmove(b->x.f32.xcode, EMP_IPL, EMP_IPL_SIZE);
        strncpy(b->x.f32.xcode + EMP_IPL_SIZE, MESG_NON_SYSTEM, sizeof(b->x.f32.xcode) - EMP_IPL_SIZE);

    } else {

        if (fat_type == FAT_16) {
            memmove(b->x.f16.fs_id, "FAT16   ", sizeof b->x.f16.fs_id);      
            cluster_size = fat16_cluster_size(fat_sector_count(b));
        }
        else if (fat_type == FAT_12) {
            memmove(b->x.f16.fs_id, "FAT12   ", sizeof b->x.f16.fs_id);
            cluster_size = fat12_cluster_size(fat_sector_count(b));
        }

        b->x.f16.drive_num = dinfo.disk;
        b->x.f16.ext_signat = 0x29;
        b->x.f16.serial_num = fat_calculate_serial(p, b);

        memmove(b->x.f16.label, label, sizeof(b->x.f16.label));

        /* install dummy IPL */
        memmove(b->jmp, "\xEB\x3C\x90", 3);
        memmove(b->x.f16.xcode, EMP_IPL, EMP_IPL_SIZE);
        strncpy(b->x.f16.xcode + EMP_IPL_SIZE, MESG_NON_SYSTEM, sizeof(b->x.f32.xcode) - EMP_IPL_SIZE);

    }

    if (cluster_size == 0) return FAT_ERR_TOO_SMALL;
    if (cluster_size == 0xffff) return FAT_ERR_TOO_LARGE;
    b->clust_size = cluster_size;

    fat_calculate_table_size(b, fat_type);

    return OK;
}


void fat32_initialize_ext_bootrec(struct boot_ms_dos *b,
                                  struct fat32_ext_bootrec *eb)
{
    memset(eb, 0, sizeof(struct fat32_ext_bootrec));
    eb->lead_sig = FAT32_LEAD_SIG;
    eb->struc_sig = FAT32_STRUC_SIG;
    eb->free_cluster_count = fat_cluster_count(b) - 1; /* root cluster */
    eb->next_free_cluster = 2;
    eb->trail_sig = FAT32_TRAIL_SIG;
    eb->trail2_sig = FAT32_TRAIL_SIG;
}


/* initializes the FAT-12/16/32 tables */
int fat_initialize_tables(struct part_long *p, struct boot_ms_dos *b,
    int (*write_progress)(unsigned long, unsigned long))
{
    unsigned long *fat;
    unsigned long fat_sz;
    int fat_typ;
    int copy;
    int result;
    char buf[40];

    fat_typ = fat_type(b);
    fat_sz = fat_size(b);

    for (copy = 0; copy < b->num_fats; copy++)
    {
        sprintf(buf, "^Format: writing FAT %d of %u ...", copy+1, b->num_fats);
        progress(buf);

        /* fill in the first cluster entries */
        if (!(fat = malloc(b->sect_size))) return FAILED;   
        memset(fat, 0, SECT_SIZE);
        if (fat_typ == FAT_32) {
            fat[0] = 0xFFFFFFF8;
            fat[1] = 0xFFFFFFFF;
            fat[2] = 0xFFFFFFFF;
        } else if (fat_typ == FAT_16) {
            fat[0] = 0xFFFFFFF8;    /* FAT-16 0xFFF8, 0xFFFF */
        } else if(fat_typ == FAT_12) {
            fat[0] = 0x00FFFFF8;
        }

        if (disk_write_rel(p, b->res_sects + fat_sz * copy, fat, 1) == FAILED) {
            free(fat);
            return FAILED;
        }
        free(fat);

        /* fill rest of FAT with zero */
        result = part_fill_sectors(p,
                    b->res_sects + fat_sz * copy + 1,
                    fat_sz - 1,
                    0,
                    write_progress,
                    NULL);
        if (result != OK) return result;


    }

    return OK;
}


/* clear root directory */
int fat_initialize_root(struct part_long *p, struct boot_ms_dos *b)
{
    unsigned short num_sectors;
    unsigned long abs_sector;

    if (is_fat32(b)) {
        abs_sector = fat_cluster_to_sector(b, b->x.f32.root_clust);
        num_sectors = b->clust_size;
    } else {
        abs_sector = b->res_sects + b->num_fats * fat_size(b);
        num_sectors = fat_root_sector_count(b);
    }

    part_fill_sectors(p, abs_sector, num_sectors, 0, NULL, NULL);

    return OK;
}

int fat_flush()
{
    int result;
    result = cache_flush(&fat_table_cache[0]);
    if (!result) return result;

    return cache_flush(&fat_table_cache[1]);    
}
