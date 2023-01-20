cpu 8086

section COMPACT_MBR_IPL class=BOOT align=16

global _COMPACT_MBR_IPL
global _EMP_IPL
global _EMP_IPL_SIZE
global _STD_IPL

_COMPACT_MBR_IPL:
	;
	;  BIOS loads MBR at 0000:7C00h
	;
	;  Lets move code to 0000:0600h
	;
	xor	bp, bp
	mov	ds, bp
	mov	es, bp
	mov	ss, bp		;  CPU clears interrupt flag for next command
	mov	sp, 7C00h
	cld
	mov	si, sp
	mov	di, 0600h
	mov	cx, 0100h
	rep
	 movsw
;	jmp	.ENTRY
		DB	0E9h
		DW	0600h-7C00h+.ENTRY-($+2)
	;
VIRUS	EQU	(M1-_COMPACT_MBR_IPL-100h)
ERROR	EQU	(M2-_COMPACT_MBR_IPL-100h)
BOOTING	EQU	(M3-_COMPACT_MBR_IPL-100h)
HD_NUM	EQU	(M4-_COMPACT_MBR_IPL-100h)
WRITING	EQU	(M5-_COMPACT_MBR_IPL-100h)
NL	EQU	(M6-_COMPACT_MBR_IPL-100h)
	;
.ENTRY:
	;
	;  Check interrupt vectors
	;
	cmp	byte [di-800h+7B8h], 0
	je	.vir3
	
	mov	bx, 4*1Ch+3		;  Check interrupts 0h to 1Ch
.vir1:
	sub	bx, 4
	jc	.vir3
	cmp	byte [bx], 0C0h	;  They must be >= C000:0000h
	jae	.vir1

	mov	bl, VIRUS	; Otherwize print message
	call	PRINT
.vir2:
	mov	ah, 0
	int	16h		;  Get a key
	cmp	al, 0Dh
	jne	.vir2		;  And loop until ENTER is pressed
	mov	bl, NL
	call	PRINT
.vir3:
	;
	;
	;
	cmp	dl, 80h
	jae	.hd1
	mov	dl, 80h
.hd1:
	mov	al, dl
	sub	al, 80h-'1'
	mov	[di-800h+700h+HD_NUM], al

	mov	bl, BOOTING
	call	PRINT
	
	mov	al, [di-800h+7B9h]	; Default partition name
	cmp	al, 0
	jne	.m4

	;
	;  Find active partition
	;
.pt1:
	mov	al, '1'
	mov	bh, 00h
	mov	di, 07BEh
.pt2:
	cmp	[di], bh ; 00
	mov	[di], dl
	jnz	.pt3			; Active partition found
	mov	[di], bh ; 00
	add	di, 10h
	inc	ax
	cmp	al, '4'
	jbe	.pt2
	;
	mov	di, 07BAh		; No active partition found
	cmp	[di], bh ; 00
	mov	al, 26		; Booting from the next hard drive
	jne	.pt3
	mov	al, 'A'		; Booting from the first floppy disk (A)
	xor	bp, bp		; Don't save changes in this case
.pt3:
	mov	ah, 0Eh
	int	10h		; Print a character
	;
	;  Load boot sector from disk
	;
	mov	si, 3		; We will try at most three times
.rd1:
	push	dx
	mov	ax, 0201h	; Read (AH=02) 1 Sector (AL=01)
	mov	bx, 7C00h	; Destination Address
	mov	cx, [di+02]	; Cyl#, Sect#
	mov	dx, [di+00]	; Head, Drive in DL
	int	13h
	pop	dx
	jnc	.rd2
				; We get here if there was an error
	mov	ah, 0		; We will try to reset device
	int	13h

	dec	si
	jnz	.rd1
	stc			; We have tried three times, so we will give up
.rd2:
	mov	bl, ERROR
	jc	.m2		; I/O Error!
	call	DOTS
	jnc	.save		; Time out or user hit ESC
	mov	bl, 0
	cmp	al, ' '
	jne	.m4
	;
	; Select new partition
	;
.m1:
	mov	bl, BOOTING
.m2:
	call	PRINT
.m3:
	mov	ah, 00h
	int	16h		; Read character
.m4:
	mov	si, 7BAh
	mov	ah, 0

	cmp	al, 'A'		; Floppy disk
	je	.m6
	cmp	al, 'a'
	je	.m6

	mov	ah, dl
	inc	ah
	
	cmp	al, 09		; Tab - Next hard drive
	je	.m6

	sub	al, '1'		; One of the partitions
	cmp	al, 04h
	jb	.m5
	or	bl, bl
	jz	.save		; wrong key, but not for us
	jmp	.m3		; wrong key, so lets ask again
.m5:
	mov	si, 7BEh
	mov	ah, 10h
	mul	ah
	add	si, ax
	mov	ah, dl

	cmp	byte [si], ah
	jne	.m6
	dec	bp
.m6:
	or	bl, bl
	jz	.m1

	mov	di, 07BEh
	mov	cx, 4
.cl1:
	mov	byte [di], 0
	add	di, 10h
	loop	.cl1
	
	mov	[si], ah
	inc	bp
	jmp	.pt1
	;
	; Save changes to disk
	;
.save:
	or	bp, bp			; Do we have to save (0=don't)
	je	.boot
	mov	bl, WRITING
	call	PRINT
	mov	ax, 0301h		; Write (AH=03) 1 Sector (AL=01)
	mov	bx, 0600h		; Source located at address 600h
	mov	cx, 0001h		; Cyl = 0, Sect = 1
	mov	dh, 00h			; Head = 0, Drive in DL
	int	13h
.boot:
	mov	dl, [di]		; Boot sector expects Drive# in DL
;	jmp	7C00h			; Transfer control to loaded BootSector
		DB	0E9h
		DW	7C00h-600h-($+2-_COMPACT_MBR_IPL)


	;
	;  PRINT
	;
PRINT:
	mov	ah, 0Eh
	mov	bh, 00h
	lea	si, [bx+700h]
.pr1:
	lodsb
	or	al, al
	jz	.pr2
	int	10h
	jmp	.pr1
.pr2:
	ret


	;
	;  Print dots and check if key is pressed (key in al)
	;
DOTS:
	push	dx
	mov	ax, 092Eh	; Print '.'
	mov	bx, 0007h
	mov	cx, [bx-7+7B6h]	; Here is how many dots will be printed
	int	10h
W1:
	mov	ah, 01h		; Check if key is pressed
	int	16h
	jz	W3		; There is no key waiting

	cmp	al, 1Bh		; ESC
	je	W2
	stc
	jmp	short W5
W2:
	mov	ah, 00		; Remove ESC from keyboard buffer
	int	16h
	jmp	short W4
W3:
	mov	ax, 0E20h
	int	10h		; Print a SPACE

	push	bx
	push	cx
	mov	ah, 0
	int	1Ah		; Read System Timer
	mov	bx, dx
WT1:	int	1Ah		; Wait one timer tick
	cmp	bx, dx
	je 	WT1
	mov	bx, dx
WT2:	int	1Ah		; Wait another timer tick
	cmp	bx, dx
	je 	WT2
	pop	cx
	pop	bx

	loop	W1
W4:
	clc
W5:
	push	ax
	pushf
	mov	ax, 0920h	; Print rest of SPACEs
	int	10h
	mov	bl, NL
	call	PRINT
	popf
	pop	ax

	pop	dx
	ret

	;
M1:	DB "Virus!!! ",0
M2:	DB " Error!",0Dh,0Ah
M3:	DB "Booting from: HD"
M4:	DB "1/",0
M5:	DB "Writing changes..."
M6:	DB 0Dh,0Ah,0
	;

GAP: times (01b6h-($-_COMPACT_MBR_IPL)) db 0

DOT_LEN	DW	30h
VIR_CHK	DB	01
ST_PART	DB	00
FD_PARM	DB	00,00,01,00	; Floppy disk boot sector parameters
	DB	40h DUP(0)
	DB	055h, 0AAh


section EMPTY_MBR_IPL class=BOOT align=16

_EMP_IPL:
		;
		;  Dummy boot record to display message like
		;
		;  "Partition is not bootable ..."
		;
		;  MBR loads this boot record at 0000:7C00h
		;
		mov	ax, cs
		mov	ds, ax
		mov	ss, ax
		mov	sp, 7C00h
		jmp	short	.p3
.p0:
		pop	si
		mov	ah, 0Eh
		mov	bh, 00h
.p1:
		cld
		lodsb
		or	al, al
		jz	.p2
		int	10h
		jmp	.p1
.p2:
		xor	ax, ax
		int	16h
		mov	ax, 0E0Dh
		int	10h
		mov	al, 0Ah
		int	10h
		int	19h
		;
.p3:
		call	.p0
_EMP_IPL_END:
		db 	0


section EMPTY_MBR_DATA class=DATA align=2
GROUP DGROUP EMPTY_MBR_DATA

_EMP_IPL_SIZE: dw (_EMP_IPL_END-_EMP_IPL)

section STD_MBR_IPL class=BOOT align=16

_STD_IPL:

		;
		;   IPL that comes with MS-DOS's fdisk.exe
		;

		CLI
		XOR	AX,AX
		MOV	SS,AX
		MOV	SP,7C00h
		MOV	SI,SP
		PUSH	AX
		POP	ES
		PUSH	AX
		POP	DS
		STI
		CLD
		MOV	DI,0600h
		MOV	CX,0100h
		REPNZ
		MOVSW
	;	JMP	0000:061D
		DB	0EAh
		DW	061Dh, 0000h
._061D:
		MOV	SI,07BEh
		MOV	BL,04h
._0622:
		CMP	BYTE [SI],80h
		JZ	._0635
		CMP	BYTE [SI],00h
		JNZ	._0648
		ADD	SI,10h
		DEC	BL
		JNZ	._0622
		INT	18h
._0635:
		MOV	DX,[SI]
		MOV	CX,[SI+02]
		MOV	BP,SI
._063C:
		ADD	SI,10h
		DEC	BL
		JZ	._065D
		CMP	BYTE [SI],00
		JZ	._063C
._0648:
		MOV	SI,068Bh
._064B:
		LODSB
		CMP	AL,00
		JZ	._065B
		PUSH	SI
		MOV	BX,0007
		MOV	AH,0Eh
		INT	10h
		POP	SI
		JMP	._064B
._065B:
		JMP	._065B
._065D:
		MOV	DI,0005h
._0660:
		MOV	BX,7C00h
		MOV	AX,0201h
		PUSH	DI
		INT	13h
		POP	DI
		JNB	._0678
		XOR	AX,AX
		INT	13h
		DEC	DI
		JNZ	._0660
		MOV	SI,06A3h
		JMP	._064B
._0678:
		MOV	SI,06C2h
		MOV	DI,7DFEh
		CMP	WORD [DI],0AA55h
		JNZ	._064B
		MOV	SI,BP
	;	JMP	0000:7C00
		DB	0EAh
		DW	7C00h, 0000h

._068B:	DB	"Invalid partition table",0
._06A3:	DB	"Error loading operating system",0
._06C2:	DB	"Missing operating system",0

		DB	293 DUP(0)

