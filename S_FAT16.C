#include "part.h"
#include "fat.h"
#include <ctype.h>
#include <string.h>
#include <time.h>


#define BBT_SIZE 128

#define F_NORM  0
#define F_VERIFY 1
#define F_DESTR 2

#define FAT12_MAX_SIZE 6144   /* 4096 12-bit entries */
#define MAX_TABLE_SECT_FAT12 12


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

unsigned int fat32_eoc(unsigned long cluster)
{
    return  cluster >= 0x0FFFFFF8;
}

static unsigned long fat_num_sectors(struct boot_ms_dos *b)
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


/* returns the numer of root sectors for FAT-16 or 0 for FAT-32 */
static unsigned long fat_root_sectors(struct boot_ms_dos *b)
{
    if (is_fat32(b)) return 0;

    return ((unsigned long)b->root_entr * sizeof(struct dirent) + b->sect_size - 1)
        / b->sect_size;
}


static unsigned long fat_non_data_sectors(struct boot_ms_dos *b)
{
    return b->res_sects + b->num_fats * fat_size(b) + fat_root_sectors(b);
}


static unsigned long fat_num_data_clusters(struct boot_ms_dos *b)
{
    return (fat_num_sectors(b) - fat_non_data_sectors(b)) / b->clust_size;
}


static int fat_type(struct boot_ms_dos *b)
{
    unsigned long clusters = fat_num_data_clusters(b);

    if (clusters > MAX_DATA_CLUST_16) return FAT_32;
    else if (clusters > MAX_DATA_CLUST_12) return FAT_16;
    else return FAT_12;
}


/* calculates the maximum cluster count based on FAT table size */
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

/*--- cache routines -------------------------------------------------------*/

static char fat_cache[SECT_SIZE];
static unsigned long fat_cache_sector = 0;
static void *fat_cache_entity = NULL;
static int fat_cache_dirty = 0; /* cache was written to */

/*static void fat_cache_reset(struct part_long *p)
{
    fat_cache_sector = 0;
    fat_cache_entity = NULL;
}*/


/* mark FAT cache as dirty */
static void fat_cache_mark_dirty()
{
    fat_cache_dirty = 1;
}


static int fat_cache_flush()
{
    struct part_long *p;

    if (fat_cache_entity == NULL) return OK;

    if (fat_cache_dirty) {
        p = (struct part_long *)fat_cache_entity;
        if (disk_write_rel(p, fat_cache_sector, fat_cache, 1) == FAILED) return FAILED;
        fat_cache_dirty = 0;
    }

    return OK;
}


static int in_fat_cache(struct part_long *p, unsigned long sector)
{
    return fat_cache_sector == sector && fat_cache_entity == p;
}


static void * fat_cache_read(struct part_long *p, unsigned long sector)
{
    if (in_fat_cache(p, sector)) return fat_cache;

    fat_cache_flush();
    if (disk_read_rel(p, sector, fat_cache, 1) == FAILED) return NULL;

    fat_cache_sector = sector;
    fat_cache_entity = p;
    return fat_cache;
}


/*--- FAT table routine ----------------------------------------------------*/

/* find FAT table entry for cluster */
int fat_get_table_entry(struct part_long *p, struct boot_ms_dos *b, unsigned table_num, unsigned long cluster, unsigned long *value)
{
    unsigned long fat_sector;
    unsigned long fat_offset;
    unsigned long base;
    int fat_typ;
    char *buf;

    fat_typ = fat_type(b);
    base = b->res_sects + fat_size(b) * table_num;

    if (fat_num_data_clusters(b) + 2 < cluster) return FAILED;

    if (fat_typ == FAT_32) {
        fat_sector = base + (FAT32_ENTRY_SIZE * cluster) / b->sect_size;
        fat_offset = (FAT32_ENTRY_SIZE * cluster) % b->sect_size;
    } else if (fat_typ == FAT_16) {
        fat_sector = base + (FAT16_ENTRY_SIZE * cluster) / b->sect_size;
        fat_offset = (FAT16_ENTRY_SIZE * cluster) % b->sect_size;        
    }

    buf = fat_cache_read(p, fat_sector);
    if (!buf) return FAILED;

    if (fat_typ == FAT_32) {
        *value = (*(unsigned long *)(buf + fat_offset));
    } else if (fat_typ == FAT_16) {
        *value = (*(unsigned short *)(buf + fat_offset));        
    }

    return OK;
}


/* set FAT table entry for cluster */
int fat_set_table_entry(struct part_long *p, struct boot_ms_dos *b, unsigned table_num, unsigned long cluster, unsigned long value)
{
    unsigned long fat_sector;
    unsigned long fat_offset;
    unsigned long base;
    int fat_typ;
    char *buf;

    fat_typ = fat_type(b);
    base = b->res_sects + fat_size(b) * table_num;

    if (fat_num_data_clusters(b) + 2 < cluster) return FAILED;

    if (fat_typ == FAT_32) {
        fat_sector = base + FAT32_ENTRY_SIZE * cluster / b->sect_size;
        fat_offset = FAT32_ENTRY_SIZE * cluster % b->sect_size;
    } else if (fat_typ == FAT_16) {
        fat_sector = base + FAT16_ENTRY_SIZE * cluster / b->sect_size;
        fat_offset = FAT16_ENTRY_SIZE * cluster % b->sect_size;        
    }

    buf = fat_cache_read(p, fat_sector);
    if (!buf) return FAILED;

    if (fat_typ == FAT_32) {
        (*(unsigned long *)(buf + fat_offset)) = value;
    } else if (fat_typ == FAT_16) {
        (*(unsigned short *)(buf + fat_offset)) = value;        
    }

    fat_cache_mark_dirty();

    return OK;
}


/*--- cluster routines -----------------------------------------------------*/

/* calculates the data sector for the given cluster */
static unsigned long fat_cluster_to_data_sector(struct boot_ms_dos *b, unsigned long cluster)
{
    return fat_non_data_sectors(b) + (cluster - 2) * b->clust_size;
}


int fat_read_cluster(unsigned char * buf,
    struct part_long *p, 
    struct boot_ms_dos *b,
    unsigned long clust)
{
    int i;
    unsigned long sect = fat_cluster_to_data_sector(b, clust);
    if (clust < 2) return FAILED;

    for (i = 0; i < b->clust_size; i++) {
        if (disk_read_rel(p, sect+i, buf, 1) == FAILED) return FAILED;
        buf += SECT_SIZE;
    }
    return OK;
}


int fat_write_cluster(unsigned char * buf,
    struct part_long *p, 
    struct boot_ms_dos *b,
    unsigned long clust)
{
    int i;
    unsigned long sect = fat_cluster_to_data_sector(b, clust);
    if (clust < 2) return FAILED;

    for (i = 0; i < b->clust_size; i++) {
        if (disk_write_rel(p, sect+i, buf, 1) == FAILED) return FAILED;
        buf += SECT_SIZE;
    }
    return OK;
}


/* prepares a string to use as FAT volume label */
/* converts character to uppercase and replaces forbidden chars with space */
/* expects the input string to be no less than 12 characters */
static void fat_canonicalize_label(char *label)
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


static char * fat_label(struct boot_ms_dos *b)
{
    if (is_fat32(b)) return b->x.f32.label;
    else return b->x.f16.label;
}


static int fat_update_label_file(struct part_long *p, struct boot_ms_dos *b)
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
        start_sect = fat_cluster_to_data_sector(b, data_cluster);
        sector_count = b->clust_size;
    } else {
        start_sect = b->res_sects + b->num_fats * fat_size(b);
        sector_count = fat_root_sectors(b);
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
            if (fat32_eoc(next_data_cluster)) break;
            else {
                data_cluster = next_data_cluster;
                if (fat_get_table_entry(p, b, 0, data_cluster, &next_data_cluster) == FAILED)
                    goto failed;
                start_sect = fat_cluster_to_data_sector(b, data_cluster);
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


/* calculate and set FAT table size for FAT-12, FAT-16  */
/* make sure to initialize boot_ms_dos struct before calling this */
static int fat_calculate_table_size(struct boot_ms_dos *b, int fat_type)
{
    unsigned long cluster_bound_high;

    /* calculate upper bound of FAT size */
    cluster_bound_high = (fat_num_sectors(b) - b->res_sects) / b->clust_size;

    if (fat_type == FAT_32) {
        b->x.f32.fat_size = FAT32_ENTRY_SIZE * cluster_bound_high / b->sect_size; 
        b->fat_size16 = 0;       
    } else if (fat_type == FAT_16) {
        b->fat_size16 = FAT16_ENTRY_SIZE * cluster_bound_high / b->sect_size;
    } else {
        b->fat_size16 = 3 * cluster_bound_high / (2 * b->sect_size);
    }
        
    /* decrease FAT size until it does not fit all clusters */
    while (fat_max_cluster_entries(b, fat_type) > fat_num_data_clusters(b)) {
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
static int fat_initialize_bootrec(struct part_long *p,
                                  struct boot_ms_dos *b,
                                  unsigned short root_entr,
                                  int fat_type,
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

    fat_canonicalize_label(label);

    if (fat_type == FAT_32) {
        memmove(b->x.f32.fs_id, "FAT32   ", sizeof b->x.f32.fs_id);      
        cluster_size = fat32_cluster_size(p->num_sect);

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
            cluster_size = fat16_cluster_size(p->num_sect);
        }
        else if (fat_type == FAT_12) {
            memmove(b->x.f16.fs_id, "FAT12   ", sizeof b->x.f16.fs_id);
            cluster_size = fat12_cluster_size(p->num_sect);
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

    b->hid_sects  = fat_calc_hidden_sectors(p);
    b->total_sect = (p->num_sect < 65536L) ? p->num_sect : 0;
    b->big_total  = (p->num_sect < 65536L) ? 0 : p->num_sect;

    if (cluster_size == 0) return FAT_ERR_TOO_SMALL;
    if (cluster_size == 0xffff) return FAT_ERR_TOO_LARGE;
    b->clust_size = cluster_size;

    fat_calculate_table_size(b, fat_type);

    return OK;
}


static void fat32_initialize_ext_bootrec(struct boot_ms_dos *b,
                                        struct fat32_ext_bootrec *eb)
{
    memset(eb, 0, sizeof(struct fat32_ext_bootrec));
    eb->lead_sig = FAT32_LEAD_SIG;
    eb->struc_sig = FAT32_STRUC_SIG;
    eb->free_cluster_count = fat_num_data_clusters(b) - 1; /* root cluster */
    eb->next_free_cluster = 3;
    eb->trail_sig = FAT32_TRAIL_SIG;
    eb->trail2_sig = FAT32_TRAIL_SIG;
}


int write_progress(unsigned long curr, unsigned long total)
{
    char buf[24];
    static unsigned long last = 0xff;

    if (curr != last) {
        sprintf(buf, "%% %3lu%% written", curr * 100 / total);
        last = curr;
        return progress(buf) != CANCEL;
    }

    return 1;
}


int verify_progress(unsigned long curr, unsigned long total)
{
    char buf[32];
    static unsigned long last = 0xff;

    if (curr != last) {
        sprintf(buf, "%% %3lu%% | 0 errors", curr * 100 / total);
        last = curr;
        return progress(buf) != CANCEL;
    }

    return 1;
}


/* initializes the FAT-16 tables */
/* marks bad clusters for sectors given in bbt if bbt != NULL and num_bad>0 */
static int fat_initialize_tables(struct part_long *p,
                                struct boot_ms_dos *b)
{
    unsigned long *fat;
    unsigned long fat_sz;
    int fat_typ;
    int copy;
    int result;
    char buf[40];

    fat_typ = fat_type(b);
    fat_sz = fat_size(b);

    /*sprintf(buf, "FAT size: %lu", fat_sz);
    show_error(buf);*/
    for (copy = 0; copy < b->num_fats; copy++)
    {
        sprintf(buf, "^Format: writing FAT %d of %u ...", copy+1, b->num_fats);
        progress(buf);

        result = part_fill_sectors(p, b->res_sects + fat_sz * copy, fat_sz, 0, write_progress, NULL);
        if (result != OK) return result;


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
    }

    return OK;
}


int fat_initialize_root(struct part_long *p, struct boot_ms_dos *b)
{
    char *buf;
    unsigned short sector;
    unsigned short num_sectors;
    unsigned long abs_sector;

    if (is_fat32(b)) {
        abs_sector = fat_cluster_to_data_sector(b, b->x.f32.root_clust);
        num_sectors = b->clust_size;
    } else {
        abs_sector = b->res_sects + b->num_fats * fat_size(b);
        num_sectors = fat_root_sectors(b);
    }

    if (!(buf = malloc(b->sect_size))) return FAILED;
    memset(buf, 0, b->sect_size);

    for (sector = 0; sector < num_sectors; sector++) {
        if (disk_write_rel(p, abs_sector++, buf, 1) == FAILED) {
            free(buf);
            return FAILED;
        }        
    }

    free(buf);
    return OK;
}


int fat_bad_block_check(struct part_long *p) {
    return OK;
}

/*   0x01, "DOS FAT-12"			*/
/*   0x04, "DOS FAT-16 (<=32Mb)"	*/
/*   0x06, "BIGDOS FAT-16 (>=32Mb)"	*/
int format_fat(struct part_long *p, char **argv)
{
    int result;
    char *data_pool;
    struct boot_ms_dos *b;
    struct fat32_ext_bootrec *eb;
    unsigned long sys_type;

    char label[FAT_LABEL_LEN + 1] = "";
    unsigned int form_type  = F_VERIFY;

    if ((data_pool = malloc(sizeof(struct boot_ms_dos) + sizeof(struct fat32_ext_bootrec))) == 0) {
        show_error(ERROR_MALLOC);
        return FAILED;
    }

    b   = (struct boot_ms_dos *) data_pool;
    eb  = (struct fat32_ext_bootrec *) (data_pool + sizeof(struct boot_ms_dos));

    if (QUICK_BASE(p) > QUICK_BASE(p) + p->num_sect) {
        progress("^Partition crosses 2TiB boundary. Refusing to format.");
        result = FAILED;
        goto done;
    }

    /* make test read to check if whole partition is accessible */
    if (disk_read_rel(p, p->num_sect-1, data_pool, 1) == FAILED) {
        progress(TEXT("^Can not access last sector of partition. Refusing to format!"));
        result = FAILED;
        goto done;
    }

    /* parse arguments */
    while (*argv != 0) {
        if (_stricmp(*argv, "/destructive") == 0)
            form_type = F_DESTR;
        else if (_stricmp(*argv, "/verify") == 0)
            form_type = F_VERIFY;
        else if (_strnicmp(*argv, "/l:", 3) == 0) {
            strncpy(label, (*argv) + 3, FAT_LABEL_LEN);
            label[FAT_LABEL_LEN] = 0;
        } else {
            progress("^Unknown option:");
            progress(*argv);
            result = FAILED;
            goto done;
        }
        argv++;
    }

    /* set FAT type */
    switch(p->os_id) {
        case 0x0b00: case 0x0c00:
        case 0x1b00: case 0x1c00:
            sys_type = FAT_32;
            break;
        case 0x0400: case 0x0600: case 0x0e00:
        case 0x1400: case 0x1600: case 0x1e00:
            sys_type = FAT_16;
            break;
        case 0x0100: case 0x1100:
            sys_type = FAT_12;
            break;
    }

    if (sys_type == FAT_32) {
        result = fat_initialize_bootrec(p, b, 512, FAT_32, label);
        
        if (result == FAT_ERR_TOO_SMALL) {
            progress(TEXT("^FAT-32 partition too small. Use FAT-16 instead!"));
            result = FAILED;
            goto done;
        }

        fat32_initialize_ext_bootrec(b, eb);       
    }
    else if (sys_type == FAT_16) {
        result = fat_initialize_bootrec(p, b, 512, FAT_16, label);
        
        if (result == FAT_ERR_TOO_SMALL) {
            progress(TEXT("^FAT-16 partition too small. Use FAT-12 instead!"));
            result = FAILED;
            goto done;
        }
        if (result == FAT_ERR_TOO_LARGE) {
            progress(TEXT("^FAT-16 partition too large. Use FAT-32 instead!"));
            result = FAILED;
            goto done;
        }
    }
    else if (sys_type == FAT_12) {
        result = fat_initialize_bootrec(p, b, 224, FAT_12, label);

        if (result == FAT_ERR_TOO_SMALL) {
            progress(TEXT("^FAT-12 partition too small!"));
            result = FAILED;
            goto done;
        }
        if (result == FAT_ERR_TOO_LARGE) {
            progress(TEXT("^FAT-12 partition too large. Use FAT-16 or FAT-32 instead!"));
            goto done;
        }
    }

    flush_caches();

    /*if (form_type == F_DESTR)
        result = generic_format(p, BBT_SIZE, bbt);
    else
        result = 0;
    */
    if (result < 0) /* format failed or canceled */
    {
        goto done;
    }

    disk_lock(dinfo.disk);

    progress("^Initializing file system ...");
    /*if (form_type == F_VERIFY) {
        progress("^Format: checking FAT area for bad sectors ...");
        result = part_verify_sectors(p, 0, fat_non_data_sectors(b), verify_progress, NULL);

        if (result != OK) {
            goto done;
        }
    }*/

    progress("^Format: writing boot sector ...");
    if (sys_type == FAT_32) {
        result = disk_write_rel(p, 0, b, 3);
        result |= disk_write_rel(p, FAT32_BACKUP_SECTOR, b, 3);
    } else {
        result = disk_write_rel(p, 0, b, 1);
    }
    if (result != OK) {
        progress("Error writing boot sector.");
        goto done;        
    }

    result = fat_initialize_tables(p, b);        

    if (result != OK) {
        progress("Error writing FAT.");
        goto done;
    }

    progress("^Format: writing root directory ...");
    result = fat_initialize_root(p, b);
    if (result != OK) {
        progress("Error writing root directory.");
        goto done;        
    }

    if (form_type == F_VERIFY) {
        progress("^Format: searching for bad sectors ...   you may skip this step via ESC");
        result = part_verify_sectors(p, fat_non_data_sectors(b), p->num_sect - fat_non_data_sectors(b), verify_progress, NULL);
        if (result == FAILED) {
            progress("Error checking for bad clusters.");
            goto done;        
        }
    } else if (form_type == F_DESTR) {
        progress("^Format: overwriting old data ...        you may skip this step via ESC");
        result = part_fill_sectors(p, fat_non_data_sectors(b), p->num_sect - fat_non_data_sectors(b), 0, write_progress, NULL);
        if (result == FAILED) {
            progress("Error cleaning data area.");
            goto done;        
        }
    }

    progress("^Format: updating volume label ...");
    result = fat_update_label_file(p, b);
    if (result != OK) {
        progress("Error updating volume label.");
        goto done;        
    }


done:
    fat_cache_flush();
    disk_unlock(dinfo.disk);
    free(data_pool);

    return result;

} /* format_fat */


int format_embr(struct part_long *p, char **argv)
{
    struct mbr *mbr;
    (void)argv;

    if (!(mbr = malloc(sizeof(struct mbr)))) return FAILED;
    flush_caches();

    disk_lock(dinfo.disk);

    progress("^Initializing Extended DOS partition ...");

    progress("Writing Extended Master Boot Record ...");

    memset(mbr, 0, SECT_SIZE);
    memmove(mbr->x.std.code, EMP_IPL, EMP_IPL_SIZE);
    strncpy(mbr->x.std.code + EMP_IPL_SIZE,
            MESG_EXT_NONBOOT,
            sizeof(mbr->x.std.code) - EMP_IPL_SIZE);
    mbr->magic_num = MBR_MAGIC_NUM;

    if (disk_write_rel(p, 0, mbr, 1) == FAILED) {
        progress("Error Writing Extended Master Boot Record.");
        disk_unlock(dinfo.disk);
        free(mbr);
        return FAILED;
    }

    disk_unlock(dinfo.disk);
    free(mbr);
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
    printf("                  Sectors per FAT:  %d\n", b->fat_size16);
    printf("                 Sectors per side:  %-2d          %d\n",
           b->track_size,
           dinfo.num_sects);
    printf("                  Number of sides:  %-3d         %d\n",
           b->num_sides,
           dinfo.num_heads);
    printf("Hidden sectors prior to partition:  %-10s  %-10s\n",
           sprintf_long(tmp1, b->hid_sects),
           sprintf_long(tmp2, fat_calc_hidden_sectors(p)));
    printf("      Big total number of sectors:  %-10s  %-10s\n",
           b->big_total == 0 ? "0" : sprintf_long(tmp1, b->big_total),
           (b->total_sect == 0 || p->num_sect > 65535L)
               ? sprintf_long(tmp2, p->num_sect)
               : "0");
    printf("                     Drive number:  %-3d         %d\n",
           b->x.f16.drive_num,
           dinfo.disk);
    printf("   Extended boot record signature:  %02Xh\n", b->x.f16.ext_signat);
    printf("             Volume serial number:  %08lX\n", b->x.f16.serial_num);
    printf("                     Volume label:  %-.11s\n", b->x.f16.label);
    printf("                 File system type:  %-.8s    %s\n",
           b->x.f16.fs_id,
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
    if (strncmp(b->x.f16.fs_id, "FAT12", 5) == 0)
        syst = 12;
    if (strncmp(b->x.f16.fs_id, "FAT16", 5) == 0)
        syst = 16;

    max_clust =
        (syst == 0) ? (0) : ((long)SECT_SIZE * 8u * b->fat_size16 / syst - 2u);

    if (syst == 12 && max_clust > MAX_DATA_CLUST_12) max_clust = MAX_DATA_CLUST_12;
    if (syst == 16 && max_clust > MAX_DATA_CLUST_16) max_clust = MAX_DATA_CLUST_16;

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

    sprintf(tmp, "%-.8s", b->x.f16.fs_id);
    write_string(DATA_COLOR, StX2, StY + 2, tmp);

    sprintf(tmp, "%Xh", b->media_desc);
    write_string(DATA_COLOR, StX3, StY + 2, tmp);

    sprintf(tmp,
            "%-3u %s bytes/cluster",
            b->clust_size,
            sprintf_long(tmp1, (unsigned long)b->clust_size * b->sect_size));
    write_string(DATA_COLOR, StX2, StY + 3, tmp);

    sprintf(tmp, "%d", b->sect_size);
    write_string(DATA_COLOR, StX3, StY + 3, tmp);

    
    sprintf(tmp,
            "%-3d",
            b->fat_size16);
    write_string(DATA_COLOR, StX2, StY + 4, tmp);
    
    sprintf(tmp, "%d", b->num_fats);
    write_string(DATA_COLOR, StX3, StY + 4, tmp);

    sprintf(tmp, "%d", b->root_entr);
    write_string(DATA_COLOR, StX2, StY + 5, tmp);

    sprintf(tmp, "%Xh", b->x.f16.ext_signat);
    write_string(DATA_COLOR, StX3, StY + 5, tmp);

    sprintf(tmp, "%08lX", b->x.f16.serial_num);
    write_string(DATA_COLOR, StX2, StY + 6, tmp);

    sprintf(tmp, " %u", dinfo.disk);
    write_string(DATA_COLOR, StX2 + 12, StY + 8, tmp);

    sprintf(tmp, " %u", dinfo.num_heads);
    write_string(DATA_COLOR, StX2 + 12, StY + 9, tmp);

    sprintf(tmp, " %u", dinfo.num_sects);
    write_string(DATA_COLOR, StX2 + 12, StY + 10, tmp);

    sprintf(tmp, " %-9lu", fat_calc_hidden_sectors(p));
    write_string(DATA_COLOR, StX2 + 12, StY + 11, tmp);

    sprintf(tmp, " %-5u", (p->num_sect > 65535L) ? 0 : p->num_sect);
    write_string(DATA_COLOR, StX2 + 12, StY + 12, tmp);

    sprintf(tmp, " %-9lu", (p->num_sect > 65535L) ? p->num_sect : 0);
    write_string(DATA_COLOR, StX2 + 12, StY + 13, tmp);

    num_sect = fat_num_sectors(b);

    n = fat_non_data_sectors(b);
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
            tmp[i] = (b->x.f16.label[i] == 0) ? ' ' : b->x.f16.label[i];
        tmp[i] = 0;
        write_string(DATA_COLOR, StX2, StY + 7, tmp);
        sprintf(tmp, "%-5d", b->x.f16.drive_num);
        write_string((b->x.f16.drive_num == dinfo.disk) ? DATA_COLOR : INVAL_COLOR,
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
        write_string((b->hid_sects == fat_calc_hidden_sectors(p)) ? DATA_COLOR : INVAL_COLOR,
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
            (num_sect - fat_non_data_sectors(b)) / b->clust_size, max_clust);

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
            memmove(tmp, b->x.f16.label, 11);
            tmp[11] = 0;
            for (i = 10; i >= 0 && tmp[i] == ' '; i--)
                tmp[i] = 0;
            memmove(tmp1, tmp, 12);
            edit_str_field(&ev, 0, EDIT_COLOR, StX2, StY + 7, 12, tmp, &pos);
            if (memcmp(tmp, tmp1, 12) != 0) {
                fat_canonicalize_label(tmp);
                memcpy(b->x.f16.label, tmp, FAT_LABEL_LEN);
            }
        }

        if (act == 1) {
            n = b->x.f16.drive_num;
            edit_int_field(&ev, 0, EDIT_COLOR, StX2, StY + 8, -5, &n, 255L);
            b->x.f16.drive_num = n;
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
                fat_update_label_file(p, b) == FAILED) {
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
            b->x.f16.drive_num  = dinfo.disk;
            b->num_sides  = dinfo.num_heads;
            b->track_size  = dinfo.num_sects;
            b->hid_sects  = fat_calc_hidden_sectors(p);
            b->total_sect = (p->num_sect < 65536L) ? p->num_sect : 0;
            b->big_total  = (p->num_sect < 65536L) ? 0 : p->num_sect;
            b->magic_num  = MBR_MAGIC_NUM;
        } else if (ev.scan == 0x4000) /* F6 - Code */
        {
            memmove(b->x.f16.xcode, fat_boot_code->x.f16.xcode, sizeof(b->x.f16.xcode));
        } else if (ev.scan == 0x4100) /* F7 - calculate min size */
        {
            write_string(HINT_COLOR, StX2, StY + 15, "Reading FAT...                 ");
            move_cursor(StX2 + 14, StY + 15);
        
            fatsz = min(256, b->fat_size16);
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
        
        
            min_num_sect = fat_non_data_sectors(b) +
                ((unsigned long) min_clust) * b->clust_size;
            
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
