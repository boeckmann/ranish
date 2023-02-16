#include "cache.h"
#include "dosext.h"
#include "fat.h"
#include "part.h"
#include <ctype.h>
#include <string.h>
#include <time.h>

#define MAX_BAD_SECTORS 512

/* fat bad block tracking data */
static struct part_long *bbt_part;
static struct boot_ms_dos *bbt_boot;
static unsigned long fat_bad_sectors;

/* temp buffer for constructing result messages */
static char fat_format_msg[80];

/* status call-back for multi-sector disk routines */
/* called when filling data area with zeros */
int write_progress( unsigned long curr, unsigned long total )
{
   char buf[32];
   static unsigned long last = 0;
   unsigned int t;

   if ( curr - last > 16 * 63 ) {
      t = (unsigned long long)curr * 100 / (unsigned long long)total;
      sprintf( buf, "%% %3u%% | %lu errors", t, fat_bad_sectors );
      last = curr;
      return progress( buf ) != CANCEL;
   }

   return 1;
}

/* status call-back for multi-sector disk routines */
/* called when doing bad block tracking */
int verify_progress( unsigned long curr, unsigned long total )
{
   char buf[32];
   static unsigned long last = 0;
   unsigned int t;

   if ( curr - last > 16 * 63 ) {
      t = (unsigned long long)curr * 100 / (unsigned long long)total;
      sprintf( buf, "%% %3u%% | %lu errors", t, fat_bad_sectors );
      last = curr;
      return progress( buf ) != CANCEL;
   }

   return 1;
}

/* read error call-back for bad block tracking */
/* marks the clusters containing the bad physical sectors as bad */
/* has to translate physical sector numbers to partition-relative */
int fat_bbt_track( unsigned long phys_sector )
{
   int fat;
   fat_bad_sectors++;

   for ( fat = 0; fat < bbt_boot->num_fats; fat++ ) {
      fat_set_table_entry(
         bbt_part, bbt_boot, fat,
         fat_sector_to_cluster( bbt_boot, phys_sector -
                                             bbt_part->container_base -
                                             bbt_part->rel_sect ),
         FAT32_INV_CLUSTER );
   }
   return fat_bad_sectors < MAX_BAD_SECTORS;
}

/* formats a FAT-12/16/32 partition */
int format_fat( struct part_long *p, char **argv, char **msg )
{
   int result;
   char *data_pool;
   struct boot_ms_dos *b;
   struct fat32_ext_bootrec *eb;
   unsigned long sys_type;
   int logical_drive = 0;

   char label[FAT_LABEL_LEN + 1] = "";
   unsigned int form_type = F_VERIFY;
   fat_bad_sectors = 0;

   if ( ( data_pool = malloc( sizeof( struct boot_ms_dos ) +
                              sizeof( struct fat32_ext_bootrec ) ) ) == 0 ) {
      show_error( ERROR_MALLOC );
      return FAILED;
   }

   b = (struct boot_ms_dos *)data_pool;
   eb = (struct fat32_ext_bootrec *)( data_pool +
                                      sizeof( struct boot_ms_dos ) );

   /* parse arguments */
   while ( *argv != 0 ) {
      if ( _stricmp( *argv, "/destructive" ) == 0 ) {
         form_type = F_DESTR;
      }
      else if ( _stricmp( *argv, "/verify" ) == 0 ) {
         form_type = F_VERIFY;
      }
      else if ( _strnicmp( *argv, "/l:", 3 ) == 0 ) {
         strncpy( label, ( *argv ) + 3, FAT_LABEL_LEN );
         label[FAT_LABEL_LEN] = 0;
      }
      else {
         snprintf( fat_format_msg, sizeof( fat_format_msg ) - 1,
                   TEXT( "unknown option: %s" ), *argv );
         *msg = fat_format_msg;
         result = FAILED;
         goto done;
      }
      argv++;
   }

   flush_caches();
   if ( disk_lock( dinfo.disk ) != OK ) {
      *msg = "could not lock disk";
      result = FAILED;
      goto done;
   }

   /* set FAT type */
   switch ( p->os_id ) {
   case 0x0b00:
   case 0x0c00:
   case 0x1b00:
   case 0x1c00:
      sys_type = FAT_32;
      break;
   case 0x0400:
   case 0x0600:
   case 0x0e00:
   case 0x1400:
   case 0x1600:
   case 0x1e00:
      sys_type = FAT_16;
      break;
   case 0x0100:
   case 0x1100:
      sys_type = FAT_12;
      break;
   }

   if ( sys_type == FAT_32 ) {
      result = fat_initialize_bootrec( p, b, 512, FAT_32, label );

      if ( result == FAT_ERR_TOO_SMALL ) {
         *msg = TEXT( "^FAT-32 partition too small. Use FAT-16 instead!" );
         result = FAILED;
         goto done;
      }

      fat32_initialize_ext_bootrec( b, eb );
      logical_drive = fat32_find_logical_drive( p );
   }
   else if ( sys_type == FAT_16 ) {
      result = fat_initialize_bootrec( p, b, 512, FAT_16, label );

      if ( result == FAT_ERR_TOO_SMALL ) {
         *msg = TEXT( "FAT-16 partition too small. Use FAT-12 instead!" );
         result = FAILED;
         goto done;
      }
      if ( result == FAT_ERR_TOO_LARGE ) {
         *msg = TEXT( "FAT-16 partition too large. Use FAT-32 instead!" );
         result = FAILED;
         goto done;
      }
   }
   else if ( sys_type == FAT_12 ) {
      result = fat_initialize_bootrec( p, b, 224, FAT_12, label );

      if ( result == FAT_ERR_TOO_SMALL ) {
         *msg = TEXT( "FAT-12 partition too small!" );
         result = FAILED;
         goto done;
      }
      if ( result == FAT_ERR_TOO_LARGE ) {
         *msg = TEXT(
            "FAT-12 partition too large. Use FAT-16 or FAT-32 instead!" );
         result = FAILED;
         goto done;
      }
   }

   progress( "^Format: writing boot sector ..." );
   if ( sys_type == FAT_32 ) {
      result = disk_write_rel( p, 0, b, 3 );
      result |= disk_write_rel( p, FAT32_BACKUP_SECTOR, b, 3 );
   }
   else {
      result = disk_write_rel( p, 0, b, 1 );
   }
   if ( result != OK ) {
      *msg = TEXT( "error writing boot sector." );
      goto done;
   }

   result = fat_initialize_tables( p, b, write_progress );

   if ( result != OK ) {
      *msg = TEXT( "error writing FAT" );
      goto done;
   }

   progress( "^Format: writing root directory ..." );
   result = fat_initialize_root( p, b );
   if ( result != OK ) {
      *msg = TEXT( "error writing root directory" );
      goto done;
   }

   bbt_part = p;
   bbt_boot = b;

   if ( form_type == F_VERIFY ) {
      progress(
         "^Format: searching for bad sectors ...   you may skip this step via ESC" );
      result = part_verify_sectors( p, fat_non_data_sector_count( b ),
                                    fat_sector_count( b ) -
                                       fat_non_data_sector_count( b ),
                                    verify_progress, fat_bbt_track );

      if ( result == FAILED ) {
         if ( fat_bad_sectors >= MAX_BAD_SECTORS ) {
            *msg = TEXT( "too many bad clusters found" );
         }
         else {
            *msg = TEXT( "could not verify sectors" );
         }
         goto done;
      }
   }
   else if ( form_type == F_DESTR ) {
      progress(
         "^Format: cleaning data area...           you may skip this step via ESC" );
      result = part_fill_sectors( p, fat_non_data_sector_count( b ),
                                  fat_sector_count( b ) -
                                     fat_non_data_sector_count( b ),
                                  0, write_progress, fat_bbt_track );
      if ( result == FAILED ) {
         *msg = TEXT( "error cleaning data area" );
         goto done;
      }
   }

   progress( "^Format: updating volume label ..." );
   result = fat_update_label_file( p, b );
   if ( result != OK ) {
      *msg = TEXT( "error updating volume label" );
      goto done;
   }

   if ( fat_bad_sectors ) {
      *msg = TEXT( "bad clusters found" );
      result = WARN;
   }

   /* flush FAT cache to disk */
   if ( result == OK ) {
      result = fat_flush();
   }
   else {
      fat_flush();
   }

   /* inform Win9x about formated drive */
   if ( sys_type == FAT_32 && detected_os == SYS_WIN9X && logical_drive ) {
      dos_set_fsinfo( logical_drive, eb->free_cluster_count,
                      eb->next_free_cluster );
      dos_force_media_change( logical_drive );
   }

done:
   disk_unlock( dinfo.disk );
   free( data_pool );

   return result;

} /* format_fat */

int print_fat( struct part_long *p )
{
   struct boot_ms_dos *b = (struct boot_ms_dos *)tmp;
   char tmp1[20], tmp2[20];

   if ( disk_read_rel( p, 0, b, 1 ) == -1 ) {
      fprintf( stderr, "Error reading boot sector.\n" );
      return FAILED;
   }

   printf( "\n              DOS Boot Sector Data              Expected "
           "Value\n\n" );
   printf( "                        System id:  %-.8s\n", b->sys_id );
   printf( "                      Sector size:  %-3d         512\n",
           b->sect_size );
   printf( "              Sectors per cluster:  %d\n", b->clust_size );
   printf( "Reserved sectors at the beginning:  %d\n", b->res_sects );
   printf( "             Number of FAT copies:  %d\n", b->num_fats );
   printf( "           Root directory entries:  %d\n", b->root_entr );
   printf( "            Total sectors on disk:  %-5u       %u\n",
           b->total_sect,
           ( b->total_sect == 0 || p->num_sect > 65535L ) ? 0 : p->num_sect );
   printf( "            Media descriptor byte:  %02Xh         F8h\n",
           b->media_desc );
   printf( "                  Sectors per FAT:  %d\n", b->fat_size16 );
   printf( "                 Sectors per side:  %-2d          %d\n",
           b->track_size, dinfo.num_sects );
   printf( "                  Number of sides:  %-3d         %d\n",
           b->num_sides, dinfo.num_heads );
   printf( "Hidden sectors prior to partition:  %-10s  %-10s\n",
           sprintf_long( tmp1, b->hid_sects ),
           sprintf_long( tmp2, fat_calc_hidden_sectors( p ) ) );
   printf( "      Big total number of sectors:  %-10s  %-10s\n",
           b->big_total == 0 ? "0" : sprintf_long( tmp1, b->big_total ),
           ( b->total_sect == 0 || p->num_sect > 65535L )
              ? sprintf_long( tmp2, p->num_sect )
              : "0" );
   printf( "                     Drive number:  %-3d         %d\n",
           b->x.f16.drive_num, dinfo.disk );
   printf( "   Extended boot record signature:  %02Xh\n",
           b->x.f16.ext_signat );
   printf( "             Volume serial number:  %08lX\n",
           b->x.f16.serial_num );
   printf( "                     Volume label:  %-.11s\n", b->x.f16.label );
   printf( "                 File system type:  %-.8s    %s\n",
           b->x.f16.fs_id,
           ( b->clust_size == 0 )
              ? ( "?" )
              : ( p->num_sect / b->clust_size < 4096 ? "FAT12" : "FAT16" ) );
   printf( "         Boot sector magic number:  0%04Xh      0AA55h\n",
           b->magic_num );
   return 0;
} /* print_fat */

#define StX 5
#define StY 5
#define StW 66
#define StH 14

#define StX2 ( StX + 30 )
#define StX3 ( StX + 69 )

#define TEXT_COLOR ( BrCyan + BakBlue )
#define DATA_COLOR ( BrWhite + BakBlue )
#define EDIT_COLOR ( BrWhite + BakBlack )

int setup_fat( struct part_long *p )
{
   struct event ev;
   int i, act, pos;
   char *tmp, *tmp1;
   unsigned long n, min_clust, max_clust, min_num_sect, max_num_sect, l, lc;
   unsigned fatsz, syst;
   unsigned long num_sect;

   struct boot_ms_dos *b, *b_orig,
      *fat_boot_code = (struct boot_ms_dos *)FAT_BOOT;

   if ( ( tmp = malloc( 3 * SECT_SIZE ) ) == 0 ) {
      show_error( ERROR_MALLOC );
      return FAILED;
   }

   tmp1 = tmp + 100;
   /* tmp2=tmp+120; */
   b = (struct boot_ms_dos *)( tmp + SECT_SIZE );
   b_orig = b + 1;

   if ( disk_read_rel( p, 0, b, 1 ) == -1 ) {
      show_error( "Error reading boot sector" );
      free( tmp );
      return FAILED;
   }

   if ( ( b->clust_size < 1 ) || ( b->clust_size > 128 ) ||
        ( b->sect_size != 512 ) || ( b->res_sects == 0 ) ) {
      show_error( "Partition seems not to be FAT formatted" );
      free( tmp );
      return FAILED;
   }

   memmove( b_orig, b, SECT_SIZE );

   clear_window( TEXT_COLOR, 2, 5, 78, 19 );
   write_string( HINT_COLOR, 45, 24, "         ESC" );
   write_string( MENU_COLOR, 57, 24, " - Return to MBR view" );

   write_string( HINT_COLOR, 64, 13, "F5" );
   write_string( MENU_COLOR, 66, 13, " - SetExpVal" );
   write_string( HINT_COLOR, 64, 14, "F6" );
   write_string( MENU_COLOR, 66, 14, " - FixBoot2G" );

   syst = 0;
   if ( strncmp( b->x.f16.fs_id, "FAT12", 5 ) == 0 ) {
      syst = 12;
   }
   if ( strncmp( b->x.f16.fs_id, "FAT16", 5 ) == 0 ) {
      syst = 16;
   }

   max_clust = ( syst == 0 )
                  ? ( 0 )
                  : ( (long)SECT_SIZE * 8u * b->fat_size16 / syst - 2u );

   if ( syst == 12 && max_clust > FAT12_MAX_DATA_CLUST ) {
      max_clust = FAT12_MAX_DATA_CLUST;
   }
   if ( syst == 16 && max_clust > FAT16_MAX_DATA_CLUST ) {
      max_clust = FAT16_MAX_DATA_CLUST;
   }

   write_string( TEXT_COLOR, StX, StY + 1, "                  System id:" );
   write_string( TEXT_COLOR, StX, StY + 2,
                 "           File system type:                             "
                 "Media desc:" );
   write_string( TEXT_COLOR, StX, StY + 3,
                 "        Sectors per cluster:                            "
                 "Sector size:" );
   write_string( TEXT_COLOR, StX, StY + 4,
                 "            Sectors per FAT:                             "
                 "FAT copies:" );
   write_string( TEXT_COLOR, StX, StY + 5,
                 "     Root directory entries:                               "
                 "Ext sign:" );
   write_string(
      TEXT_COLOR, StX, StY + 6,
      "       Volume serial number:                                "
      "  Magic:" );
   write_string(
      TEXT_COLOR, StX, StY + 7,
      "               Volume label:               Expected Value" );
   write_string( TEXT_COLOR, StX, StY + 8, "               Drive number:" );
   write_string( TEXT_COLOR, StX, StY + 9, "            Number of sides:" );
   write_string( TEXT_COLOR, StX, StY + 10, "           Sectors per side:" );
   write_string( TEXT_COLOR, StX, StY + 11,
                 " Sectors prior to partition:               123456789" );
   write_string( TEXT_COLOR, StX, StY + 12,
                 "    Total number of sectors:               12345" );
   write_string( TEXT_COLOR, StX, StY + 13,
                 "Big total number of sectors:               123456789" );
   write_string( TEXT_COLOR, StX, StY + 14, "         Number of clusters:" );

   write_string( TEXT_COLOR, StX, StY + 15,
                 "     Minimum partition size:  press    to calculate" );
   write_string( HINT_COLOR, StX + 36, StY + 15, "F7" );
   write_string( TEXT_COLOR, StX, StY + 16,
                 "     Current partition size:  ?" );
   write_string( TEXT_COLOR, StX, StY + 17,
                 "     Maximum partition size:  ?" );

   sprintf( tmp, "%-.8s", b->sys_id );
   write_string( DATA_COLOR, StX2, StY + 1, tmp );

   sprintf( tmp, "%-.8s", b->x.f16.fs_id );
   write_string( DATA_COLOR, StX2, StY + 2, tmp );

   sprintf( tmp, "%Xh", b->media_desc );
   write_string( DATA_COLOR, StX3, StY + 2, tmp );

   sprintf(
      tmp, "%-3u %s bytes/cluster", b->clust_size,
      sprintf_long( tmp1, (unsigned long)b->clust_size * b->sect_size ) );
   write_string( DATA_COLOR, StX2, StY + 3, tmp );

   sprintf( tmp, "%d", b->sect_size );
   write_string( DATA_COLOR, StX3, StY + 3, tmp );

   sprintf( tmp, "%-3d", b->fat_size16 );
   write_string( DATA_COLOR, StX2, StY + 4, tmp );

   sprintf( tmp, "%d", b->num_fats );
   write_string( DATA_COLOR, StX3, StY + 4, tmp );

   sprintf( tmp, "%d", b->root_entr );
   write_string( DATA_COLOR, StX2, StY + 5, tmp );

   sprintf( tmp, "%Xh", b->x.f16.ext_signat );
   write_string( DATA_COLOR, StX3, StY + 5, tmp );

   sprintf( tmp, "%08lX", b->x.f16.serial_num );
   write_string( DATA_COLOR, StX2, StY + 6, tmp );

   sprintf( tmp, " %u", dinfo.disk );
   write_string( DATA_COLOR, StX2 + 12, StY + 8, tmp );

   sprintf( tmp, " %u", dinfo.num_heads );
   write_string( DATA_COLOR, StX2 + 12, StY + 9, tmp );

   sprintf( tmp, " %u", dinfo.num_sects );
   write_string( DATA_COLOR, StX2 + 12, StY + 10, tmp );

   sprintf( tmp, " %-9lu", fat_calc_hidden_sectors( p ) );
   write_string( DATA_COLOR, StX2 + 12, StY + 11, tmp );

   sprintf( tmp, " %-5u", ( p->num_sect > 65535L ) ? 0 : p->num_sect );
   write_string( DATA_COLOR, StX2 + 12, StY + 12, tmp );

   sprintf( tmp, " %-9lu", ( p->num_sect > 65535L ) ? p->num_sect : 0 );
   write_string( DATA_COLOR, StX2 + 12, StY + 13, tmp );

   num_sect = fat_sector_count( b );

   n = fat_non_data_sector_count( b );
   max_num_sect = n + max_clust * b->clust_size;

   sprintf( tmp, "%lu sectors = %s kbytes", max_num_sect,
            sprintf_long( tmp1, ( max_num_sect ) / 2 ) );
   write_string( DATA_COLOR, StX2, StY + 17, tmp );

   n = num_sect;
   sprintf( tmp, "%lu sectors = %s kbytes", n, sprintf_long( tmp1, n / 2 ) );
   write_string( DATA_COLOR, StX2, StY + 16, tmp );

   pos = 0;
   act = 0;

   while ( 1 ) {
      for ( i = 0; i < 11; i++ ) {
         tmp[i] = ( b->x.f16.label[i] == 0 ) ? ' ' : b->x.f16.label[i];
      }
      tmp[i] = 0;
      write_string( DATA_COLOR, StX2, StY + 7, tmp );
      sprintf( tmp, "%-5d", b->x.f16.drive_num );
      write_string( ( b->x.f16.drive_num == dinfo.disk ) ? DATA_COLOR
                                                         : INVAL_COLOR,
                    StX2, StY + 8, tmp );
      sprintf( tmp, "%-5d", b->num_sides );
      write_string( ( b->num_sides == dinfo.num_heads ) ? DATA_COLOR
                                                        : INVAL_COLOR,
                    StX2, StY + 9, tmp );
      sprintf( tmp, "%-5d", b->track_size );
      write_string( ( b->track_size == dinfo.num_sects ) ? DATA_COLOR
                                                         : INVAL_COLOR,
                    StX2, StY + 10, tmp );
      sprintf( tmp, "%-9lu", b->hid_sects );
      write_string( ( b->hid_sects == fat_calc_hidden_sectors( p ) )
                       ? DATA_COLOR
                       : INVAL_COLOR,
                    StX2, StY + 11, tmp );
      sprintf( tmp, "%-6u", b->total_sect );
      write_string( ( b->total_sect == 0 || b->total_sect == p->num_sect )
                       ? DATA_COLOR
                       : INVAL_COLOR,
                    StX2, StY + 12, tmp );
      sprintf( tmp, "%-9lu", b->big_total );
      write_string( ( b->big_total == 0 || b->big_total == p->num_sect )
                       ? DATA_COLOR
                       : INVAL_COLOR,
                    StX2, StY + 13, tmp );
      sprintf( tmp, "%-11lu  max %-11lu",
               ( num_sect - fat_non_data_sector_count( b ) ) / b->clust_size,
               max_clust );

      write_string( DATA_COLOR, StX2, StY + 14, tmp );

      sprintf( tmp, "%04X", b->magic_num );
      write_string( ( b->magic_num == MBR_MAGIC_NUM ) ? DATA_COLOR
                                                      : INVAL_COLOR,
                    StX3, StY + 6, tmp );

      p->changed = ( memcmp( b, b_orig, SECT_SIZE ) == 0 ) ? 0 : 1;
      if ( p->changed == 0 ) {
         write_string( HINT_COLOR, 15, 24, "F2" );
      }
      if ( p->changed == 1 ) {
         write_string( Blink + HINT_COLOR, 15, 24, "F2" );
      }

      if ( act == 0 ) {
         memmove( tmp, b->x.f16.label, 11 );
         tmp[11] = 0;
         for ( i = 10; i >= 0 && tmp[i] == ' '; i-- ) {
            tmp[i] = 0;
         }
         memmove( tmp1, tmp, 12 );
         edit_str_field( &ev, 0, EDIT_COLOR, StX2, StY + 7, 12, tmp, &pos );
         if ( memcmp( tmp, tmp1, 12 ) != 0 ) {
            fat_canonicalize_label( tmp );
            memcpy( b->x.f16.label, tmp, FAT_LABEL_LEN );
         }
      }

      if ( act == 1 ) {
         n = b->x.f16.drive_num;
         edit_int_field( &ev, 0, EDIT_COLOR, StX2, StY + 8, -5, &n, 255L );
         b->x.f16.drive_num = n;
      }

      if ( act == 2 ) {
         n = b->num_sides;
         edit_int_field( &ev, 0, EDIT_COLOR, StX2, StY + 9, -5, &n, 255L );
         b->num_sides = n;
      }

      if ( act == 3 ) {
         n = b->track_size;
         edit_int_field( &ev, 0, EDIT_COLOR, StX2, StY + 10, -5, &n, 63L );
         b->track_size = n;
      }

      if ( act == 4 ) {
         edit_int_field( &ev, 0, EDIT_COLOR, StX2, StY + 11, -9,
                         &b->hid_sects, 999999999L );
      }
      if ( act == 5 ) {
         n = b->total_sect;
         edit_int_field( &ev, 0, EDIT_COLOR, StX2, StY + 12, -6, &n, 65535L );
         b->total_sect = n;
      }

      if ( act == 6 ) {
         edit_int_field( &ev, 0, EDIT_COLOR, StX2, StY + 13, -9,
                         &b->big_total, 999999999L );
      }
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
      p->changed = ( memcmp( b, b_orig, SECT_SIZE ) == 0 ) ? 0 : 1;
      if ( p->changed == 0 ) {
         write_string( HINT_COLOR, 15, 24, "F2" );
      }
      if ( p->changed == 1 ) {
         write_string( Blink + HINT_COLOR, 15, 24, "F2" );
      }

      if ( ev.key == 27 ) /* ESC */
      {
         if ( p->changed ) {
            continue;
         }
         break;
      }
      if ( ev.key == 13 || ev.key == 9 ) /* Enter or Tab */
      {
         act = ( act + 1 ) % 7;
      }
      else if ( ev.scan == 0x3B00 ) /* F1 - Help */
      {
         sprintf( tmp, "#setup_fat" );
         html_view( tmp );
      }
      else if ( ev.scan == 0x50E0 || ev.scan == 0x5000 ) /* down */
      {
         if ( act < 6 ) {
            act++;
         }
      }
      else if ( ev.scan == 0x48E0 || ev.scan == 0x4800 ) /* up */
      {
         if ( act > 0 ) {
            act--;
         }
      }
      else if ( ev.scan == 0x3C00 ) /* F2 - Save */
      {
         if ( p->inh_invalid || p->inh_changed ) {
            show_error( ERROR_INH_INVAL );
            continue;
         }

         flush_caches();

         disk_lock( dinfo.disk );

         if ( disk_write_rel( p, 0, b, 1 ) == FAILED ||
              fat_update_label_file( p, b ) == FAILED ) {
            show_error( "Error saving boot sector or volume label" );
         }
         else {
            memmove( b_orig, b, SECT_SIZE );
         }

         disk_unlock( dinfo.disk );
      }
      else if ( ev.scan == 0x3D00 ) /* F3 - Undo */
      {
         memmove( b, b_orig, SECT_SIZE );
      }
      else if ( ev.scan == 0x3F00 ) /* F5 - Set */
      {
         b->x.f16.drive_num = dinfo.disk;
         b->num_sides = dinfo.num_heads;
         b->track_size = dinfo.num_sects;
         b->hid_sects = fat_calc_hidden_sectors( p );
         b->total_sect = ( p->num_sect < 65536L ) ? p->num_sect : 0;
         b->big_total = ( p->num_sect < 65536L ) ? 0 : p->num_sect;
         b->magic_num = MBR_MAGIC_NUM;
      }
      else if ( ev.scan == 0x4000 ) /* F6 - Code */
      {
         memmove( b->x.f16.xcode, fat_boot_code->x.f16.xcode,
                  sizeof( b->x.f16.xcode ) );
      }
      else if ( ev.scan == 0x4100 ) /* F7 - calculate min size */
      {
         write_string( HINT_COLOR, StX2, StY + 15,
                       "Reading FAT...                 " );
         move_cursor( StX2 + 14, StY + 15 );

         fatsz = min( 256, b->fat_size16 );
         l = lc = 0;
         for ( n = b->res_sects; n < b->res_sects + fatsz; n++ ) {
            get_event( &ev, EV_KEY | EV_NONBLOCK );
            if ( ev.key == 27 ) {
               write_string(
                  TEXT_COLOR, StX, StY + 15,
                  "     Minimum partition size:  press    to calculate" );
               write_string( HINT_COLOR, StX + 36, StY + 15, "F7" );
               goto calc_aborted;
            }

            if ( disk_read_rel( p, n, tmp, 1 ) == -1 ) {
               show_error( "Error reading FAT table" );
               break;
            }
            else {
               write_int( HINT_COLOR, StX2 + 16, StY + 15, 5,
                          n - b->res_sects );

               for ( i = 0; i < SECT_SIZE; i++, l++ ) {
                  if ( tmp[i] != 0 ) {
                     lc = l;
                  }
               }
            }
         }
         min_clust =
            ( syst == 0u )
               ? ( 0u )
               : ( lc * 8u / syst + ( lc * 8u % syst == 0u ? 0u : 1u ) - 2u );

         min_num_sect = fat_non_data_sector_count( b ) +
                        ( (unsigned long)min_clust ) * b->clust_size;

         sprintf( tmp, "%lu sectors = %s kbytes", min_num_sect,
                  sprintf_long( tmp1, ( min_num_sect ) / 2 ) );
         write_string( DATA_COLOR, StX2, StY + 15, tmp );

      calc_aborted:
         (void)0;
      }

   } /* while(1) */

   free( tmp );
   return OK;
} /* setup_fat */
