#include "allocs.h"
#include "diskio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OK     0
#define FAILED -1
#define CANCEL -2

struct disk_info dinfo;

unsigned long force_num_cyls = 0;
unsigned short force_num_heads = 0;
unsigned short force_num_sects = 0;
unsigned long reported_total_sectors = 0;

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl _diskio_init( void );
void __cdecl _diskio_exit( void );

int __cdecl _get_disk_info( int hd, struct disk_info *, char *buf_4096 );

int __cdecl disk_op_lba( struct disk_addr *, void *, int num_sect,
                         int operation );

int __cdecl disk_read_chs( struct disk_addr_chs *, void *, int num_sect );
int __cdecl disk_write_chs( struct disk_addr_chs *, void *, int num_sect );
int __cdecl disk_format_chs( struct disk_addr_chs *, void *ftable );
int __cdecl disk_verify_chs( struct disk_addr_chs *, void *, int num_sect );

#ifdef __cplusplus
}
#endif

#ifdef DEBUG_DISKIO
static FILE *f_diskio;
#endif

void diskio_init( void )
{
   _diskio_init();
#ifdef DEBUG_DISKIO
   f_diskio = fopen( "diskdbg.txt", "w" );
#endif
}

void diskio_exit( void )
{
#ifdef DEBUG_DISKIO
   if ( f_diskio ) {
      fclose( f_diskio );
   }
#endif
   _diskio_exit();
}

static void lba_to_chs( struct disk_addr *laddr, struct disk_addr_chs *chs )
{
   chs->disk = laddr->disk;
   chs->cyl = CYL( laddr->sect );
   chs->head = HEAD( laddr->sect );
   chs->sect = SECT( laddr->sect );
}

unsigned long test_sectors[] = { 0x10000000, 0xfffffff, 16450559, 16434494,
                                 0 };

int get_disk_info( int hd, struct disk_info *di, char *buf_4096 )
{
   int result, i;
   struct disk_addr addr;

   if ( ( result = _get_disk_info( hd, di, buf_4096 ) ) == -1 ) {
      return result;
   }
   reported_total_sectors = dinfo.total_sects;

   if ( di->lba && lba_enabled ) {
      /* comment out to force disk to 2TB size for testing */
      /* di->total_sects = 0xffffffff; */

      /* use BIOS assisted LBA setup */
      if ( ( di->num_heads <= 16 ) &&
           ( di->total_sects > 63ul * 16ul * 1024ul ) &&
           ( force_num_heads == 0 ) ) {
         /* calculate virtual CHS value */
         if ( di->total_sects > 63ul * 128ul * 1024ul ) {
            di->num_heads = 255;
         }
         else if ( di->total_sects > 63ul * 64ul * 1024ul ) {
            di->num_heads = 128;
         }
         else if ( di->total_sects > 63ul * 32ul * 1024ul ) {
            di->num_heads = 64;
         }
         else if ( di->total_sects > 63ul * 16ul * 1024ul ) {
            di->num_heads = 32;
         }
         else {
            di->num_heads = 16;
         }
      }
      
      addr.disk = dinfo.disk;
      addr.sect = dinfo.total_sects - 1;

      /* test if last sector is accessable */
      if ( disk_read( &addr, buf_4096, 1 ) != 0 ) {
         i = 0;
         while ( test_sectors[i] ) {
            addr.sect = test_sectors[i];
            if ( disk_read( &addr, buf_4096, 1 ) == 0 ) {
               di->total_sects = test_sectors[i] + 1;
               break;
            }
         }
         if ( test_sectors[i] == 0 ) {
            return -1;
         }
      }
   }

   if ( force_num_sects ) {
      di->num_sects = force_num_sects;
   }
   if ( force_num_heads ) {
      di->num_heads = force_num_heads;
   }
   if ( force_num_cyls ) {
      di->num_cyls = force_num_cyls;
   }

   if ( ( di->lba && lba_enabled ) || force_num_heads || force_num_sects ) {
      di->sect_per_cyl = di->num_heads * di->num_sects;
      if ( !force_num_cyls ) {
         /* recalculate cylinder count based on total sectors and
           sectors per cylinder */
         di->num_cyls = di->total_sects / di->sect_per_cyl;
         if ( di->total_sects % di->sect_per_cyl ) {
            di->num_cyls++;
         }
      }
      else {
         di->total_sects =
            ( (unsigned long)di->num_cyls ) * di->num_heads * di->num_sects;
         reported_total_sectors = di->total_sects;
      }
   }

   return result;
}

int disk_read( struct disk_addr *daddr, void *buf, int num_sect )
{
   struct disk_addr_chs chs;
   int result;

   /* uint32 overflow check */
   if ( daddr->sect + num_sect < daddr->sect ) {
      return FAILED;
   }

   if ( dinfo.lba ) {
      result = disk_op_lba( daddr, buf, num_sect, INT13_READ_EXT );
   }
   else {
      lba_to_chs( daddr, &chs );
      result = disk_read_chs( &chs, buf, num_sect );
   }

#ifdef DEBUG_DISKIO
   if ( f_diskio ) {
      fprintf( f_diskio, "r: %lu, len %d, res %d\n", daddr->sect, num_sect, result );
   }
#endif

   return result;
}

int disk_write( struct disk_addr *daddr, void *buf, int num_sect )
{
   struct disk_addr_chs chs;
   int result;

   /* uint32 overflow check */
   if ( daddr->sect + num_sect < daddr->sect ) {
      return FAILED;
   }

   if ( dinfo.lba ) {
      result = disk_op_lba( daddr, buf, num_sect, INT13_WRITE_EXT );
   }
   else {
      lba_to_chs( daddr, &chs );
      result = disk_write_chs( &chs, buf, num_sect );
   }

#ifdef DEBUG_DISKIO
   if ( f_diskio ) {
      fprintf( f_diskio, "W: %lu, len %d, res %d\n", daddr->sect, num_sect, result );
   }
#endif

   return result;
}

int disk_format( struct disk_addr *daddr, void *ftable )
{
   struct disk_addr_chs chs;

   lba_to_chs( daddr, &chs );

   return disk_format_chs( &chs, ftable );
}

int disk_verify( struct disk_addr *daddr, void *buf, int num_sect )
{
   struct disk_addr_chs chs;
   int result;

   /* uint32 overflow check */
   if ( daddr->sect + num_sect < daddr->sect ) {
      return FAILED;
   }

   if ( dinfo.lba ) {
      result = disk_op_lba( daddr, buf, num_sect, INT13_VERIFY_EXT );
   }
   else {
      lba_to_chs( daddr, &chs );
      result = disk_verify_chs( &chs, buf, num_sect );
   }

   return result;
}

/* verifies sectors
   calls callback *func for progress and *bbt_func for bad block tracking */
int disk_process_sectors( disk_operation op, struct disk_addr daddr,
                          unsigned long num_sect, unsigned char data,
                          int ( *func )( unsigned long, unsigned long ),
                          int ( *bbt_func )( unsigned long ) )
{
   char *buf;
   int sect_per_block;
   unsigned long curr_sect = 0;
   unsigned long start_sect = daddr.sect;
   unsigned long block_sect;
   int i, result = OK;

   sect_per_block = dinfo.num_sects;
   buf = malloc( sect_per_block * SECT_SIZE );

   /* if we can not allocate buffer for whole track process sectors one at a time */
   if ( !buf ) {
      buf = malloc( SECT_SIZE );
      if ( !buf ) {
         return FAILED;
      }
      else {
         sect_per_block = 1;
      }
   }

   if ( op == disk_write ) {
      memset( buf, data, sect_per_block * SECT_SIZE );
   }

   if ( sect_per_block > 1 ) {
      block_sect =
         sect_per_block - ( start_sect + curr_sect ) % sect_per_block;
      while ( curr_sect < num_sect ) {

         /* align to multiple of sectors per block (track) */
         if ( curr_sect + block_sect > num_sect ) {
            block_sect = num_sect - curr_sect;
         }

         if ( ( *op )( &daddr, buf, block_sect ) < 0 ) {
            for ( i = 0; i < block_sect; i++ ) {
               if ( ( *op )( &daddr, buf, 1 ) < 0 ) {
                  if ( bbt_func ) {
                     if ( !( *bbt_func )( daddr.sect ) ) {
                        result = FAILED;
                        goto done;
                     }
                  }
                  else {
                     result = FAILED;
                     goto done;
                  }
               }
               daddr.sect += 1;
            }
         }
         else {
            daddr.sect += block_sect;
         }
         curr_sect += block_sect;
         block_sect = sect_per_block;

         if ( func ) {
            if ( ( *func )( curr_sect, num_sect ) == 0 ) {
               result = CANCEL;
               goto done;
            }
         }
      }
   }

   /* test all or (on error) remaining sectors one sector at a time */
   while ( curr_sect < num_sect ) {
      daddr.sect = start_sect + curr_sect;
      if ( ( *op )( &daddr, buf, 1 ) < 0 ) {
         if ( bbt_func ) {
            if ( !( *bbt_func )( daddr.sect ) ) {
               result = FAILED;
               goto done;
            }
         }
         else {
            result = FAILED;
            goto done;
         }
      }

      curr_sect++;
      if ( func ) {
         if ( ( *func )( curr_sect, num_sect ) == 0 ) {
            result = CANCEL;
            goto done;
         }
      }
   }

done:
   free( buf );

   return result;
}
