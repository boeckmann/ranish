#ifndef _DISKIO_H
#define _DISKIO_H

#define SYS_DOS    0
#define SYS_DOS5   1
#define SYS_DOS7   2
#define SYS_WIN3   3
#define SYS_WIN9X  4
#define SYS_WIN_NT 5

#define SECT_SIZE 512 /* Must not be changed */

struct disk_addr_chs {
   unsigned short disk;
   unsigned short cyl;
   unsigned short head;
   unsigned short sect;
};

struct disk_addr {
   unsigned short disk;
   unsigned long sect;
};

struct disk_info {
   unsigned short disk;
   unsigned long num_cyls;
   unsigned short num_heads;
   unsigned short num_sects;
   unsigned long total_sects;
   unsigned short sect_per_cyl;
   unsigned short sect_per_track;
   unsigned short sect_size;
   unsigned short bios_num_cyls;
   unsigned short lba;
};

extern struct disk_info dinfo;
extern unsigned long force_num_cyls;
extern unsigned short force_num_heads;
extern unsigned short force_num_sects;
extern unsigned long reported_total_sectors;

extern int lba_enabled; /* on=1 by default, set 0 th disable lba access */

extern unsigned short detected_os;

#define SYS_DOS    0 /* MS-DOS v1.00 - v4.xx 	*/
#define SYS_DOS5   1 /* MS-DOS v5.00 - v6.xx 	*/
#define SYS_DOS7   2 /* Comand line mode of Win95	*/
#define SYS_WIN3   3 /* GUI mode of Windows 3.xx	*/
#define SYS_WIN95  4 /* GUI mode of Windows 95	*/
#define SYS_WIN_NT 5 /* Windows NT 			*/

#define CYL( abs_sect ) ( ( abs_sect ) / ( dinfo.sect_per_cyl ) )
#define HEAD( abs_sect )                                                     \
   ( ( abs_sect ) % ( dinfo.sect_per_cyl ) / ( dinfo.num_sects ) )
#define SECT( abs_sect )                                                     \
   ( ( abs_sect ) % ( dinfo.sect_per_cyl ) % ( dinfo.num_sects ) + 1 )

#define ABS_REL_SECT( p )                                                    \
   ( ( p )->start_cyl * dinfo.sect_per_cyl +                                 \
     ( p )->start_head * dinfo.sect_per_track + ( p )->start_sect - 1 )

#define ABS_END_SECT( p )                                                    \
   ( ( p )->end_cyl * dinfo.sect_per_cyl +                                   \
     ( p )->end_head * dinfo.sect_per_track + ( p )->end_sect - 1 )

#define ABS_END_SECT_MINUS_1( p )                                            \
   ( ( ( p )->end_cyl - 1 ) * dinfo.sect_per_cyl +                           \
     ( p )->end_head * dinfo.sect_per_track + ( p )->end_sect - 1 )

#define REL_SECT( p ) ( ABS_REL_SECT( p ) - ( p )->container_base )
#define NUM_SECT( p ) ( ABS_END_SECT( p ) - ABS_REL_SECT( p ) + 1 )

#define NUM_SECT_RA( p, r ) ( ABS_END_SECT( p ) - ( r ) + 1 )
#define NUM_SECT_RR( p, r )                                                  \
   ( ABS_END_SECT( p ) - ( r ) + 1 - ( p )->container_base )

#define QUICK_BASE( p )                                                      \
   ( ( mode == MODE_LBA || ( p )->valid )                                    \
        ? ( ( p )->container_base + ( p )->rel_sect )                        \
        : ( ABS_REL_SECT( p ) ) )

#define QUICK_END( p )                                                       \
   ( ( mode == MODE_LBA || ( p )->valid )                                    \
        ? ( ( p )->container_base + ( p )->rel_sect + ( p )->num_sect - 1 )  \
        : ( ABS_END_SECT( p ) ) )

#define QUICK_SIZE( p )                                                      \
   ( ( mode == MODE_LBA || ( p )->valid )                                    \
        ? ( ( p )->num_sect )                                                \
        : ( ABS_END_SECT( p ) - ABS_REL_SECT( p ) + 1 ) )

#define INT13_READ_EXT   0x4200
#define INT13_WRITE_EXT  0x4300
#define INT13_VERIFY_EXT 0x4400

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl reboot( void );
int __cdecl flush_caches( void );

int __cdecl disk_lock( int hd );
int __cdecl disk_unlock( int hd );

int __cdecl dos_drive_reset( int drive );

#ifdef __cplusplus
}
#endif

extern unsigned char diskio_errno;

int get_disk_info( int hd, struct disk_info *, char *buf_4096 );

#define DISK_OP_WRITE  1
#define DISK_OP_VERIFY 2

typedef int ( *disk_operation )( struct disk_addr *, void *, int num_sect );

void diskio_init( void );
void diskio_exit( void );

/* result < 0 if error occured */
int disk_read( struct disk_addr *, void *, int num_sect );
int disk_write( struct disk_addr *, void *, int num_sect );

int disk_format( struct disk_addr *, void *ftable );
int disk_verify( struct disk_addr *, void *, int num_sect );

int disk_process_sectors( disk_operation op, struct disk_addr daddr,
                          unsigned long num_sect, unsigned char data,
                          int ( *func )( unsigned long, unsigned long ),
                          int ( *bbt_func )( unsigned long ) );

#endif
