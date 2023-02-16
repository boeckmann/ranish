#include "dosext.h"

extern int _dos_7304( unsigned char drive, void __far *buf );

#pragma aux _dos_7304 = "mov ax, 0x7304"                                     \
                        "mov cx, 0x18"                                       \
                        "int 0x21"                                           \
                        "jc end"                                             \
                        "xor ax, ax"                                         \
                        "end: " parm[dl][es di] modify[cx] value[ax];

int dos_set_fsinfo( unsigned char drive, unsigned long free_count,
                    unsigned long next_free )
{
   unsigned long buf[6];
   buf[0] = sizeof buf;
   buf[1] = 0; /* sub function 0 */
   buf[2] = free_count;
   buf[3] = next_free;
   buf[4] = 0; /* unused */
   buf[5] = 0; /* unused */
   return _dos_7304( drive, buf );
}

int dos_force_media_change( unsigned char drive )
{
   unsigned long buf[6];
   buf[0] = sizeof buf;
   buf[1] = 2; /* sub function 2 */
   buf[2] = 0;
   buf[3] = 0;
   buf[4] = 0;
   buf[5] = 0;
   return _dos_7304( drive, buf );
}
