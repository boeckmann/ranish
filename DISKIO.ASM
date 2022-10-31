section DATA class=DATA

SECT_SIZE	equ	512	; If not we will eventually crash

global	_detected_os

SYS_DOS		equ	0	; MS-DOS v1.00 - v4.xx
SYS_DOS5	equ	1	; MS-DOS v5.00 - v6.xx
SYS_DOS7	equ	2	; Comand line mode of Win95
SYS_WIN3	equ	3	; GUI mode of Windows 3.xx
SYS_WIN95	equ	4	; GUI mode of Windows 95
SYS_WIN_NT	equ	5	; Windows NT

_detected_os	dw	0

_reboot_addr	dd	0FFFF0000h

section DISKIO_TEXT class=CODE align=16

global _diskio_init
global _diskio_exit

global _reboot
global _flush_caches

global _get_disk_info

global _disk_lock
global _disk_unlock

global _dos_drive_reset

global _disk_read
global _disk_write
global _disk_format
global _disk_verify

;----------------------------------------------------------------


_diskio_init:
		push	bp
		mov	bp, sp
		push	es
		push	si
		push	di

		mov	si, SYS_DOS
		mov	ax, 3000h	; Get DOS version
		int	21h
		cmp	al, 5
		jb	.end
		
		mov	ax, 3306h	; Get true DOS version
		int	21h
		
		mov	si, SYS_WIN_NT
		cmp	bx, 3205h	; WinNT dos box returns v5.50
		je	.end

		mov	ax, 1600h	; Get Windows version
		int	2Fh
		cmp	al, 0
		je	.no_win
		cmp	al, 80h
		je	.no_win
		cmp	al, 0FFh
		je	.no_win

		mov	si, SYS_WIN3
		cmp	al, 3
		je	.end

		mov	si, SYS_WIN95	; Windows 95 GUI
		
		mov	ax, 1684h	; Lets get Reboot VxD API
		mov	bx, 0009h
		int	2Fh
		mov	bx, es
		or	bx, di		; Did it returned 0000:0000h ?
		jz	.end

		mov	[_reboot_addr], di
		mov	[_reboot_addr+2], es

		jmp	.end
		
	.no_win:			; Windows 3+ is not running

		mov	ax, 3306h	; Get true DOS version
		int	21h

		mov	si, SYS_DOS5
		cmp	bl, 7		; Windows 95 reports v 7.xx
		jb	.end

		mov	si, SYS_DOS7
	.end:
		mov	[_detected_os], si

		pop	di
		pop	si
		pop	es
		pop	bp
		retf


_diskio_exit:
		retf

;----------------------------------------------------------------

_reboot:
	call	_flush_caches
	mov	ax, 0100h	; Warm reboot if
	call	[_reboot_addr]	; calling Win95 API

crazy:	jmp	crazy		; We should never get crazy


;----------------------------------------------------------------

_flush_caches:
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


;----------------------------------------------------------------

_get_disk_info:

	.buf_ptr	equ	0Ch
	.disk_info	equ	08h
	.hd		equ	06h

	.disk		equ	0
	.num_cyls	equ	2
	.num_heads	equ	4
	.num_sects	equ	6
	.total_sects0	equ	8
	.total_sects1	equ	10
	.sect_per_cyl	equ	12
	.sect_per_track	equ	14
	.sect_size	equ	16
	.bios_num_cyls	equ	18

		push	bp
		mov	bp, sp
		push	es
		push	si
		push	di
		les	si, [bp+.disk_info]


		mov	ah, 08h
		mov	dl, [bp+.hd]
		int	13h

		mov	ax, -1
		jnc	.fill_struct
		jmp	.end

.fill_struct:
		mov	ax, SECT_SIZE
		mov	[es:si+.sect_size], ax

		mov	ah, 0
		mov	al, dl
		push	ax	; num_disks

		mov	ah, 0
		mov	al, cl
		and	al, 3Fh
		mov	[es:si+.num_sects], ax
		mov	[es:si+.sect_per_track], ax

		mov	al, dh
		inc	ax
		mov	[es:si+.num_heads], ax

		mov	al, ch
		shl	cx, 1
		shl	cx, 1
		and	ch, 3
		mov	ah, ch
		inc	ax
		mov	[es:si+.num_cyls], ax
		mov	[es:si+.bios_num_cyls], ax

		mov	ax, [bp+.hd]
		mov	[es:si+.disk], ax
		
		
		push	es			; Lets check if BIOS hides last cyl

		mov	dx, [es:si+.disk]	; read last sector from last + 1 cyl
		mov	ax, [es:si+.num_heads]
		dec	ax
		mov	dh, al 	; head
		mov	ax, [es:si+.num_sects]
		mov	cl, al	; sect
		mov	bx, [es:si+.bios_num_cyls]
		cmp	bx, 1024
		jz	.nomore
		mov	ch, bl
		mov	bl, 0
		shr	bx, 1
		shr	bx, 1
		or	cl, bl

		les	bx, [bp+.buf_ptr]
		mov	ax, es
		or	ax, bx
		jz	.nomore
		mov	ah, 02h	; Read 1
		mov	al, 1	; sector
		int	13h
		jc	.nomore
		mov	bx, [es:si+.bios_num_cyls]
		inc	bx
		mov	[es:si+.num_cyls], bx
	.nomore:
		pop	es

		mov	ax, [es:si+.num_sects]
		mov	bx, [es:si+.num_heads]
		mul	bx
		mov	[es:si+.sect_per_cyl], ax
		mov	bx, [es:si+.num_cyls]
		mul	bx
		mov	[es:si+.total_sects0], ax
		mov	[es:si+.total_sects1], dx

		pop	ax	; num_disks

	.end:
		pop	di
		pop	si
		pop	es
		pop	bp
		retf


;----------------------------------------------------------------

_disk_lock:

    .disk	equ	06h

		push	bp
		mov	bp, sp

		cmp	word [_detected_os], SYS_WIN95
		jb	.skip

		mov	ax, 440Dh
		mov	cx, 084Bh	; Lock phisical disk
		mov	bl, [bp+.disk]
		mov	bh, 1		; Lock level (0-3)
		mov	dx, 0		; Device permissions
		int	21h

		mov	ax, -1
		jc	.end
	.skip:
		mov	ax, 0
	.end:
		pop	bp
		retf


;----------------------------------------------------------------

_disk_unlock:

    .disk	equ	06h

		push	bp
		mov	bp, sp

		cmp	word [_detected_os], SYS_WIN95
		jb	.skip

		mov	ax, 440Dh
		mov	cx, 086Bh	; Unlock phisical disk
		mov	bl, [bp+.disk]
		int	21h

		mov	ax, -1
		jc	.end
	.skip:
		mov	ax, 0
	.end:
		pop	bp
		retf


;----------------------------------------------------------------

_dos_drive_reset:

  .drive_num	equ	06h

		push	bp
		mov	bp, sp
		push	ds
		push	es
		push	si
		push	di

		mov	ah, 32h
		mov	dl, [bp+.drive_num]
		int	21h
		mov	ah, al
		
		pop	di
		pop	si
		pop	es
		pop	ds
		pop	bp
		retf


;----------------------------------------------------------------

  disk		equ	0
  cyl		equ	2
  head		equ	4
  sect		equ	6

 %macro Get_Disk_Params 0

		mov	dl, [es:si+disk]
		mov	dh, [es:si+head]
		mov	cl, [es:si+sect]
		and	cl, 3Fh
		mov	bx, [es:si+cyl]
		mov	ch, bl
		mov	bl, 0
		shr	bx, 1
		shr	bx, 1
		or	cl, bl
%endmacro


;----------------------------------------------------------------

_disk_read:

  .num		equ	0Eh
  .buf_ptr	equ	0Ah
  .disk_addr	equ	06h

		push	bp
		mov	bp, sp
		push	es
		push	si
		push	di
		les	si, [bp+.disk_addr]

		mov	ah, 02h	; Read
		mov	al, [bp+.num]	; Sectors
		Get_Disk_Params
		les	bx, [bp+.buf_ptr]
		int	13h
		
		mov	ax, -1
		jc	.skip
		mov	ax, 0
	.skip:

		pop	di
		pop	si
		pop	es
		pop	bp
		retf


;----------------------------------------------------------------


_disk_write:

  .num		equ	0Eh
  .buf_ptr	equ	0Ah
  .disk_addr	equ	06h

		push	bp
		mov	bp, sp
		push	es
		push	si
		push	di
		les	si, [bp+.disk_addr]

		mov	ah, 03h	; Write
		mov	al, [bp+.num] ; Sectors
		Get_Disk_Params
		les	bx, [bp+.buf_ptr]
		int	13h
		
		mov	ax, -1
		jc	.skip
		mov	ax, 0
	.skip:

		pop	di
		pop	si
		pop	es
		pop	bp
		retf


;----------------------------------------------------------------


_disk_verify:

  .num_sect	equ	0Ah
  .disk_addr	equ	06h

		push	bp
		mov	bp, sp
		push	es
		push	si
		push	di
		les	si, [bp+.disk_addr]

		mov	ah, 04h			; Verify
		mov	al, [bp+.num_sect]	; How many sectors
		Get_Disk_Params
		int	13h
		
		mov	ax, -1
		jc	.end
		mov	ax, 0
	.end:
		pop	di
		pop	si
		pop	es
		pop	bp
		retf


;----------------------------------------------------------------

_disk_format:

  .ftable	equ	0Ah
  .disk_addr	equ	06h

		push	bp
		mov	bp, sp
		push	es
		push	si
		push	di
		les	si, [bp+.disk_addr]

		mov	ah, 05h		; Format
		Get_Disk_Params
		les	bx, [bp+.ftable]
		int	13h
		
		mov	ax, -1
		jc	.end
		mov	ax, 0
	.end:
		pop	di
		pop	si
		pop	es
		pop	bp
		retf

