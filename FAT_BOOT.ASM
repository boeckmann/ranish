; Source code under Creative Commons CC-0

; Assemble to OMF object with:	nasm fatboot.asm -t -f obj
; Or assemble to binary with:	nasm fatboot.asm -t

%ifidni __OUTPUT_FORMAT__, obj
	section FAT_BOOT class=BOOT align=16
	global _FAT_BOOT
%endif

_FAT_BOOT:
	;
	;  MBR loads BOOT at 0000:7C00h
	;
	jmp	short .xcode
	nop
	;
RESERVD: equ $
	times 59 db 0
	;
.xcode:
	xor	ax, ax
	mov	ds, ax
	mov	es, ax
	mov	ss, ax		;  CPU clears interrupt flag for next command
	mov	sp, 7C00h
	;
	mov	bp, sp
	mov	[bp+24h], dl	; Save disk number

	mov	al, 20h			; 32 bytes in each directory entry
	mul	word [bp+11h]	; number of entries in root directory
	div	word [bp+0Bh]	; sector size
	mov	cx, ax			; number of sectors in root directory

	mov	al, [bp+10h]		; Number of FATs
	cbw
	mul	word [bp+16h]	; Size of the FAT
	add	ax, [bp+0Eh]		; Reserved Sectors
	adc	dx, 0
	add	ax, [bp+1Ch]		; Hidden sectors
	adc	dx, [bp+1Eh]

	push	cx
	mov	bx, 0500h		; Lets read root dir to that address
	mov	cx, 1
	call	READ_N_SECT		; Reading 1 sector to make DOS happy
	pop	cx

	mov	bx, 8000h		; Here we have enought space for large
					; root directory (more than 944 entr.)

	call	READ_N_SECT		; Reading CX sectors to address BX
	jb	.print_error		; starting with sector DX:AX

	add	ax, cx
	adc	dx, 0

	push	dx	; Now DX:AX is the first sector of the first cluster
	push	ax

	; Looking for WINBOOT.SYS, IO.SYS, ...

	lea	di, [bp+(IO_SYS-_FAT_BOOT)]
.next_name:
	mov	si, bx
	cmp	Byte [di], 0
	je	.print_error
	mov	cx, [bp+11h]	; number of entries in root directory
.next_entr:
	push	si
	push	di
	push	cx
	mov	cx, 0Bh
	repz
	 cmpsb
	pop	cx
	pop	di
	pop	si
	jz	.found
	add	si, 20h
	loop	.next_entr
	add	di, 0Bh
	jmp	.next_name
.found:
	push	ax
	push	dx

	mov	di, [si+1Ah]	; first cluster in file
	lea	ax, [di-2]
	jc	.print_error
	mov	dl, [bp+0Dh]	; cluster size
	mov	dh, 0
	mul	dx

	pop	cx
	pop	bx
	add	ax, bx
	adc	dx, cx

	mov	bx, 0700h
	mov	cx, 4

	call	READ_N_SECT

	jb	.print_error

	cmp	word [bx], 5A4Dh	; "MZ"
	je	.win95
	
	pop	bx	;ax
	pop	ax	;dx

	mov	ch, [bp+15h]	; Media descriptor byte
	mov	dl, [bp+24h]	; Hard drive number
.dos:
	jmp	0070h:0000h

.win95:
	mov	bx, 0078h	; IO.SYS expects to see all this garbage.
	lds	si, [bx]	; Plus it's cluster number in DI and
	push	ds		; it's relative sector in [bp-02]:[bp-04].
	push	si
	push	ss
	push	bx
	lea	si, [bp+(NULL00-_FAT_BOOT)]
	jmp	0070h:0200h

	;
	; Print error, wait for key, reboot.
	;
.print_error:
	lea	si, [bp+(ERRMSG-_FAT_BOOT)]
	mov	ah, 0Eh
	mov	bx, 0007h
.pr1:
	lodsb
	or	al, al
	jz	.pr2
	int	10h
	jmp	.pr1
.pr2:
	xor	ax, ax
	int	16h
	int	19h

	;
	;
	;
READ_N_SECT:
		;
		;
		;	ES:BX - Destination address
		;	DX:AX - Relative sector on disk
		;	CX    - Number of sectors to read
		;
		;	Returns: Flag CF set if error
		;
		push	ax
		push	bx
		push	cx
		push	dx

.next_sect:
		call	READ_SECT
		jb	.end

		add	ax, 1
		adc	dx, 0

		add	bx, [bp+0Bh]	; Sector size

		loop	.next_sect
.end:
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret

		;
		;
		;
READ_SECT:
		;
		;
		;	ES:BX - Destination address
		;	DX:AX - Relative sector on disk
		;
		;	Returns: Flag CF set if error
		;
		push	si
		push	di
		push	bx
		push	cx
		push	dx
		push	ax

		mov	cx, 3
		push	cx
.next_try:
		mov	ah, 08		; Get disk parameters
		mov	dl, [bp+24h]	; Hard disk number
		int	13h
		jb	.reset

		mov	ah, 0
		mov	al, dh
		inc	ax		; Number of heads / cylinder
		and	cx, 3Fh
		mov	di, cx		; Number of sectors / head
		mul	cx
		mov	si, ax		; Number of sectors / cylinder

		pop	cx
		pop	ax		; Rel sect low
		pop	dx		; Rel sect high
		push	dx
		push	ax
		push	cx

		div	si	; Now ax=cylinder, dx=sector on cylinder
		mov	cx, ax
		shr	cx, 1
		shr	cx, 1
		and	cl, 0C0h
		mov	ch, al
		mov	ax, dx
		xor	dx, dx
		div	di	; Now ax=head, dx=sector on head
		mov	dh, al
		inc	dl
		and	dl, 3Fh
		or	cl, dl
		mov	dl, [bp+24h]	; Hard disk number
		mov	ax, 0201h	; Read (AH=02) 1 Sector (AL=01)
		int	13h
		jnb	.end
.reset:
		pop	cx
		dec	cx
		push	cx
		jb	.end
		
		mov	ax, 0
		int	13h
		jmp	.next_try
.end:
		pop	cx
		pop	ax
		pop	dx
		pop	cx
		pop	bx
		pop	di
		pop	si
		ret

	;
	;
	;
ERRMSG:	DB 0Dh,0Ah,"Non-system disk or error."
	DB 0Dh,0Ah,"Hit a key to reboot ... "
NULL00:	DB 0,0
	;
	;  "1234567 123"
IO_SYS:	
	DB "WINBOOT SYS"
	DB "IO      SYS"
	DB 0
	;

	times 510 - ($ - $$) db 0

	DB	055h, 0AAh
