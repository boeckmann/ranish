#ifndef DOSEXT_H
#define DOSEXT_H

_Packed struct dos_disk_info {
    unsigned short info_level;
    unsigned long serial_number;
    char volume_label[11];
    char filesystem_type[8];
};

extern int dos_get_serial(unsigned char drive, struct dos_disk_info __far *di);
extern int dos_set_fsinfo(unsigned char drive, unsigned long free_count, unsigned long next_free);
extern int dos_force_media_change(unsigned char drive);


#pragma aux dos_get_serial = \
    "mov ax, 0x6900" \
    "mov bh, 0" \
    "int 0x21" \
    "jc end" \
    "xor ax, ax" \
    "end: " \
    parm [bl] [ds dx] \
    modify [bh] \
    value [ax];
    
#endif

