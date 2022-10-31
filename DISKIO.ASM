.MODEL LARGE
.DATA

SECT_SIZE	EQU	512	; If not we will eventually crash

PUBLIC	_detected_os

SYS_DOS		EQU	0	; MS-DOS v1.00 - v4.xx
SYS_DOS5	EQU	1	; MS-DOS v5.00 - v6.xx
SYS_DOS7	EQU	2	; Comand line mode of Win95
SYS_WIN3	EQU	3	; GUI mode of Windows 3.xx
SYS_WIN95	EQU	4	; GUI mode of Windows 95
SYS_WIN_NT	EQU	5	; Windows NT

_detected_os	DW	0

_reboot_addr	DD	0FFFF0000h

.CODE
LOCALS

PUBLIC _diskio_init
PUBLIC _diskio_exit

PUBLIC _reboot
PUBLIC _flush_caches

PUBLIC _get_disk_info

PUBLIC _disk_lock
PUBLIC _disk_unlock

PUBLIC _dos_drive_reset

PUBLIC _disk_read
PUBLIC _disk_write
PUBLIC _disk_format
PUBLIC _disk_verify

;----------------------------------------------------------------


_diskio_init	PROC
		push	bp
		mov	bp, sp
		push	es
		push	si
		push	di

		mov	si, SYS_DOS
		mov	ax, 3000h	; Get DOS version
		int	21h
		cmp	al, 5
		jb	@@end
		
		mov	ax, 3306h	; Get true DOS version
		int	21h
		
		mov	si, SYS_WIN_NT
		cmp	bx, 3205h	; WinNT dos box returns v5.50
		je	@@end

		mov	ax, 1600h	; Get Windows version
		int	2Fh
		cmp	al, 0
		je	@@no_win
		cmp	al, 80h
		je	@@no_win
		cmp	al, 0FFh
		je	@@no_win

		mov	si, SYS_WIN3
		cmp	al, 3
		je	@@end

		mov	si, SYS_WIN95	; Windows 95 GUI
		
		mov	ax, 1684h	; Lets get Reboot VxD API
		mov	bx, 0009h
		int	2Fh
		mov	bx, es
		or	bx, di		; Did it returned 0000:0000h ?
		jz	@@end

		mov	Word Ptr [_reboot_addr], di
		mov	Word Ptr [_reboot_addr+2], es

		jmp	@@end
		
	@@no_win:			; Windows 3+ is not running

		mov	ax, 3306h	; Get true DOS version
		int	21h

		mov	si, SYS_DOS5
		cmp	bl, 7		; Windows 95 reports v 7.xx
		jb	@@end

		mov	si, SYS_DOS7
	@@end:
		mov	_detected_os, si

		pop	di
		pop	si
		pop	es
		pop	bp
		retf
_diskio_init	ENDP


_diskio_exit	PROC
		retf
_diskio_exit	ENDP

;----------------------------------------------------------------

_reboot	PROC
	call	_flush_caches
	mov	ax, 0100h	; Warm reboot if
	call	[_reboot_addr]	; calling Win95 API
crazy:	jmp	crazy		; We should never get crazy
_reboot	ENDP

;----------------------------------------------------------------

_flush_caches	PROC
		push	ds
		push	es
		push	bp
		push	si
		push	di

		mov	ah, 0Dh		; Flush and reset MS-DOS buffers
		int	21h

		mov	ax, 4A10h	; Flush SmartDrive 4+ Caches
		mov	bx, 01
		mov	cx, 0EBABh
		int	2Fh
		mov	ax, 4A10h	; Reset SmartDrive 4+ Caches
		mov	bx, 02
		mov	cx, 0EBABh
		int	2Fh

		xor	ax, ax
		
		pop	di
		pop	si
		pop	bp
		pop	es
		pop	ds
		retf
_flush_caches	ENDP


;----------------------------------------------------------------



_get_disk_info	PROC

   buf_ptr	EQU	[bp+0Ch]
  disk_info	EQU	[bp+08h]
     hd		EQU	[bp+06h]

 disk		EQU	Word Ptr es:[si]
 num_cyls	EQU	Word Ptr es:[si+2]
 num_heads	EQU	Word Ptr es:[si+4]
 num_sects	EQU	Word Ptr es:[si+6]
 total_sects0	EQU	Word Ptr es:[si+8]
 total_sects1	EQU	Word Ptr es:[si+10]
 sect_per_cyl	EQU	Word Ptr es:[si+12]
 sect_per_track	EQU	Word Ptr es:[si+14]
 sect_size	EQU	Word Ptr es:[si+16]
 bios_num_cyls	EQU	Word Ptr es:[si+18]

		push	bp
		mov	bp, sp
		push	es
		push	si
		push	di
		les	si, disk_info


		mov	ah, 08h
		mov	dl, hd
		int	13h

		mov	ax, -1
		jnc	@@fill_struct
		jmp	@@end

@@fill_struct:
		mov	ax, SECT_SIZE
		mov	sect_size, ax

		mov	ah, 0
		mov	al, dl
		push	ax	; num_disks

		mov	ah, 0
		mov	al, cl
		and	al, 3Fh
		mov	num_sects, ax
		mov	sect_per_track, ax

		mov	al, dh
		inc	ax
		mov	num_heads, ax

		mov	al, ch
		shl	cx, 1
		shl	cx, 1
		and	ch, 3
		mov	ah, ch
		inc	ax
		mov	num_cyls, ax
		mov	bios_num_cyls, ax

		mov	ax, hd
		mov	disk, ax
		
		
		push	es		; Lets check if BIOS hides last cyl

		mov	dx, disk	; read last sector from last + 1 cyl
		mov	ax, num_heads
		dec	ax
		mov	dh, al 	; head
		mov	ax, num_sects
		mov	cl, al	; sect
		mov	bx, bios_num_cyls
		cmp	bx, 1024
		jz	@@nomore
		mov	ch, bl
		mov	bl, 0
		shr	bx, 1
		shr	bx, 1
		or	cl, bl

		les	bx, buf_ptr
		mov	ax, es
		or	ax, bx
		jz	@@nomore
		mov	ah, 02h	; Read 1
		mov	al, 1	; sector
		int	13h
		jc	@@nomore
		mov	bx, bios_num_cyls
		inc	bx
		mov	num_cyls, bx
	@@nomore:
		pop	es

		mov	ax, num_sects
		mov	bx, num_heads
		mul	bx
		mov	sect_per_cyl, ax
		mov	bx, num_cyls
		mul	bx
		mov	total_sects0, ax
		mov	total_sects1, dx

		pop	ax	; num_disks

	@@end:
		pop	di
		pop	si
		pop	es
		pop	bp
		retf
_get_disk_info	ENDP


;----------------------------------------------------------------


_disk_lock	PROC

    disk	EQU	[bp+06h]

		push	bp
		mov	bp, sp

		cmp	_detected_os, SYS_WIN95
		jb	@@skip

		mov	ax, 440Dh
		mov	cx, 084Bh	; Lock phisical disk
		mov	bl, disk
		mov	bh, 1		; Lock level (0-3)
		mov	dx, 0		; Device permissions
		int	21h

		mov	ax, -1
		jc	@@end
	@@skip:
		mov	ax, 0
	@@end:
		pop	bp
		retf
_disk_lock	ENDP


;----------------------------------------------------------------


_disk_unlock	PROC

    disk	EQU	[bp+06h]

		push	bp
		mov	bp, sp

		cmp	_detected_os, SYS_WIN95
		jb	@@skip

		mov	ax, 440Dh
		mov	cx, 086Bh	; Unlock phisical disk
		mov	bl, disk
		int	21h

		mov	ax, -1
		jc	@@end
	@@skip:
		mov	ax, 0
	@@end:
		pop	bp
		retf
_disk_unlock	ENDP

;----------------------------------------------------------------

_dos_drive_reset	PROC

  drive_num	EQU	[bp+06h]

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		mov	ah, 32h
		mov	dl, drive_num
		int	21h
		mov	ah, al
		
		pop	di
		pop	si
		pop	es
		pop	ds
		pop	bp
		retf
_dos_drive_reset	ENDP


;----------------------------------------------------------------

  disk		EQU	es:[si]
  cyl		EQU	es:[si+2]
  head		EQU	es:[si+4]
  sect		EQU	es:[si+6]

 Get_Disk_Params	MACRO

		mov	dl, disk
		mov	dh, head
		mov	cl, sect
		and	cl, 3Fh
		mov	bx, cyl
		mov	ch, bl
		mov	bl, 0
		shr	bx, 1
		shr	bx, 1
		or	cl, bl

 		ENDM
;----------------------------------------------------------------


_disk_read	PROC

    num		EQU	[bp+0Eh]
  buf_ptr	EQU	[bp+0Ah]
 disk_addr	EQU	[bp+06h]

		push	bp
		mov	bp, sp
		push	es
		push	si
		push	di
		les	si, disk_addr

		mov	ah, 02h	; Read
		mov	al, num	; Sectors
		Get_Disk_Params
		les	bx, buf_ptr
		int	13h
		
		mov	ax, -1
		jc	@@skip
		mov	ax, 0
	@@skip:

		pop	di
		pop	si
		pop	es
		pop	bp
		retf
_disk_read	ENDP


;----------------------------------------------------------------


_disk_write	PROC

    num		EQU	[bp+0Eh]
  buf_ptr	EQU	[bp+0Ah]
 disk_addr	EQU	[bp+06h]

		push	bp
		mov	bp, sp
		push	es
		push	si
		push	di
		les	si, disk_addr

		mov	ah, 03h	; Write
		mov	al, num ; Sectors
		Get_Disk_Params
		les	bx, buf_ptr
		int	13h
		
		mov	ax, -1
		jc	@@skip
		mov	ax, 0
	@@skip:

		pop	di
		pop	si
		pop	es
		pop	bp
		retf
_disk_write	ENDP


;----------------------------------------------------------------


_disk_verify	PROC

  num_sect	EQU	[bp+0Ah]
 disk_addr	EQU	[bp+06h]

		push	bp
		mov	bp, sp
		push	es
		push	si
		push	di
		les	si, disk_addr

		mov	ah, 04h		; Verify
		mov	al, num_sect	; How many sectors
		Get_Disk_Params
		int	13h
		
		mov	ax, -1
		jc	@@end
		mov	ax, 0
	@@end:
		pop	di
		pop	si
		pop	es
		pop	bp
		retf
_disk_verify	ENDP


;----------------------------------------------------------------


_disk_format	PROC

  ftable	EQU	[bp+0Ah]
 disk_addr	EQU	[bp+06h]

		push	bp
		mov	bp, sp
		push	es
		push	si
		push	di
		les	si, disk_addr

		mov	ah, 05h		; Format
		Get_Disk_Params
		les	bx, ftable
		int	13h
		
		mov	ax, -1
		jc	@@end
		mov	ax, 0
	@@end:
		pop	di
		pop	si
		pop	es
		pop	bp
		retf
_disk_format	ENDP


;----------------------------------------------------------------


END
