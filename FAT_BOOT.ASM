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
		; This assumes that the root directory amount of entries
		;  fits into a multiple of the sector size. (A lot of loaders
		;  assume this.) If this is not true then the number of
		;  sectors calculated here will be one too low, so that
		;  wouldn't be a fatal error. (Except if the calculated
		;  number of sectors results in zero.)

		; Above instruction can be replaced by xchg cx, ax
		;  in order to save one byte.

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
		; This assumes that MS-DOS v6 IO.SYS is loaded into the first
		;  directory entry to 00500h and MSDOS.SYS is loaded into the
		;  second directory entry to 00520h. These addresses are
		;  required to allow the MS-DOS v6 initial loader to proceed.
		; Unlike Microsoft's loader we do not verify that these are
		;  actually those files' directory entries.
		; This is also unlike ldosboot boot.asm, which will actually
		;  find and store the files' entries in those locations when
		;  its full MS-DOS v6 load protocol compatibility is enabled.
		;  For ldosboot it does not matter where in the root directory
		;  the entries are, it will always search for them.

		; The above call could very likely be replaced by a single
		;  call to READ_SECT, not requiring to preserve and set CX.

	mov	bx, 8000h		; Here we have enought space for large
					; root directory (more than 944 entr.)

	call	READ_N_SECT		; Reading CX sectors to address BX
	jb	.print_error		; starting with sector DX:AX
		; This assumes a root directory no larger than 32 KiB.
		;  (64 sectors * 512 Bytes/sector, or 1024 directory entries
		;  * 32 Bytes/entry.) If the root directory is larger then
		;  the surplus sectors will overwrite the IVT it appears.

	add	ax, cx
	adc	dx, 0

	push	dx	; Now DX:AX is the first sector of the first cluster
	push	ax

	; Looking for WINBOOT.SYS, IO.SYS, ...
		; Note that we actually are searching only for one file.
		;  The first name that matches a file in the root directory
		;  is selected. If WINBOOT.SYS is found first, it is used
		;  and IO.SYS is not searched for.

	lea	di, [bp+(IO_SYS-_FAT_BOOT)]
		; This loads a constant to DI using four bytes. The same
		;  value loaded with a mov di, imm16 takes only three bytes.
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
	jz	.found		; if jumping here we have ZR NC -->
	add	si, 20h
	loop	.next_entr
	add	di, 0Bh
	jmp	.next_name
.found:			; (only running this if prior jz branched)
	push	ax
	push	dx		; dx:ax = datastart (including hidden) still

	mov	di, [si+1Ah]	; first cluster in file
	lea	ax, [di-2]
	jc	.print_error
		; This jc never branches because we got here with ZR NC.
		;  All of push, mov, and lea do not modify the flags!
		;  Presumably this was intended to jump if di was 0 or 1.

	mov	dl, [bp+0Dh]	; cluster size
	mov	dh, 0
	mul	dx

	pop	cx
	pop	bx		; cx:bx = datastart (including hidden)
	add	ax, bx
	adc	dx, cx		; sector (in unit) of first cluster of file

	mov	bx, 0700h
	mov	cx, 4

	call	READ_N_SECT

	jb	.print_error
		; This will very likely not read the correct data if
		;  the cluster size is below 2 KiB and the file is
		;  fragmented at the beginning. (I believe that this
		;  failure case is shared by Microsoft loaders.)

		; This assumes a sector size of 512 Bytes/sector.
		;  If the sector size is smaller, not enough of the
		;  initial loader will be loaded. If the sector size is
		;  8 KiB then 32 KiB will be loaded, overwriting our own
		;  stack and BPB and code. Sector sizes 512 to 4 KiB
		;  should work fine at this point.

	cmp	word [bx], 5A4Dh	; "MZ"
	je	.win95
		; There are two signatures in an MS-DOS v7 kernel file:
		;  A pseudo-MZ .EXE header signature "MZ" at word [0] and
		;  the "BJ" at word [200h]. (The lDebug initial loader
		;  replicates both of these, albeit it is actually a multi
		;  format file that can be loaded either as MS-DOS v7 or as
		;  MS-DOS v6 kernel, among others. And its MZ .EXE header is
		;  valid for a true MZ executable so that the program can be
		;  loaded as a DOS application.)

	pop	bx	;ax
	pop	ax	;dx	; ax:bx = datastart (including hidden)
		; It appears that the MS-DOS v6 initial loader only uses
		;  doublewords for datastart and hidden sectors if the
		;  BPBN signature matches (value 29h at sector byte [26h]).

	mov	ch, [bp+15h]	; Media descriptor byte
	mov	dl, [bp+24h]	; Hard drive number
		; MS-DOS v6 load protocol may actually need DS:SI = original
		;  int 1Eh vector and/or the two dwords on stack like for the
		;  MS-DOS v7 entrypoint, too. DS:SI seen in the pcjs.org
		;  "MS-DOS 6.22 (1.2M Boot)" diskette image's IO.SYS.
.dos:
	jmp	0070h:0000h

.win95:
		; dword [ss:bp - 4] = datastart (including hidden)
		; dword [ss:sp] = IVT entry of interrupt 1Eh (0:78h)
		; dword [ss:sp + 4] = original content of int 1Eh vector

		; DL may be expected to hold load unit. It would be better to
		;  set it in either case (before dispatching on kernel type).
	mov	bx, 0078h	; IO.SYS expects to see all this garbage.
	lds	si, [bx]	; Plus it's cluster number in DI and
	push	ds		; it's relative sector in [bp-02]:[bp-04].
	push	si
	push	ss
	push	bx
	lea	si, [bp+(NULL00-_FAT_BOOT)]
		; I don't believe that setting SI is required for the
		;  FAT12/FAT16 entrypoint of an MS-DOS v7 kernel.
	jmp	0070h:0200h

	;
	; Print error, wait for key, reboot.
	;
.print_error:
	lea	si, [bp+(ERRMSG-_FAT_BOOT)]
		; Another case of loading a constant with a 4-byte instruction
		;  where a mov si, imm16 would do it in 3 bytes.
	mov	ah, 0Eh
	mov	bx, 0007h
.pr1:
	lodsb
	or	al, al
		; test al, al is more idiomatic.
	jz	.pr2
	int	10h
	jmp	.pr1
		; A skipping instruction could save a byte here, by putting
		;  the int 10h instruction after label .pr1 and using the
		;  skipping instruction to skip that instruction on the
		;  beginning of the first iteration. Then the conditional
		;  branch changes to "jnz .pr1".
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
			; It is more space efficient to jump to an error
			;  handler directly from the read sector functions
			;  instead of having to add conditional jumps after
			;  every call to one of the functions.
		push	ax
		push	bx
		push	cx
		push	dx

.next_sect:
		call	READ_SECT
		jb	.end

		add	ax, 1
		adc	dx, 0
			; inc ax \ jnz @F \ inc dx is 4 bytes as opposed
			;  to these instructions needing 6 bytes.

		add	bx, [bp+0Bh]	; Sector size
			; Assumes that this offset does not carry,
			;  so that the function will return NC on success.
			;  (This is another edge case that is triggered if
			;  the root directory is exactly 32 KiB long.
			;  That is, exactly 1024 root directory entries.)

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
			; This does not support LBA extensions reads.
		push	si
		push	di
		push	bx
		push	cx
		push	dx
		push	ax

		mov	cx, 3
			; This appears to be intended as a repeat count.
			;  However, it does not work currently.
		push	cx
.next_try:
		mov	ah, 08		; Get disk parameters
		mov	dl, [bp+24h]	; Hard disk number
		int	13h
		jb	.reset		; (only branches if CY -->)
			; It is not very elegant to call the get parameters
			;  function on every read sector function call.

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
			; No checks for overflows are included here.

		mov	dl, [bp+24h]	; Hard disk number
		mov	ax, 0201h	; Read (AH=02) 1 Sector (AL=01)
		int	13h
		jnb	.end
			; (Only falls through if CY.)
.reset:
		pop	cx
		dec	cx
			; Bug: The dec instruction never modifies CF.
			;  Therefore, we will always have CY set here.
			;  So the following conditional jump always jumps,
			;  not just when the repeat count is exhausted.
			; Solution: Replace next jb by a jz, and increase
			;  the initialisation of the repeat count by 1.
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
			; This is not in the correct format nor pointed to
			;  as expected by the MS-DOS v7 initial loader. To be
			;  fair, even Microsoft got their message table wrong
			;  in their original loaders. (dosemu2 and lDebug are
			;  likely the only loaders that get it right. The
			;  ldosboot loaders also do not provide the table.)

	;
	;  "1234567 123"
IO_SYS:	
	DB "WINBOOT SYS"
	DB "IO      SYS"
	DB 0
	;

	times 510 - ($ - $$) db 0

	DB	055h, 0AAh
