#ifndef _DISKIO_H
#define _DISKIO_H


struct disk_addr
    {
     unsigned short disk;
     unsigned short cyl;
     unsigned short head;
     unsigned short sect;
    };

struct disk_info
    {
     unsigned short disk;
     unsigned short num_cyls;
     unsigned short num_heads;
     unsigned short num_sects;
     unsigned long  total_sects;
     unsigned short sect_per_cyl;
     unsigned short sect_per_track;
     unsigned short sect_size;
     unsigned short bios_num_cyls;
    };

extern unsigned short int const detected_os;

#define SYS_DOS		0	/* MS-DOS v1.00 - v4.xx 	*/
#define SYS_DOS5	1	/* MS-DOS v5.00 - v6.xx 	*/
#define SYS_DOS7	2	/* Comand line mode of Win95	*/
#define SYS_WIN3	3	/* GUI mode of Windows 3.xx	*/
#define SYS_WIN95	4	/* GUI mode of Windows 95	*/
#define SYS_WIN_NT	5	/* Windows NT 			*/



#ifdef __cplusplus
extern "C" {
#endif


void __cdecl diskio_init(void);
void __cdecl diskio_exit(void);

void __cdecl reboot(void);
int __cdecl flush_caches(void);

int __cdecl get_disk_info(int hd, struct disk_info *, char *buf_4096);

int __cdecl disk_lock(int hd);
int __cdecl disk_unlock(int hd);

int __cdecl dos_drive_reset(int drive);

int __cdecl disk_read(struct disk_addr *, void *, int num_sect);
int __cdecl disk_write(struct disk_addr *, void *, int num_sect);
int __cdecl disk_format(struct disk_addr *, void *ftable);
int __cdecl disk_verify(struct disk_addr *, int num_sect);


#ifdef __cplusplus
}
#endif

#endif
