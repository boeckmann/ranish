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

int format_embr( struct part_long *p, char **argv, char **msg )
{
   struct mbr *mbr;
   (void)argv;

   if ( !( mbr = malloc( sizeof( struct mbr ) ) ) ) {
      return FAILED;
   }
   flush_caches();

   disk_lock( dinfo.disk );

   progress( "^Initializing Extended DOS partition ..." );

   progress( "Writing Extended Master Boot Record ..." );

   memset( mbr, 0, SECT_SIZE );
   memmove( mbr->x.std.code, EMP_IPL, EMP_IPL_SIZE );
   strncpy( mbr->x.std.code + EMP_IPL_SIZE, MESG_EXT_NONBOOT,
            sizeof( mbr->x.std.code ) - EMP_IPL_SIZE );
   mbr->magic_num = MBR_MAGIC_NUM;

   if ( disk_write_rel( p, 0, mbr, 1 ) == FAILED ) {
      *msg = TEXT( "error writing Extended MBR" );
      disk_unlock( dinfo.disk );
      free( mbr );
      return FAILED;
   }

   disk_unlock( dinfo.disk );
   free( mbr );
   return OK;
} /* format_embr */

int generic_verify( struct part_long *p, int bbt_size, unsigned long *bbt )
{
   char *z, tmp[90];
   struct disk_addr daddr;
   unsigned short percent;
   unsigned long total_done = 0;
   unsigned long curr_sect, end_sect, sect_count;
   unsigned long sect;
   unsigned num_bad = 0;

   if ( ( z = (char *)malloc( 63 * SECT_SIZE ) ) == 0 ) {
      show_error( ERROR_MALLOC );
      return FAILED;
   }
   memset( z, 0, 63 * SECT_SIZE );

   progress( MESG_VERIFYING );

   disk_lock( hd );

   daddr.disk = dinfo.disk;
   curr_sect = QUICK_BASE( p );
   end_sect = curr_sect + p->num_sect - 1;

   /* we do not write across track boundaries, one track at a time */
   sect_count = dinfo.num_sects - ( curr_sect % dinfo.num_sects );
   while ( curr_sect <= end_sect ) {

      /* we do not want to go beyond end of partition */
      if ( end_sect - curr_sect + 1 < sect_count ) {
         sect_count = end_sect - curr_sect + 1;
      }

      daddr.sect = curr_sect;
      if ( disk_verify( &daddr, z, sect_count ) < 0 ) {

         /* if bad sectors detected inside track test which exactly
               are bad */
         for ( sect = curr_sect; sect <= end_sect; sect++ ) {
            daddr.sect = sect;

            if ( disk_verify( &daddr, z, 1 ) < 0 ) {
               if ( bbt_size != -1 ) {
                  if ( num_bad == bbt_size ) {
                     disk_unlock( hd );
                     free( z );
                     return FAILED;
                  }
                  bbt[num_bad] = sect;
               }
               num_bad++;
            }
         }
      }

      curr_sect += sect_count;
      total_done += sect_count;
      percent = total_done * 100ull / p->num_sect;

      sprintf( tmp, TEXT( "%% %3d%% verified" ), percent );
      if ( progress( tmp ) == CANCEL ) {
         disk_unlock( hd );
         free( z );
         return CANCEL;
      }

      sect_count = dinfo.num_sects; /* next track */
   }

   disk_unlock( hd );
   free( z );
   return num_bad;

} /* generic_verify */

/*
   Some BIOSes have problems with mapping logical cylinders into physical
on the large hard disks. If you use destructive format on such systems it
may cause corruption of the several sectors at the beginning of the next
partition. To avoid it generic_format will not format first and last side
of the partition, but will verify it and clear with zeros.

*/

int generic_format( struct part_long *p, int bbt_size, unsigned long *bbt )
{
   char *z, *z2, tmp[90];
   struct disk_addr daddr;
   unsigned short percent;
   unsigned long total_done = 0;
   unsigned long curr_sect, end_sect, sect_count;
   unsigned long sect;
   unsigned num_bad = 0;

   progress( MESG_FORMATTING );

   /*if (p->start_sect != 1 || p->end_sect != dinfo.num_sects) {
        progress(ERROR_FORMAT_FRACTION);
        return FAILED;
    }

    if (detected_os == SYS_WIN95) {
        progress(ERROR_FORMAT_WIN95);
        return FAILED;
    }*/

   if ( ( z = (char *)malloc( 63 * SECT_SIZE ) ) == 0 ) {
      show_error( ERROR_MALLOC );
      return FAILED;
   }
   memset( z, 0, 63 * SECT_SIZE );

   if ( ( z2 = (char *)malloc( 63 * SECT_SIZE ) ) == 0 ) {
      show_error( ERROR_MALLOC );
      free( z );
      return FAILED;
   }

   disk_lock( hd );

   daddr.disk = dinfo.disk;
   curr_sect = QUICK_BASE( p );
   end_sect = curr_sect + p->num_sect - 1;

   /* we do not write across track boundaries, one track at a time */
   sect_count = dinfo.num_sects - ( curr_sect % dinfo.num_sects );
   while ( curr_sect <= end_sect ) {

      /* we do not want to go beyond end of partition */
      if ( end_sect - curr_sect + 1 < sect_count ) {
         sect_count = end_sect - curr_sect + 1;
      }

      daddr.sect = curr_sect;

      if ( !( disk_write( &daddr, z, sect_count ) >= 0 &&
              disk_verify( &daddr, z2, sect_count ) >= 0 ) ) {

         /* if bad sectors detected inside track test which exactly
               are bad */
         for ( sect = curr_sect; sect <= end_sect; sect++ ) {
            daddr.sect = sect;

            if ( disk_verify( &daddr, z, 1 ) < 0 ) {
               if ( bbt_size != -1 ) {
                  if ( num_bad == bbt_size ) {
                     disk_unlock( hd );
                     free( z );
                     free( z2 );
                     return FAILED;
                  }
                  bbt[num_bad] = sect;
               }
               num_bad++;
            }
         }
      }

      curr_sect += sect_count;
      total_done += sect_count;
      percent = total_done * 100ull / p->num_sect;

      sprintf( tmp, TEXT( "%% %3d%% formatted" ), percent );
      if ( progress( tmp ) == CANCEL ) {
         disk_unlock( hd );
         free( z );
         free( z2 );
         return CANCEL;
      }

      sect_count = dinfo.num_sects; /* next track */
   }

   disk_unlock( hd );
   free( z );
   free( z2 );
   return num_bad;
} /* generic_format */

int generic_clean( struct part_long *p )
{
   char *z, tmp[90];
   struct disk_addr daddr;
   unsigned short percent;
   unsigned long total_done = 0;
   unsigned long curr_sect, end_sect, sect_count;

   if ( ( z = (char *)malloc( 63 * SECT_SIZE ) ) == 0 ) {
      show_error( ERROR_MALLOC );
      return FAILED;
   }
   memset( z, 0, 63 * SECT_SIZE );

   progress( MESG_CLEANING );

   disk_lock( hd );

   daddr.disk = dinfo.disk;
   curr_sect = QUICK_BASE( p );
   end_sect = curr_sect + p->num_sect - 1;

   /* we do not write across track boundaries, one track at a time */
   sect_count = dinfo.num_sects - ( curr_sect % dinfo.num_sects );
   while ( curr_sect <= end_sect ) {

      /* we do not want to go beyond end of partition */
      if ( end_sect - curr_sect + 1 < sect_count ) {
         sect_count = end_sect - curr_sect + 1;
      }

      daddr.sect = curr_sect;
      if ( disk_write( &daddr, z, sect_count ) < 0 ) {
         show_error( TEXT( "disk_write returned failure" ) );
         disk_unlock( hd );
         free( z );
         return FAILED;
      }

      curr_sect += sect_count;
      total_done += sect_count;
      percent = total_done * 100ull / p->num_sect;

      sprintf( tmp, TEXT( "%% %3d%% cleaned" ), percent );
      if ( progress( tmp ) == CANCEL ) {
         disk_unlock( hd );
         free( z );
         return CANCEL;
      }

      sect_count = dinfo.num_sects; /* next track */
   }

   disk_unlock( hd );
   free( z );
   return OK;

} /* generic_clean */
