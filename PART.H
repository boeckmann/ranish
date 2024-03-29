#ifndef _PART_H
#define _PART_H

#include <io.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocs.h"
#include "diskio.h"
#include "screen.h"

#define WEBSITE_URL    "https://github.com/boeckmann/ranish"
#define REPOSITORY_URL "https://github.com/boeckmann/ranish"

#define CHS_MAX_SECT 1024ul * 255ul * 63ul

#define TEXT( x ) x

_Packed struct part_rec {
   unsigned char boot_flag;
   unsigned char start_head;
   unsigned char start_sectCylH;
   unsigned char start_cylL;
   unsigned char os_id;
   unsigned char end_head;
   unsigned char end_sectCylH;
   unsigned char end_cylL;
   unsigned long rel_sect; /* Number os sectors before partition */
   unsigned long num_sect; /* Number of sectors in the partition */
};

struct part_long {
   int level;
   int empty;
   int valid;
   int active;
   int os_num;
   int mbr_err;
   int range_err;
   int overlap_err;
   int inconsist_err;
   int boot_record_err;
   unsigned short os_id;
   unsigned long start_cyl;
   unsigned long start_head;
   unsigned long start_sect;
   unsigned long end_cyl;
   unsigned long end_head;
   unsigned long end_sect;
   unsigned long rel_sect;
   unsigned long num_sect;
   int changed;
   int orig_row;
   int inh_changed;
   int inh_invalid;
   struct part_long *container;
   unsigned long container_base;
};

#define ADV_IPL_CHKSUM_OFFSET 2
#define ADV_IPL_CHKSUM_MAGIC  0x4711

#define ADV_MBR_MAGIC 0x334D4241L /* "ABM3" */

#define ADV_DATA_SIGNATURE "AdvBootManager\0"
#define ADV_DATA_VERSION   3

_Packed struct mbr {
   union {
      struct x_std {
         /*            unsigned char  code[0x1BE]; */
         unsigned char code[0x100];
      } std;
      _Packed struct x_ipl {
         unsigned char code[0x1B6];
         unsigned short dot_len;
         unsigned char vir_chk;
         unsigned char def_part;
         unsigned char fd_params[4];
      } ipl;
      _Packed struct x_adv_old {
         unsigned char code[0x1A0];
         unsigned char signature[15]; /* AdvBootManager\0 */
         unsigned char version;       /* Data structure version */
         unsigned long rel_sect;
         unsigned char act_menu;
         unsigned char reserved[9];
      } adv_old;
      _Packed struct x_adv /*  0123456789ABCD  */
      {                    /*  rrrr____aoABM3  */
         /*  rrrrffffatABM3  */
         unsigned char code[0x1AE];
         unsigned short adv_chk_sum;
         unsigned long rel_sect;
         unsigned long reserved;
         unsigned char act_menu;
         unsigned char boptions;
         unsigned long adv_mbr_magic;
      } adv;
      _Packed struct x_adv_new {
         unsigned char code[0x1BC];
         unsigned short bm_sign; /* 'BM' = 0x4D42 */
      } new;
   } x;
   _Packed struct part_rec part_rec[4];
   unsigned short magic_num; /* Magic number (must be 0xAA55) */
};

#define MBR_MAGIC_NUM ( 0xAA55 )

#define BM_238_SIGN ( 0x4D42 )

#define MENU_BOOT_EMPTY   0
#define MENU_BOOT_PART    1
#define MENU_BOOT_NEXT_HD 2
#define MENU_BOOT_FLOPPY  3

#define SHOW_NONE  0
#define SHOW_LAST  1
#define SHOW_NEXT  2
#define SHOW_PREV  3
#define SHOW_LAST3 4

#define MENU_OPT_PASSW 1

_Packed struct adv_menu_rec /* 80 bytes */
{
   unsigned char boot_type; /* if 0 - empty */
   unsigned char options;
   unsigned char name[30];
   union {
      _Packed struct x_part {
         unsigned char tag;
         unsigned char show;
         unsigned char reserved[14];
         unsigned short num_keys;
         unsigned short keys[15];
      } part;
   } x;
};

_Packed struct adv_part_rec /* 16 bytes */
{
   unsigned short os_id;
   unsigned char tag;
   unsigned char orig_row;
   unsigned char reserved[4];
   unsigned long rel_sect;
   unsigned long num_sect;
};

#define MAX_MENU_ROWS 16
#define MAX_PART_ROWS 32

#define ADV_CODE_SIZE 8192
#define ADV_DATA_SIZE 2048

#define ADV_CODE_SECT ( ADV_CODE_SIZE / SECT_SIZE )
#define ADV_DATA_SECT ( ADV_DATA_SIZE / SECT_SIZE )

#define ADV_NUM_SECTS ( ADV_CODE_SECT + ADV_DATA_SECT )

#define ADV_OPT_VIR_CHECK  1
#define ADV_OPT_CLEAR_SCR  2
#define ADV_OPT_DEF_MENU   4
#define ADV_OPT_IGN_UNUSED 8

_Packed struct adv /* 80+1280+512+176=2048 */
{
   unsigned char signature[15]; /* AdvBootManager\0 */
   unsigned char version;       /* Data structure version */
   unsigned char def_menu;
   unsigned char timeout;
   unsigned char options;
   unsigned char options2;
   unsigned int password; /* code to validate password */
   unsigned char reserved2[26];
   unsigned char adv_title[32];
   struct adv_menu_rec menu[MAX_MENU_ROWS]; /* 1280 */
   struct adv_part_rec part[MAX_PART_ROWS]; /*  512 */
   unsigned char reserved[176];
};

struct os_desc {
   unsigned short os_id;
   char *name;
   unsigned short flags;
   int ( *setup )( struct part_long * );
   int ( *format )( struct part_long *, char **argv, char **msg );
   int ( *print )( struct part_long * );
};

#define OS_EXT     0x0500
#define OS_EXT_LBA 0x0F00
#define OS_HIDDEN  0xFF80
#define OS_ADV     0xFF81
#define OS_UNKN    0xFFFF

#define MODE_CHS 0
#define MODE_LBA 1

#define VIEW_MBR  0
#define VIEW_ADV  1
#define VIEW_EMBR 2

#define OK     0
#define FAILED -1
#define CANCEL -2
#define WARN   -3

#define BORDER_COLOR ( Yellow + BakBlue )
#define HEADER_COLOR ( BrCyan + BakBlue )
#define TITLE_COLOR  ( BrWhite + BakBlue )
#define TABLE_COLOR  ( BrWhite + BakBlue )
#define INVAL_COLOR  ( BrRed + BakBlue )
#define ACTIVE_COLOR ( BrWhite + BakBlack )
#define MENU_COLOR   ( BrCyan + BakBlue )
#define HINT_COLOR   ( Yellow + BakBlue )
#define MESG_COLOR   ( BrGreen + BakBlue )
#define WARN_COLOR   ( BrRed + BakBlue )

#define MAIN_BORDER "�ͻ� ��ͼ"
#define VERT_BORDER "� �� �� �"
#define HRZN_BORDER " �       "

extern unsigned EMP_IPL_SIZE;

void __cdecl COMPACT_MBR_IPL( void ); /* external assembly code */
void __cdecl ADV_IPL( void );
void __cdecl EMP_IPL( void );
void __cdecl STD_IPL( void );
void __cdecl FAT_BOOT( void );
void __cdecl ADV_MANAGER( void );

unsigned short calc_chksum( const unsigned short *p, const int len );

void get_base_dir( char * );

void html_view( char *target );

int progress( char *msg );

int disk_read_rel( struct part_long *p, unsigned long sect, void *buf,
                   int num_sect );
int disk_write_rel( struct part_long *p, unsigned long sect, void *buf,
                    int num_sect );

void part_set_id( struct part_long *part, unsigned short id );
int part_is_fat_type( struct part_long *part );

void adjust_part_type( struct part_long *p, int interactive );

int part_verify_sectors( struct part_long *p, unsigned long start_sect,
                         unsigned long num_sect,
                         int ( *func )( unsigned long, unsigned long ),
                         int ( *bbt_func )( unsigned long ) );

int part_fill_sectors( struct part_long *p, unsigned long start_sect,
                       unsigned long num_sect, unsigned char fill_value,
                       int ( *func )( unsigned long, unsigned long ),
                       int ( *bbt_func )( unsigned long ) );

int generic_clean( struct part_long *p );
int generic_verify( struct part_long *p, int bbt_size, unsigned long *bbt );
int generic_format( struct part_long *p, int bbt_size, unsigned long *bbt );

void start_gui( void );

int setup_mbr( struct part_long *p );

void convert_adv_data( struct adv *adv );
void recalculate_part( struct part_long *part, int mode );
int validate_table( struct part_long *part, int n,
                    struct part_long *container );
int prepare_adv_mbr_for_save( struct part_long *part, struct mbr *mbr,
                              struct adv *adv );

int write_int( int attr, int x, int y, int w, unsigned long xx );

void parse_arg( char *str, char **argv, int argv_size );

char *sprintf_long( char *tmp, unsigned long num );
char *sprintf_systype( char *tmp, int os_num );
char *sprintf_os_name( char *tmp, struct part_long * );
char *sprintf_partrec( char *tmp, struct part_long *, int n, int view );

void determine_os_num( struct part_long * );

void pack_part_tab( struct part_long *part, struct part_rec *part_rec,
                    int n );
char *unpack_part_tab( struct part_rec *part_rec, struct part_long *part,
                       int n, struct part_long *container );

void pack_adv_part_tab( struct part_long *part, struct adv_part_rec *part_rec,
                        int n );
void unpack_adv_part_tab( struct adv_part_rec *part_rec,
                          struct part_long *part, int n,
                          struct part_long *container );
void show_error( char * );
int two_btn_dialog( char *msg, char *btn1, int res1, char *btn2, int res2 );
void setup_ipl( struct mbr * );
void edit_boot_menu( struct adv *adv, int p, int flag );
void select_system_type( struct part_long *p );
int enter_string( int x, int y, char *prompt, int maxlen, char *str,
                  char *help );

unsigned int __cdecl encrypt_password( char *password );

void edit_str_field( struct event *ev, int ev_mask, int attr, int x, int y,
                     int maxlen, char *str, int *pos );
void edit_int_field( struct event *ev, int ev_mask, int attr, int x, int y,
                     int field_len, unsigned long *n, unsigned long limit );

void redraw_menu( int view );
void redraw_header( int view, int mode, struct part_long *p );

int save_to_file( char *filename, void *buf, int len );
int load_from_file( char *filename, void *buf, int len );
void print_part_details( struct part_long *part, int num_rows );
void print_adv_menu( struct adv *adv );
void print_ide_info( void );

void usage( void );
void command_line( int argc, char **argv );
void cmd_error( char *msg );
int print_unknown( struct part_long *p );

int print_embr( struct part_long *p );
int format_embr( struct part_long *p, char **argv, char **msg );

int setup_fat( struct part_long *p );
int print_fat( struct part_long *p );

int setup_fat32( struct part_long *p );
int print_fat32( struct part_long *p );
int format_unused( struct part_long *p, char **argv, char **msg );

void set_messages( void );

extern struct os_desc *os_desc;
extern struct event ev;
extern int hd, select_target, mode, quiet;
extern char tmp2[80];
extern char tmp3[80];
extern char file_name[80];

extern int interactive;
extern char buf[25 * 80 * 2];
extern char base_dir[256];
extern char tmp[SECT_SIZE * 3];
extern struct disk_info dinfo;

#ifdef _PART_C
#define mesg( M ) char *M
#else
#define mesg( M ) extern char *M
#endif

mesg( PROGRAM_TITLE );
mesg( MANAGER_TITLE );

mesg( HEADER_GET );
mesg( HEADER_EMBR );
mesg( HEADER_READ );
mesg( HEADER_SYSTYPE );
mesg( HEADER_BOOT_MENU );

mesg( HEADER_CMD );

mesg( HEADER_CHS );
mesg( HEADER_CHS2 );
mesg( HEADER_CHS3 );

mesg( HEADER_LBA );
mesg( HEADER_LBA2 );
mesg( HEADER_LBA3 );

mesg( MENU_HIDE );
mesg( MENU_FORMAT );
mesg( MENU_VERIFY );

mesg( MENU_SAVE_MBR );
mesg( MENU_LOAD_MBR );
mesg( MENU_SAVE_ADV );
mesg( MENU_LOAD_ADV );
mesg( MENU_INST_IPL );

mesg( MENU_INSTALL );
mesg( MENU_UNINSTALL );
mesg( MENU_PREVIEW );
mesg( MENU_ADV_UNINST );
mesg( MENU_INST_EXT );
mesg( MENU_MAKE_PRI );

mesg( WARN_INVALID );
mesg( MESG_BIOS_CYL );

mesg( MESG_MBR_SAVED );
mesg( MESG_ADV_SAVED );
mesg( MESG_NOT_SAVED );
mesg( MESG_INSTALL );
mesg( MESG_UNINSTALL );
mesg( MESG_MBR_SAVED2 );
mesg( MESG_ADV_SAVED2 );
mesg( MESG_FILE_SAVED );
mesg( MESG_ADVFILE_SAVED );
mesg( MESG_ADVFILE_LOADD );

mesg( MESG_FILE_LOADD );

mesg( ERROR_MALLOC );
mesg( ERROR_DISK_INFO );
mesg( ERROR_FIX_FIRST );
mesg( ERROR_INH_INVAL );

mesg( ERROR_READ_MBR );
mesg( ERROR_SAVE_MBR );
mesg( ERROR_READ_ADV );
mesg( ERROR_SAVE_ADV );
mesg( ERROR_LOAD_FILE );
mesg( ERROR_SAVE_FILE );

mesg( ERROR_NO_ADV );
mesg( ERROR_ADV_BAD );
mesg( ERROR_ADV_VER );
mesg( ERROR_GR_FOUR );
mesg( ERROR_NO_UNUSED );

mesg( ERR_BOOTREC );
mesg( ERR_INCONS );
mesg( ERR_OVERLAP );
mesg( ERR_RANGE );
mesg( ERR_MBR );

mesg( HINT_ADV );
mesg( HINT_INS );
mesg( HINT_CHS );
mesg( HINT_LBA );
mesg( HINT_RETURN );

mesg( HELP_CMD_LINE );
mesg( HELP_FILE_NAME );
mesg( PROMPT_FILE );
mesg( PROMPT_FORMAT );
mesg( PROMPT_FSTYPE );

mesg( ERROR_NO_SETUP );
mesg( ERROR_NO_FORMAT );

mesg( MESG_FORMAT_OK );
mesg( WARN_FORMAT_CANCEL );
mesg( ERROR_FORMAT_FAILED );

mesg( MESG_VERIFY_OK );
mesg( WARN_VERIFY_CANCEL );
mesg( ERROR_VERIFY_FAILED );

mesg( MESG_CLEANING );
mesg( MESG_VERIFYING );
mesg( MESG_FORMATTING );

mesg( ERROR_FORMAT_GEN );
mesg( ERROR_FORMAT_WIN95 );
mesg( ERROR_FORMAT_EMPTY );
mesg( ERROR_FORMAT_FRACTION );

mesg( IPL_VIRUS );
mesg( IPL_DOTS );
mesg( IPL_DEFAULT );
mesg( IPL_NOACTV );

mesg( HTML_DOC_EMPTY );
mesg( HTML_NOT_FOUND );
mesg( HTML_ERROR_READ );

mesg( MESG_NON_SYSTEM );
mesg( MESG_EXT_NONBOOT );

mesg( NAME_OS_UNUSED );
mesg( NAME_OS_ADV );
mesg( NAME_OS_HIDDEN );
mesg( NAME_OS_UNKN );

mesg( MESG_DIFFERENT );
mesg( MESG_NO_DIFFERENCES );

mesg( MESG_LOGICAL );
mesg( ERROR_ADV_NOSPACE );

mesg( MENU_ADV_OPT_TITLE );
mesg( MENU_ADV_OPT_TIMEOUT );
mesg( MENU_ADV_OPT_VIRCHECK );
mesg( MENU_ADV_OPT_CLEARSCR );
mesg( MENU_ADV_OPT_DEFAULT );
mesg( MENU_ADV_OPT_PASSWORD );
mesg( MENU_ADV_OPT_IGN_UNUSED );

#undef mesg

#endif
