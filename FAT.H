#ifndef FAT_HEADER
#define FAT_HEADER

#define FAT_LABEL_LEN (sizeof ((struct boot_ms_dos *)(0))->x.f16.label)

#define MAX_DATA_CLUST_12 4084 
#define MAX_DATA_CLUST_16 65524

#define FAT12_MAX_TABLE_SECTS 12
#define FAT12_MAX_TABLE_SIZE  6144
#define FAT16_MAX_TABLE_SECTS 256

#define FAT16_ENTRY_SIZE 2
#define FAT32_ENTRY_SIZE 4

#define FAT_ERR_TOO_SMALL -1
#define FAT_ERR_TOO_LARGE -2

enum {
    FAT_12 = 12,
    FAT_16 = 16,
    FAT_32 = 32
};

#define FAT32_BACKUP_SECTOR 6 /* location of second boot record copy */

#define FAT32_TRAIL_SIG 0xAA550000L
#define FAT32_LEAD_SIG 0x41615252
#define FAT32_STRUC_SIG 0x61417272

#define INVALID_CLUSTER 0xfffffff7

_Packed struct boot_ms_dos
{
    unsigned char jmp[3];      /* Must be 0xEB, 0x3C, 0x90		*/
    unsigned char sys_id[8];   /* Probably:   "MSDOS5.0"		*/
    unsigned short sect_size;  /* Sector size in bytes (512)		*/
    unsigned char clust_size;  /* Sectors per cluster (1,2,4,...,128)	*/
    unsigned short res_sects;  /* Reserved sectors at the beginning	*/
    unsigned char num_fats;    /* Number of FAT copies (1 or 2)	*/
    unsigned short root_entr;  /* Root directory entries		*/
    unsigned short total_sect; /* Total sectors (if less 64k)		*/
    unsigned char media_desc;  /* Media descriptor byte (F8h for HD)	*/
    unsigned short fat_size16; /* Sectors per fat			*/
    unsigned short track_size; /* Sectors per track			*/
    unsigned short num_sides;  /* Sides				*/
    unsigned long hid_sects;   /* Special hidden sectors		*/
    unsigned long big_total;   /* Big total number of sectors  	*/
    union {
        _Packed struct x_fat16 {
            unsigned short drive_num;  /* Drive number              */
            unsigned char ext_signat;  /* Extended Boot Record signature (29h)  */
            unsigned long serial_num;  /* Volume serial number          */
            unsigned char label[11];   /* Volume label              */
            unsigned char fs_id[8];    /* File system id            */
            unsigned char xcode[448];  /* Loader executable code        */
        } f16;
        _Packed struct x_fat32 {
            unsigned long fat_size;     /* Sectors per FAT (big) */
            unsigned short ext_flags;   /* active FAT etc.  */        
            unsigned char fs_ver_maj;   /* File System Version (major) */
            unsigned char fs_ver_min;  /* File System Version (minor) */       
            unsigned long root_clust;   /* First cluster in root */       
            unsigned short fs_info_sect_num; /* FS Sector number (1) */
            unsigned short bs_bak_sect; /* Boot sector backup (6) */        
            unsigned char _resrvd1[12]; /* Reserved */
            unsigned char drive_num;    /* Physical drive number (80h) */
            unsigned char _resrvd2;     /* Reserved */        
            unsigned char ext_signat;   /* Extended Boot Record signature (29h) */
            unsigned long serial_num;   /* Volume serial number */
            unsigned char label[11];    /* Volume label */
            unsigned char fs_id[8];     /* File system id ("FAT32   ") */
            unsigned char xcode[420];   /* Boot loader code (first part) */
        } f32;
    } x;
    unsigned short magic_num;  /* Magic number (Must be 0xAA55)     */
};

/* sectors 3 and 3 of FAT-32 extended boot record */
_Packed struct fat32_ext_bootrec {
    unsigned long lead_sig;
    char _resrvd1[480];
    unsigned long struc_sig;
    unsigned long free_cluster_count;
    unsigned long next_free_cluster;
    char _resrvd2[12];
    unsigned long trail_sig;
    char _resrvd3[508];
    unsigned long trail2_sig;
};


_Packed struct boot_fat32
{
    /* Sector 1 */

    unsigned char jmp[3];    /* Must be 0xEB, 0x58, 0x90 = jmp 5A	*/
    unsigned char sys_id[8]; /* Probably:   "MSWIN4.1"		*/

    unsigned short sect_size; /* Sector size in bytes (512)		*/
    unsigned char clust_size; /* Sectors per cluster (1,2,4,...,128)	*/
    unsigned short res_sects; /* Reserved sectors at the beginning (32)*/
    unsigned char fat_copies; /* Number of FAT copies (2)		*/

    unsigned char resrvd1[4]; /* Reserved				*/
    unsigned char media_desc; /* Media descriptor byte (F8h)		*/
    unsigned short sfat_size; /* Sectors per FAT			*/

    unsigned short track_size; /* Sectors per track			*/
    unsigned short num_sides;  /* Sides				*/

    unsigned long hid_sects; /* Special hidden sectors		*/
    unsigned long num_sects; /* Big total number of sectors  	*/
    unsigned long fat_size;  /* Sectors per FAT (big)		*/
    unsigned short fat_attr;  /* FAT attributes (I guess)		*/

    unsigned char fs_ver_maj;  /* File System Version (major)		*/
    unsigned char fs_ver_min; /* File System Version (minor)		*/

    unsigned long root_clust; /* First cluster in root		*/

    unsigned short fs_info_sect_num; /* FS Sector number (1) ???		*/
    unsigned short bs_bak_sect; /* Boot sector backup (6)		*/

    unsigned char resrvd2[12]; /* Reserved				*/
    unsigned char drive_num;   /* Physical drive number (80h)		*/
    unsigned char resrvd3[1];  /* Reserved				*/

    unsigned char ext_signat; /* Extended Boot Record signature (29h)	*/
    unsigned long serial_num; /* Volume serial number			*/
    unsigned char label[11];  /* Volume label				*/
    unsigned char fs_id[8];   /* File system id ("FAT32   ")		*/
    unsigned char xcode[420]; /* Boot loader code (first part)	*/
    unsigned short magic_num;  /* Magic number (Must be 0xAA550000) 	*/

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


_Packed struct dirent
{
    unsigned char name[11];
    unsigned char attr;
    unsigned char nt_res;
    unsigned char crt_time_tenth;
    unsigned int  crt_time;
    unsigned int  crt_date;
    unsigned int  lst_acc_date;
    unsigned int  fst_clus_hi;
    unsigned int  wrt_time;
    unsigned int  wrt_date;
    unsigned int  fst_clus_lo;
    unsigned long file_size;
};

static const unsigned DIRENT_PER_SECT = 512 / sizeof(struct dirent);

enum {
	DIRENT_ATTR_READ_ONLY = 0x01,
	DIRENT_ATTR_HIDDEN = 0x02,
	DIRENT_ATTR_SYSTEM = 0x04,
    DIRENT_ATTR_VOL = 0x08,
    DIRENT_ATTR_DIR = 0x10,
    DIRENT_ATTR_ARCHIVE = 0x20
};

static const unsigned char  DIRENT_LONG_NAME_MASK = 
    DIRENT_ATTR_READ_ONLY | DIRENT_ATTR_HIDDEN | DIRENT_ATTR_SYSTEM |DIRENT_ATTR_VOL;

#define NO_NAME_LABEL "NO NAME    "

/* defined in S_FAT32.C */
unsigned long fat_calc_hidden_sectors(struct part_long *p);


#endif
