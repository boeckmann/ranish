.MODEL LARGE
ASSUME	CS:ADV_MAN_TEXT, DS:BOOT_DATA, ES:BOOT_DATA, SS:BOOT_DATA
PUBLIC _ADV_IPL, _ADV_MANAGER, _encrypt_password
LOCALS

SECT_SIZE	EQU	  512

MAX_MENU_ROWS	EQU	   16
MAX_PART_ROWS	EQU	   32

ADV_CODE_SIZE	EQU	 8192
ADV_DATA_SIZE	EQU	 2048

ADV_CODE_SECT	EQU	(ADV_CODE_SIZE/SECT_SIZE)
ADV_DATA_SECT	EQU	(ADV_DATA_SIZE/SECT_SIZE)

ADV_CODE_ADDR	EQU	(800h)
ADV_DATA_ADDR	EQU	(800h+ADV_CODE_SIZE)

ADV_MAGIC_VALUE	EQU	0ABCDh

ADV_OPT_VIR_CHECK	EQU	1
ADV_OPT_CLEAR_SCR	EQU	2
ADV_OPT_DEF_MENU	EQU	4
ADV_OPT_IGN_UNUSED	EQU	8

OS_HIDDEN	EQU	0FF80h
OS_ADV		EQU	0FF81h
OS_UNKN		EQU	0FFFFh

M_BOOT_EMPTY	EQU	0
M_BOOT_PART	EQU	1
M_BOOT_NEXT_HD	EQU	2
M_BOOT_FLOPPY	EQU	3

SHOW_ONE	EQU	0
SHOW_LAST	EQU	1
SHOW_NEXT	EQU	2
SHOW_PREV	EQU	3
SHOW_LAST3	EQU	4

M_OPT_PASSW	EQU	1

INCLUDE COLORS.INC

	X	EQU	30
	Y	EQU	2
	W	EQU	50
;	H	EQU	16

	KEYS_X	EQU	(X+4)
;	KEYS_Y	EQU	(Y+H-2)
	KEYS_W  EQU	(31+12)

	DOT_X	EQU	(KEYS_X+11)
	DOT_Y	EQU	(KEYS_Y)
	DOT_NUM	EQU	(31)


	DOT1	EQU	07h
	DOT2	EQU	0FAh

TITLE_COLOR	EQU	Yellow+BakBlack
MENU_COLOR	EQU	BrWhite+BakBlack
ACTIVE_COLOR	EQU	Black+BakCyan
KEYS_KEY_COLOR	EQU	Yellow+BakBlack
KEYS_TXT_COLOR	EQU	BrCyan+BakBlack
BORDER_COLOR	EQU	BrGreen+BakBlack	; Yellow+BakBlue
DOTBAR_COLOR	EQU	BrWhite+BakBlack


BOOT_DATA	SEGMENT	AT 0h

		ORG 600h

MBR_SECT	DB 	512 Dup(?)

		ORG 7B0h

ADV_REL_SECT	DD	?
ADV_RESERVED	DD	?
adv_act_menu	DB	?
adv_boptions	DB	?
adv_abmmagic	DB	4 Dup(?)

mbr_part_rec	STRUC		; 16 bytes
  b_boot_flag	DB	?
  b_chs_start	DB	3 Dup(?)
  b_os_id	DB	?
  b_chs_end	DB	3 Dup(?)
  b_rel_sect	DD	?
  b_num_sect	DD	?
ENDS

part_rec	mbr_part_rec 	4 DUP(?)

		ORG	ADV_DATA_ADDR

ADV_SIGNATURE	DB	15 Dup(?) ; "AdvBootManager",0
ADV_VERSION	DB	?
adv_def_menu	DB	?
adv_timeout	DB	?
adv_options	DB	?
adv_options2	DB	?
adv_password	DW	?
adv_reserved2	DB	26 Dup(?)
adv_title	DB	32 Dup(?)

adv_menu_rec	STRUC			; 80 bytes
  m_type	DB	 ?
  m_options	DB	 ?
  m_name	DB	30 Dup(?)
  m_tag		DB	 ?
  m_show	DB	 ?
  m_reserved	DB	14 Dup(?)
  m_num_keys	DW	 ?
  m_keys    	DW	15 Dup(?)
ENDS

adv_part_rec	STRUC			; 16 bytes
  p_os_id	DW	?
  p_tag		DB	?
  p_orig_row	DB	?
  p_reserved	DB	4 Dup(?)
  p_rel_sect	DD	?
  p_num_sect	DD	?
ENDS

menu		adv_menu_rec	MAX_MENU_ROWS  Dup(?)
part		adv_part_rec	MAX_PART_ROWS  Dup(?)

		ORG	ADV_DATA_ADDR + ADV_DATA_SIZE

NUM_DISKS	DB	?
DISK		DB	?
DISK_NUM_CYLS	DW	?
DISK_NUM_HEADS	DW	?
DISK_NUM_SECTS	EQU	SECT_PER_TRACK
SECT_PER_CYL	DW	?
SECT_PER_TRACK	DW	?

_ScreenArea	DD   ?	; B800h:0000h
_ScreenWidth	DB   ?	;  80
_ScreenHeight	DB   ?	;  25
_ScreenLength	DW   ?	; 80*25

NUM_MENUS	DW	?
ACT_MENU	DW	?
MENU_PTR	DW	MAX_MENU_ROWS Dup(?)
PART_PTR	DW	MAX_MENU_ROWS Dup(?)

ACT_MBR_REC	DW	?
ACT_PART_PTR	DW	?
PART_TMP	adv_part_rec	4 Dup(?)
PART_TMP2	adv_part_rec	?
MBR_SECT_BAK	DB	512 Dup(?) 

TMP		DB	  80 Dup(?)
SAV_BUFFER	DB	4096 Dup(?)
SAV_BUFFER_ERR	DB	 640 Dup(?)
CURSOR_SAVE_XY	DW	?
TICKS_PER_DOT	DW	?
H		DB	?
KEYS_Y		DB	?
PASS_VALIDATED	DW	?
FILL_KB_BUFFER	DW	?
ALT_ENTER	DW	?
LAST_PART	DW	?
IMPORTED_FLAG	DW	?
FD_PARAMS	DB	4 Dup(?)


STACK_AREA	DB	1024	Dup(?)	; Reserving at list 1k for stack

		ORG	7C00h
		
OS_BOOT_SECT	DB	SECT_SIZE Dup(?)


BOOT_DATA	ENDS


;----------------------------------------------------------------------
PUSH_REGS	MACRO
		push	ax
		push	bx
		push	cx
		push	dx
		push	di
		push	si
		push	ds
		push	es
ENDM
;----------------------------------------------------------------
POP_REGS	MACRO
		pop	es
		pop	ds
		pop	si
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	ax
ENDM
;----------------------------------------------------------------

ADV_MAN_TEXT	SEGMENT	PARA PRIVATE	'CODE'

		mov	ax, cs
		mov	ds, ax
		mov	es, ax
		mov	ss, ax
		mov	sp, 7C00h
		xor	cx, cx
		mov	dl, 80h
		mov	Word Ptr ADV_REL_SECT, 7
		mov	Word Ptr ADV_REL_SECT+2, 0
		jmp	debug_entry

		ORG	600h

_ADV_IPL	PROC	NEAR
		;
		;  BIOS loads MBR at 0000:7C00h
		;
		;  Lets move code to 0000:0600h
		;
		xor	ax, ax
		mov	ds, ax
		mov	es, ax
		mov	ss, ax	;  CPU clears interrupt flag for next command
		mov	sp, 7C00h
		cld
		mov	si, sp
		mov	di, 0600h
		mov	cx, 0100h
		rep
		 movsw
	;	jmp	0000:@@ENTRY
		DB	0EAh
		DW	(@@ENTRY-_ADV_IPL+600h), 0000
		;
		;
ErrBootMan	EQU	(@@M1-_ADV_IPL+600h)
BootAorHD	EQU	(@@M2-_ADV_IPL+600h)
ErrBootSct	EQU	(@@M3-_ADV_IPL+600h)
MsgNL		EQU	(@@M4-_ADV_IPL+600h)
		;
@@M1:		DB "Cannot load Boot Manager.",0Dh,0Ah
@@M2:		DB "Boot from A or hard disk? ",0
@@M3:		DB "Error reading Boot Sector."
@@M4:		DB 0Dh,0Ah,0
		;
ADV_CODE_CHECK_SUM	DW	0
		;
debug_entry:
@@ENTRY:
;GET_DISK_INFO	PROC	NEAR
		;
		;  dl - disk number
		;
		cmp	dl, 80h
		jae	@@skip
		mov	dl, 80h
@@skip:
		mov	DISK, dl
		mov	cl, 3
@@get_again:
		mov	ah, 08		; Get disk parameters
		int	13h
		jnc	@@eval
		mov	ah, 0
		int	13h
		loop	@@get_again
		jmp	@@err1		; Error
@@eval:
;		mov	NUM_DISKS, dl
		xor	ah, ah
		mov	al, dh
		inc	ax		; AX - Number of HEADS (SIDES)
		mov	DISK_NUM_HEADS, ax 
		xor	bh, bh
		mov	bl, cl
		and	bl, 3Fh		; BX - Number of SECTORS / TRACK
		mul	bx
		mov	SECT_PER_CYL, ax
		mov	SECT_PER_TRACK, bx

		mov	bl, ch
		shl	cx, 1
		shl	cx, 1
		and	ch, 3
		mov	bh, ch
		inc	bx
		mov	DISK_NUM_CYLS, bx
;GET_DISK_INFO	ENDP
		;
		;	Read Advanced Boot Manager Data and Code
		;
		mov	ax, Word Ptr ADV_REL_SECT
		mov	dx, Word Ptr ADV_REL_SECT+2

		mov	bx, ADV_DATA_ADDR
		mov	cx, ADV_DATA_SECT
		
		call	READ_N_SECT
		jc	@@err1
		
		add	ax, ADV_DATA_SECT
		adc	dx, 0

		mov	bx, ADV_CODE_ADDR
		mov	cx, ADV_CODE_SECT

		call	READ_N_SECT
		jc	@@err1

		cmp	[bx], ADV_MAGIC_VALUE
		jne	@@err1
		;
		jmp	adv_code_entry_point
	;	jmp	0000h:0800h		; Jump to Boot Manager
	;	DB	0EAh
	;	DW	0800h,0000h
		;
bad_boot_man:
@@err1:
		mov	si, ErrBootMan
@@err2:
		call	PRINT
		mov	ah, 0
		int	16h		;  Get a key
		mov	si, MsgNL
		call	PRINT
		cmp	al, 'A'
		je	@@floppy	;  Boot from floppy
		cmp	al, 'a'
		je	@@floppy
		;
		;  Find active partition on hard disk
		;
@@part_tab:
		mov	cx, 4
		mov	di, 7BEh		; Address of the first record
@@next_part:
		cmp	Byte Ptr [di], 00
		jnz	@@read_hd		; Active partition found
		add	di, 10h
		loop	@@next_part
@@floppy:					; No active partition found
		xor	dx, dx
		;mov	dx, 0000		; Lets boot from floppy
		mov	cx, 0001
		jmp	@@read_sect
@@read_hd:
		mov	dx, [di]
		mov	cx, [di+2]
@@read_sect:
		mov	bx, 7C00h
		call	READ_SECT
		jnc	@@go_boot
		;
		mov	si, ErrBootSct
		call	PRINT
		mov	si, BootAorHD
		jmp	@@err2
		;
@@go_boot:
		mov	dl, [di]	; Boot sector expects Drive# in DL
	;	jmp	0000h:7C00h	; Transfer control to loaded BootSector
		DB	0EAh
		DW	7C00h,0000h
		;
		;
		;
REL_SECT_TO_CHS	PROC	NEAR
		;
		;  Input:    DX:AX - relative sector
		;  Output:   DH,CX - CHS and DL - disk
		;  Destroys: AX
		;
		div	SECT_PER_CYL		; AX=CYL, DX=SECT on CYL
		mov	cx, ax
		shr	cx, 1
		shr	cx, 1
		and	cl, 0C0h
		mov	ch, al
		mov	ax, dx
		xor	dx, dx
		div	SECT_PER_TRACK		; AX=HEAD, DX=SECT
		mov	dh, al
		inc	dl
		and	dl, 3Fh
		or	cl, dl
		mov	dl, DISK
		ret
REL_SECT_TO_CHS	ENDP
		;
		;
READ_N_SECT	PROC	NEAR
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
@@next_sect:
		push	ax
		push	cx
		push	dx
		call	REL_SECT_TO_CHS
		call	READ_SECT
		pop	dx
		pop	cx
		pop	ax
		jc	@@end

		add	ax, 1
		adc	dx, 0

		add	bx, SECT_SIZE

		loop	@@next_sect
@@end:
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret
READ_N_SECT	ENDP
		;
		;
		;
READ_SECT	PROC	NEAR
		;
		;   ES:BX - Address
		;   CX,DX - CHS
		;
		;   Returns: CF set if error
		;
		push	si
		mov	si, 3		; We will try at most three times
@@try_again:
		mov	ax, 0201h	; Read (AH=02) 1 Sector (AL=01)
		int	13h
		jnc	@@end
					; We get here if there was an error
		mov	ah, 0		; We will try to reset device
		int	13h
	
		dec	si
		jnz	@@try_again
		;
		; We have tried three times, so we will give up
		;
		stc
	@@end:
		pop	si
		ret
READ_SECT	ENDP
		;
		;
PRINT		PROC	NEAR
		;
		;  ds:si - address of null terminated string to print
		;
		push	ax
		push	bx
		push	si
		mov	ah, 0Eh
		mov	bh, 00h
@@pr1:
		lodsb
		or	al, al
		jz	@@pr2
		int	10h
		jmp	@@pr1
@@pr2:
		pop	si
		pop	bx
		pop	ax
		ret
PRINT		ENDP
		;
		;
GAP1		PROC
GAPLEN1		EQU	(01B0h-(GAP1-_ADV_IPL))
IF GAPLEN1
		DB	GAPLEN1 DUP(0)	
ENDIF
GAP1		ENDP

ADV_MBR_MISC	DB	14+64+2 Dup(0)

_ADV_IPL	ENDP
		;
		;
		;
		;
_ADV_MANAGER	PROC	NEAR
		;
		;  IPL loads MANAGER at 0000h:0800h = 0800h
		;
		;               SS:SP = 0000h:7C00h = 7C00h
		;
ADV_MAGIC_NUM	DW	ADV_MAGIC_VALUE
		;
adv_code_entry_point:
		;
		;  First of all lets check integrity of code
		;
CHECK_CODE_INTEGRITY	PROC	NEAR
		xor	bx, bx
		mov	cx, ADV_CODE_SIZE
		shr	cx, 1
		mov	si, ADV_CODE_ADDR
@@add_next_word:
		lodsw
		add	bx, ax
		loop	@@add_next_word
		cmp	ADV_CODE_CHECK_SUM, 0
		je	@@initialize_sum
		cmp	ADV_CODE_CHECK_SUM, bx
		je	@@check_data_header
		jmp	bad_boot_man
@@initialize_sum:
		mov	ADV_CODE_CHECK_SUM, bx
@@check_data_header:
		mov	cx, 15
		lea	si, ADV_SIGNATURE
		lea	di, ADV_SIGNATURE2
		repe
		  cmpsb
		je	@@data_header_valid
		jmp	bad_boot_man
@@data_header_valid:
CHECK_CODE_INTEGRITY	ENDP
		;
CHECK_INTERRUPT_VECTORS	PROC
		test	adv_options, ADV_OPT_VIR_CHECK
		jz	@@ints_okay
		mov	cx, 1Dh			;  Check interrupts 0h to 1Ch
		mov	bx, 3
@@next_int:
		cmp	byte ptr [bx], 0C0h	;  They must be >= C000:0000h
		jb	@@int_changed
		add	bx, 4
		loop	@@next_int

		mov	bx, 4Ah*4-1		; int 4Ah - User Alarm
		cmp	byte ptr [bx], 0C0h
		jb	@@int_changed

		mov	bx, 70h*4-1		; int 70h - Real-Time Clock
		cmp	byte ptr [bx], 0C0h
		jb	@@int_changed
		
		jmp	@@ints_okay
@@int_changed:
		lea	si, VirusWarning
		call	PRINT
@@wait_enter:
		mov	ah, 0
		int	16h		;  Get a key
		cmp	al, 0Dh
		jne	@@wait_enter	;  And loop until ENTER is pressed
		lea	si, VirusNL
		call	PRINT
@@ints_okay:
CHECK_INTERRUPT_VECTORS	ENDP
		;
		;
		jmp	@@start
		;
		;   Messages
		;
ADV_SIGNATURE2	DB  "AdvBootManager",0
Border		DB  "ÉÍ»º ºÈÍ¼"
Border1		DB  "ÚÄ¿³ ³ÀÄÙ"
Mesg_OK		DB  " OK ",0
BottomKeysESC	DB  "ESC",0
BottomKeysText	DB  "ESC - Boot",0
BottomKeysEnt	DB  "Enter",0
BottomKeysText2	DB  "Press Enter to boot from selected partition",0
MesgSetTimeout	DB  "Select menu timeout (+/-): ",0
MesgErrorRead	DB  "Error reading boot sector",0
MesgErrorSave	DB  "Error saving MBR to disk",0
MesgErrorSaveAdv DB "Error saving Advanced MBR to disk",0
MesgEnterPassword DB "Enter password:",0
MesgPasswordInvalid DB "Password invalid",0
MesgBootInvalid	DB  "Boot sector is invalid",0
MesgImported	DB  "One or more partitions was changed in MBR. Please, run PART.EXE",0
VirusWarning	DB  0Dh, 0Ah
		DB  "One or more interrupts does not point to BIOS.", 0Dh, 0Ah
		DB  "This may be a sign of a stealth boot virus.", 0Dh, 0Ah
		DB  "Turn the computer OFF then ON and run antivirus.", 0Dh, 0Ah
		DB  "Or press ENTER to continue booting... ", 0
VirusNL		DB  0Dh, 0Ah, 0
		;
		;
@@start:
		call	_conio_init
		call	CHECK_LAST_CYL
		call	IMPORT_NEW_PART
		call	PREP_MENU_LIST
		call	BACKUP_MBR_SECT

	@@menu:
		mov	ax, NUM_MENUS
		add	ax, 6
		mov	H, al
		add	al, Y
		sub	al, 2
		mov	KEYS_Y, al

		mov	PASS_VALIDATED, 0
		
		call	INIT_SCREEN
		call	MAIN_MENU
		call	DONE_SCREEN
		
		cmp	ALT_ENTER, 1
		jne	@@no_need_to_wait
		
		mov	cx, 24	; If user presses ALT-ENTER we will wait
	@@L3:			; for about 1.5 seconds before proceding.
		push	cx
		mov	ah, 0
		int	1Ah		; Read System Timer
		mov	bx, dx

	@@WT1:	int	1Ah		; Wait one timer tick
		cmp	bx, dx
		je 	@@WT1
		pop	cx
		loop	@@L3

@@no_need_to_wait:

		mov	di, ACT_MBR_REC
		mov	dl, [di]	; Boot sector expects Drive# in DL
	;	jmp	0000h:7C00h	; Transfer control to loaded BootSector
		DB	0EAh
		DW	7C00h,0000h
_ADV_MANAGER	ENDP
		;
		;
		;
INIT_SCREEN	PROC	NEAR

		test	adv_options, ADV_OPT_CLEAR_SCR
		jz	@@skip1
		mov	ah, White+BakBlack
		mov	bl, 1
		mov	bh, 1
		mov	dl, 80
		mov	dh, 25
		call	_clear_window
		mov	bl, 1
		mov	bh, 1
		call	_move_cursor
@@skip1:
		call	_save_cursor
		call	_hide_cursor

		mov	bl, X
		mov	bh, Y
		mov	dl, W
		mov	dh, H
		lea	si, SAV_BUFFER
		call	_save_window

		mov	ah, BORDER_COLOR
		lea	si, Border
		call	_border_window
		
		mov	ah, TITLE_COLOR
		add	bx, 0109h
		lea	si, adv_title
		call	_write_string
		
		mov	ah, KEYS_TXT_COLOR
		mov	bl, KEYS_X
		mov	bh, KEYS_Y
		lea	si, BottomKeysText2
		call	_write_string
		
		mov	ah, KEYS_KEY_COLOR
		add	bl, 6
		lea	si, BottomKeysEnt
		call	_write_string

		
		ret
INIT_SCREEN	ENDP
		;
		;
		;
DONE_SCREEN	PROC	NEAR
		mov	bl, X
		mov	bh, Y
		mov	dl, W
		mov	dh, H
		lea	si, SAV_BUFFER
		call	_load_window
		
		call	_restore_cursor
		ret
DONE_SCREEN	ENDP
		;
		;
		;
PREP_MENU_LIST	PROC	NEAR
		test	adv_options, ADV_OPT_DEF_MENU
		jz	@@no_def_menu
		mov	al, adv_def_menu
		mov	adv_act_menu, al
	@@no_def_menu:
		mov	ACT_MENU, 0
		mov	NUM_MENUS, 0
		mov	cx, 0
		lea	si, menu
		mov	di, 0
	@@next:
		cmp	[si].m_type, M_BOOT_EMPTY
		je	@@cont

		mov	MENU_PTR[di], si
		mov	al, [si].m_tag
		call	PART_NUM_BY_TAG
		jnc	@@part_ok
		cmp	[si].m_type, M_BOOT_PART
		je	@@cont
		mov	ax, 0
	@@part_ok:
		mov	PART_PTR[di], ax

		cmp	cl, adv_act_menu
		jne	@@skip
		mov	ax, NUM_MENUS
		mov	ACT_MENU, ax
	@@skip:
		add	di, 2
		inc	NUM_MENUS
	@@cont:
		add	si, SIZE adv_menu_rec
		inc	cx
		cmp	cx, MAX_MENU_ROWS
		jne	@@next
		ret
PREP_MENU_LIST	ENDP
		;
		;
		;
PART_NUM_BY_TAG	PROC	NEAR
		;
		;  Input:  AL - Tag
		;  Output: AX - Pointer to partition
		;
		;  CF - Set if part not found, AX=0
		;
		push	bx
		push	cx
		cmp	al, 0
		je	@@fail
		xor	cx, cx
		lea	bx, part
	@@next:
		cmp	[bx].p_tag, al
		jne	@@cont
		clc
		mov	ax, bx
		jmp	@@end
	@@cont:
		add	bx, SIZE adv_part_rec
		inc	cx
		cmp	cx, MAX_PART_ROWS
		jne	@@next
	@@fail:
		stc
		mov	ax, 0
	@@end:
		pop	cx
		pop	bx
		ret
PART_NUM_BY_TAG	ENDP
		;
		;
		;
MAIN_MENU	PROC	NEAR
		;
		;	Main loop		
		;
		cmp	IMPORTED_FLAG, 10h
		jb	@@no_import
		lea	si, MesgImported
		call	SHOW_ERROR
@@no_import:
		mov	di, 0
		mov	FILL_KB_BUFFER, 1
@@while1:
		mov	ax, 0
		jmp	@@cond1
	@@next1:
		call	SPRINTF_MENU
		push	ax
		mov	bl, (X+3)
		mov	bh, (Y+3)
		add	bh, al
		cmp	ax, ACT_MENU
		mov	ah, MENU_COLOR
		jne	@@norm1
		mov	ah, ACTIVE_COLOR
	@@norm1:
		lea	si, TMP
		call	_write_string
		pop	ax
		inc	ax
	@@cond1:
		cmp	ax, NUM_MENUS
		jne	@@next1


		mov	ALT_ENTER, 0

		cmp	di, 0
		jne	@@wait_key
					; First time here
		cmp	adv_timeout, 0
		je	@@wait_key

		xor	ah, ah
		mov	al, adv_timeout
		inc	ax
		shr	ax, 1
		mov	TICKS_PER_DOT, ax
		call	DOT_BAR
		jnc	@@no_keys	; no keys was pressed - time run out
		mov	ah, 10h
		int	16h
		cmp	al, ' '
		jne	@@cmp_keys
		jmp	@@wait_key
	@@no_keys:
		jmp	@@boot
	@@wait_key:
		mov	ah, 10h
		int	16h
	@@cmp_keys:
		cmp	ax, 48E0h
		je	@@up
		cmp	ax, 4800h
		je	@@up
		cmp	ax, 50E0h
		je	@@down
		cmp	ax, 5000h
		je	@@down
		cmp	ax, 47E0h
		je	@@home
		cmp	ax, 4700h
		je	@@home
		cmp	ax, 4FE0h
		je	@@end
		cmp	ax, 4F00h
		je	@@end
		cmp	ax, 3920h	; Space
		je	@@space
		cmp	ax, 1C0Dh	; Enter
		je	@@boot2short
		cmp	ax, 0E00Dh	; Enter
		je	@@boot2short
		cmp	ax, 1C00h	; Alt-Enter
		je	@@alt_boot2short
		cmp	ax, 0A600h	; Alt-Enter
		je	@@alt_boot2short
		cmp	ax, 011Bh	; ESC
		je	@@boot_esc2short
		cmp	al, 'A'
		je	@@boot_a_short
		cmp	al, 'a'
		je	@@boot_a_short
		cmp	al, 9	; Tab
		je	@@boot_d_short
		cmp	al, 'H'
		je	@@hide_all
		cmp	al, 'h'
		je	@@hide_all
		sub	al, '1'
		xor	ah, ah
		cmp	ax, NUM_MENUS
		jb	@@boot_n_short
		jmp	@@wait_key
	@@up:
		cmp	ACT_MENU, 0
		jz	@@cont2short
		dec	ACT_MENU
		jmp	@@cont2
	@@down:
		mov	ax, NUM_MENUS
		dec	ax
		jb	@@cont2short
		cmp	ACT_MENU, ax
		jnb	@@cont2short
		inc	ACT_MENU
		jmp	@@cont2
	@@home:
		mov	ACT_MENU, 0
		jmp	@@cont2
	@@end:
		mov	ax, NUM_MENUS
		dec	ax
		jb	@@cont2short
		mov	ACT_MENU, ax
		jmp	@@cont2
	@@space:
		call	SETUP_MENU
		xor	di, di
		jmp	@@while1
	@@boot_a_short:
		jmp	@@boot_a
	@@boot_d_short:
		jmp	@@boot_d
	@@boot_n_short:
		jmp	@@boot_n
	@@boot2short:
		jmp	@@boot
	@@alt_boot2short:
		jmp	@@alt_boot
	@@cont2short:
		jmp	@@cont2
	@@boot_esc2short:
		jmp	@@boot_esc
	@@hide_all:
		mov	bx, 0
		call	CHECK_PASSWORD
		jc	@@cont2short
		mov	ax, ACT_MENU
		push	ax
		mov	ax, -1
		mov	ACT_MENU, ax
		mov	dl, 0
		call	PREP_BOOT_SECT_X
		pop	ax
		mov	ACT_MENU, ax
		jnc	@@break_out_short
		jmp	@@cont2
	@@break_out_short:
		jmp	@@break_out
	@@alt_boot:
		mov	ALT_ENTER, 1
		jmp	@@boot
	@@boot_a:
		mov	bx, 0
		call	CHECK_PASSWORD
		jc	@@cont2short
		mov	dl, 0
		call	PREP_BOOT_SECT_X
		jnc	@@break_out_short
		jmp	@@cont2
	@@boot_d:
		mov	bx, 0
		call	CHECK_PASSWORD
		jc	@@cont2short
		mov	dl, DISK
		inc	dl
		call	PREP_BOOT_SECT_X
		jnc	@@break_out
		jmp	@@cont2
	@@boot_n:
		mov	ACT_MENU, ax
	@@boot:
		cmp	NUM_MENUS, 0
		je	@@boot_a
		mov	bx, ACT_MENU
		shl	bx, 1
		mov	ax, MENU_PTR[bx]
		lea	bx, menu
		sub	ax, bx
		mov	bl, SIZE adv_menu_rec
		div	bl
		mov	adv_act_menu, al
	@@boot_esc:
		cmp	NUM_MENUS, 0
		je	@@boot_a
		

		mov	bx, ACT_MENU
		shl	bx, 1
		mov	bx, MENU_PTR[bx]

		push	bx
		call	CHECK_PASSWORD
		pop	bx
		jc	@@cont2

		cmp	[bx].m_type, M_BOOT_PART
		je	@@boot_type_part
		cmp	[bx].m_type, M_BOOT_NEXT_HD
		je	@@boot_type_next_hd
		cmp	[bx].m_type, M_BOOT_FLOPPY
		je	@@boot_type_floppy
		jmp	@@cont2		; We dont know what it is - ingnoring 
@@boot_type_part:
		call	PREP_BOOT_SECT
		jnc	@@break_out
		jmp	@@cont2
@@boot_type_next_hd:
		mov	dl, DISK
		inc	dl
		call	PREP_BOOT_SECT_X
		jc	@@cont2
		call	FILL_KEYB_BUFFER
		jmp	@@break_out
@@boot_type_floppy:
		mov	dl, 0
		call	PREP_BOOT_SECT_X
		jc	@@cont2
		call	FILL_KEYB_BUFFER
		jmp	@@break_out
	@@cont2:
		mov	di, 1
		jmp	@@while1
	@@break_out:
		ret
MAIN_MENU	ENDP
		;
		;
CHECK_PASSWORD	PROC	NEAR
		;
		; Input:   bx - pointer to ActiveMenu, or 0 if no menu
		;
		; Output:  CF - clear if password validated
		;	   CF - set if user cannot proceed
		;
		PUSH_REGS

		cmp	PASS_VALIDATED, 1
		je	@@valid

		cmp	bx, 0
		jz	@@do_check

		test	[bx].m_options, M_OPT_PASSW
		jz	@@valid		; This menu item needs no validation
@@do_check:
		cmp	adv_password, 0
		je	@@valid		; No master password set

		call	GET_PASSWORD

		cmp	ax, adv_password
		je	@@valid
@@invalid:
		lea	si, MesgPasswordInvalid
		call	SHOW_ERROR
		stc
		jmp	@@end
@@valid:
		mov	PASS_VALIDATED, 1
      		clc
@@end:
		POP_REGS
		ret
CHECK_PASSWORD	ENDP
		;
		;
		;
PREP_BOOT_SECT	PROC	NEAR
		;
		;  Output:  CF - set if error
		;
		PUSH_REGS
		
		call	MAKE_PART_TAB
		
		mov	di, ACT_MBR_REC

		mov	bx, Offset OS_BOOT_SECT
		mov	dx, [di]
		mov	cx, [di+2]
		call	READ_SECT
		jc	@@err_read
		
		mov	di, Offset OS_BOOT_SECT
		mov	cx, 512
		mov	al, 0
		repe
		   scasb
		jcxz	@@sect_empty

		call	COMPARE_MBR_SECT	; did we change boot sector
		jnc	@@not_changed		; no need to save

		mov	bx, Offset MBR_SECT
		xor	dx, dx
		xor	ax, ax
		mov	cx, 1
		call	WRITE_N_SECT
		jc	@@err_save
@@not_changed:
		call	FILL_KEYB_BUFFER
		clc
		jmp	@@end
@@err_read:
		lea	si, MesgErrorRead
		call	SHOW_ERROR
		stc
		jmp	@@end
@@sect_empty:
		lea	si, MesgBootInvalid
		call	SHOW_ERROR
		stc
		jmp	@@end
@@err_save:
		lea	si, MesgErrorSave
		call	SHOW_ERROR
		stc
@@end:
		POP_REGS
		ret
PREP_BOOT_SECT	ENDP
		;
		;
PREP_BOOT_SECT_X PROC	NEAR
		;
		; Input:   DL - Disk number
		; Output:  CF - set if error
		;
		PUSH_REGS
		
		push	dx
		call	MAKE_PART_TAB
		pop	dx
		
		lea	di, FD_PARAMS
		mov	cx, 1
		mov	dh, 0
		mov	[di], dx
		mov	[di+2],cx
		mov	ACT_MBR_REC, di
		mov	bx, Offset OS_BOOT_SECT

		call	READ_SECT
		jc	@@err_read

		call	COMPARE_MBR_SECT	; did we change boot sector
		jnc	@@not_changed		; no need to save

		mov	bx, Offset MBR_SECT
		xor	dx, dx
		xor	ax, ax
		mov	cx, 1
		call	WRITE_N_SECT
		jc	@@err_save
@@not_changed:
		clc
		jmp	@@end
@@err_read:
		lea	si, MesgErrorRead
		call	SHOW_ERROR
		stc
		jmp	@@end
@@err_save:
		lea	si, MesgErrorSave
		call	SHOW_ERROR
		clc			; We still want to boot from A: (or D:)
@@end:
		POP_REGS
		ret
PREP_BOOT_SECT_X ENDP
		;
		;
		;
BACKUP_MBR_SECT PROC	NEAR
		push	cx
		push	si
		push	di

		mov	cx, 512
		mov	si, Offset MBR_SECT
		mov	di, Offset MBR_SECT_BAK
		
		rep
		  movsb

		pop	di
		pop	si
		pop	cx
		ret
BACKUP_MBR_SECT ENDP
		;
		;
		;
COMPARE_MBR_SECT PROC	NEAR
		;
		; CF - set if boot sector was changed
		;
		push	cx
		push	si
		push	di

		mov	cx, 512
		mov	si, Offset MBR_SECT
		mov	di, Offset MBR_SECT_BAK
		
		repe
		 cmpsb

		je	@@equal
		stc
		jmp	@@end
	@@equal:
		clc
	@@end:
		pop	di
		pop	si
		pop	cx
		ret
COMPARE_MBR_SECT ENDP
		;
		;
		;
MAKE_PART_TAB	PROC	NEAR
		;
		;  Input:  ACT_MENU
		;
		;  Output: ACT_MBR_REC - points to active mbr_part_rec
		;
		push	bp

		mov	ACT_PART_PTR, 0
		;
		;  Clearing PART_TMP and MBR part_rec
		;
		xor	ax, ax
		lea	di, PART_TMP
		mov	cx, SIZE PART_TMP
		rep
		  stosb
		lea	di, PART_TMP2
		mov	cx, SIZE PART_TMP2
		rep
		  stosb
		lea	di, part_rec
		mov	cx, SIZE part_rec
		rep
		  stosb
		;
		; Lets find last partition
		;
		xor	bp, bp
		xor	si, si
		lea	di, part
		mov	cx, MAX_PART_ROWS
		mov	bx, di
@@next_step:
		mov	ax, Word Ptr [di].p_num_sect
		or	ax, Word Ptr [di].p_num_sect+2
		jz	@@last_found
		add	di, SIZE adv_part_rec
		loop	@@next_step
@@last_found:
		cmp	bx, di
		je	@@show_one_je_short
		sub	di, SIZE adv_part_rec

		cmp	NUM_MENUS, 0
@@show_one_je_short:
		je	@@show_one

		mov	bx, ACT_MENU
		cmp	bx, -1
		je	@@show_unused
		shl	bx, 1
		mov	si, MENU_PTR[bx]
		mov	al, [si].m_show
		mov	si, PART_PTR[bx]
		mov	ACT_PART_PTR, si
		cmp	si, 0
		jne	@@not_zero
		xor	di, di		; No partition associated with menu
		xor	bp, bp		; so we will hide everything
		jmp	@@eval_part
	@@not_zero:
		;
		;  SI -> active partition
		;  DI -> last partition
		;  AL - representation mode
		;
		cmp	al, SHOW_NEXT
		je	@@show_next
		cmp	al, SHOW_PREV
		je	@@show_prev
		cmp	al, SHOW_LAST
		je	@@show_last
		cmp	al, SHOW_LAST3
		je	@@show_last3
		jmp	@@show_one
@@show_prev:
		lea	bx, part
		cmp	bx, si
		je	@@show_one
		mov	di, si
		sub	si, SIZE adv_part_rec
		jmp	@@eval_part
@@show_next:
		cmp	si, di
		je	@@show_one
		mov	di, si
		add	di, SIZE adv_part_rec
		jmp	@@eval_part
@@show_last:
		cmp	si, di
		jne	@@eval_part
		jmp	@@show_one
@@show_last3:
		push	di
		sub	di, SIZE adv_part_rec
		sub	di, SIZE adv_part_rec
		cmp	di, Offset part
		jnb	@@now_check_si
		pop	di
		jmp	@@show_last
@@now_check_si:
		cmp	si, di
		pop	di
		jb	@@show_last
		mov	bp, di
		sub	di, SIZE adv_part_rec
		mov	si, di
		sub	si, SIZE adv_part_rec
		jmp	@@eval_part
@@show_one:
		xor	di, di
		jmp	@@eval_part
@@show_unused:					; Show first unused space
		xor	di, di			; greater than 63 sectors
		lea	si, part
		mov	cx, MAX_PART_ROWS
@@unused_next:
		cmp	[si].p_os_id, 0
		jne	@@unused_cont
		cmp	Word Ptr [si].p_num_sect+2, 0
		jne	@@eval_part
		cmp	Word Ptr [si].p_num_sect, 63
		ja	@@eval_part
@@unused_cont:
		add	si, SIZE adv_part_rec
		loop	@@unused_next
		xor	si, si			; didn't find any
@@eval_part:
		;
		;  SI -> first partition, or zero
		;  DI -> second partition, or zero
		;  BP -> third partition, or zero
		;
		lea	bx, part
		mov	ax, Word Ptr [bx].p_rel_sect
		mov	dx, Word Ptr [bx].p_rel_sect+2
		
		lea	bx, PART_TMP
		mov	cx, 0

		mov	LAST_PART, 0
@@next_row:
		cmp	si, 0
		jne	@@check_gap
		jmp	@@tail
@@check_gap:
		cmp	Word Ptr [si].p_rel_sect+2, dx
		ja	@@fill_gap
		cmp	Word Ptr [si].p_rel_sect, ax
		ja	@@fill_gap

		mov	ax, [si].p_os_id
		mov	[bx].p_os_id, ax
		mov	al, [si].p_orig_row
		mov	[bx].p_orig_row, al
		mov	al, 0
		cmp	si, ACT_PART_PTR
		jne	@@not_active
		mov	al, DISK
@@not_active:
		mov	[bx].p_tag, al

		mov	ax, Word Ptr [si].p_rel_sect
		mov	dx, Word Ptr [si].p_rel_sect+2
		mov	Word Ptr [bx].p_rel_sect, ax
		mov	Word Ptr [bx].p_rel_sect+2, dx
		mov	ax, Word Ptr [si].p_num_sect
		mov	dx, Word Ptr [si].p_num_sect+2
		mov	Word Ptr [bx].p_num_sect, ax
		mov	Word Ptr [bx].p_num_sect+2, dx
		add	ax, Word Ptr [si].p_rel_sect
		adc	dx, Word Ptr [si].p_rel_sect+2

		mov	LAST_PART, si
		mov	si, di
		mov	di, bp
		mov	bp, 0
		add	bx, SIZE adv_part_rec
		inc	cx
		cmp	cx, 4
		je	@@table_filled_short
		jmp	@@next_row
@@table_filled_short:
		jmp	@@table_filled
@@fill_gap:
		test	adv_options, ADV_OPT_IGN_UNUSED
		jz	@@go_fill_gap
		cmp	LAST_PART, 0
		je	@@go_fill_gap
		push	si
		sub	si, SIZE adv_part_rec
		cmp	si, LAST_PART
		pop	si
		jne	@@go_fill_gap
		mov	ax, Word Ptr [si].p_rel_sect
		mov	dx, Word Ptr [si].p_rel_sect+2
		jmp	@@next_row
@@go_fill_gap:
		mov	[bx].p_os_id, OS_HIDDEN
		mov	[bx].p_tag, 0
		mov	[bx].p_orig_row, 0
		mov	Word Ptr [bx].p_rel_sect, ax
		mov	Word Ptr [bx].p_rel_sect+2, dx
		mov	ax, Word Ptr [si].p_rel_sect
		mov	dx, Word Ptr [si].p_rel_sect+2
		sub	ax, Word Ptr [bx].p_rel_sect
		sbb	dx, Word Ptr [bx].p_rel_sect+2
		mov	Word Ptr [bx].p_num_sect, ax
		mov	Word Ptr [bx].p_num_sect+2, dx
		mov	ax, Word Ptr [si].p_rel_sect
		mov	dx, Word Ptr [si].p_rel_sect+2
		
		add	bx, SIZE adv_part_rec
		inc	cx
		cmp	cx, 4
		je	@@table_filled
		jmp	@@next_row
@@tail:
		mov	Word Ptr [bx].p_rel_sect, ax
		mov	Word Ptr [bx].p_rel_sect+2, dx
		mov	ax, SECT_PER_CYL
		mov	dx, DISK_NUM_CYLS
		mul	dx
		sub	ax, Word Ptr [bx].p_rel_sect
		sbb	dx, Word Ptr [bx].p_rel_sect+2
		mov	Word Ptr [bx].p_num_sect, ax
		mov	Word Ptr [bx].p_num_sect+2, dx
		or	ax, dx				; is part size zero
		jz	@@table_filled

		mov	[bx].p_os_id, OS_HIDDEN
		mov	[bx].p_tag, 0
		mov	[bx].p_orig_row, 0

		cmp	bx, offset PART_TMP
		jne	@@table_filled
		mov	al, DISK
		mov	[bx].p_tag, al
@@table_filled:
		;
		;	Now lets make sure that active partition occupies
		;	slot in partition table from which it was taken
		;
@@check_locations:
		mov	cx, 4
		lea	si, PART_TMP
@@check_next_part:
		mov	ah, 0
		mov	al, [si].p_orig_row
		cmp	al, 0
		jne	@@row_is_not_0
		add	si, SIZE adv_part_rec
		loop	@@check_next_part
		jmp	@@nothing_to_move
@@row_is_not_0:
		dec	ax
		mov	bl, SIZE adv_part_rec
		mul	bl
		lea	bx, PART_TMP
		add	bx, ax
		cmp	bx, si
		je	@@part_in_place

		push	bx	; that's where partition supposed to be
		push	si	; that's where it is
		lea	di, PART_TMP2
		mov	cx, SIZE adv_part_rec
		rep		; moving partition to PART_TMP2
		  movsb
		pop	di
		pop	si
		push	si
		mov	cx, SIZE adv_part_rec
		rep		; moving whatever occupies place out
		  movsb
		pop	di
		push	di
		lea	si, PART_TMP2
		mov	cx, SIZE adv_part_rec
		rep		; moving partition to its proper place
		  movsb
		pop	si
@@part_in_place:
		mov	[si].p_orig_row, 0	; so we won't move it again
		jmp	@@check_locations
@@nothing_to_move:
		;
		;	Converting temporary structures
		;	into records in the partition table
		;
		mov	cx, 4
		lea	si, PART_TMP
		lea	di, part_rec
@@next_part_rec:
		push	cx
		push	si
		push	di
		cmp	[si].p_os_id, 0
		je	@@cont
		mov	ax, Word Ptr [si].p_rel_sect
		mov	dx, Word Ptr [si].p_rel_sect+2
		push	dx
		push	ax
		call	REL_SECT_TO_CHS
		mov	dl, [si].p_tag
		or	dl, dl
		jz	@@part_not_active
		mov	ACT_MBR_REC, di
@@part_not_active:
		mov	ax, dx
		stosw
		mov	ax, cx
		stosw
		pop	ax
		pop	dx
		add	ax, Word Ptr [si].p_num_sect
		adc	dx, Word Ptr [si].p_num_sect+2
		sub	ax, 1
		sbb	dx, 0
		call	REL_SECT_TO_CHS
		mov	dl, Byte Ptr [si].p_os_id+1
		mov	ax, dx
		stosw
		mov	ax, cx
		stosw
		lea	si, [si].p_rel_sect
		movsw
		movsw
		movsw
		movsw
	@@cont:
		pop	di
		pop	si
		add	si, SIZE adv_part_rec
		add	di, SIZE mbr_part_rec
		pop	cx
		loop	@@next_part_rec
	@@end:
		pop	bp
		ret		
MAKE_PART_TAB	ENDP
		;
		;
		;
IMPORT_NEW_PART	PROC	NEAR
		push	bp
		mov	IMPORTED_FLAG, 0
		mov	cx, 1
		lea	si, part_rec
@@next_mbr_part:
		push	cx
		cmp	[si].b_os_id, 0		; Unused - ignore it
		je	@@mbr_cont_short
		cmp	[si].b_os_id, 0FFh	; Hidden - ignore it
		je	@@mbr_cont_short
		jmp	@@test_part
@@mbr_cont_short:
		jmp	@@mbr_cont
     @@test_part:
		mov	ax, Word Ptr [si].b_rel_sect
		mov	dx, Word Ptr [si].b_rel_sect+2
		push	ax
		or	ax, dx
		pop	ax
		jz	@@mbr_cont		; Empty - ignore it
		mov	cx, MAX_PART_ROWS
		lea	di, part
     @@next_part:
     		cmp	ax, Word Ptr [di].p_rel_sect
     		jne	@@part_cont
     		cmp	dx, Word Ptr [di].p_rel_sect+2
     		jne	@@part_cont
     		; Relative sectors matched
		mov	ax, Word Ptr [si].b_num_sect
		mov	dx, Word Ptr [si].b_num_sect+2

     		cmp	ax, Word Ptr [di].p_num_sect
     		jne	@@import_part
     		cmp	dx, Word Ptr [di].p_num_sect+2
     		jne	@@import_part
     		; Number of sectors also same
     		jmp	@@check_id
     @@part_cont:
     		add	di, SIZE adv_part_rec
     		loop	@@next_part
     		jmp	@@import_part
      @@check_id:
      		mov	ax, [di].p_os_id
      		cmp	ax, OS_ADV
      		je	@@import_part
      		cmp	ah, [si].b_os_id
      		je	@@mbr_cont
      		mov	ah, [si].b_os_id
      		mov	al, 0
      		mov	[di].p_os_id, ax
      		mov	IMPORTED_FLAG, 1
      		jmp	@@mbr_cont
     @@import_part:
     		mov	cx, MAX_PART_ROWS
     		lea	di, part + SIZE part
     @@step_back:
     		sub	di, SIZE adv_part_rec
     		cmp	[di].p_os_id, 0
     		je	@@copy_part
     		loop	@@step_back
     		jmp	@@mbr_cont
     @@copy_part:
     		mov	ah, [si].b_os_id
     		mov	al, 0
     		stosw	; p_os_id
     		mov	al, 0
     		stosb	; p_tag
     		pop	ax
     		push	ax
     		stosb	; p_orig_row
     		xor	ax, ax
     		stosw	; reserved
     		stosw	; reserved
     		push	si
     		add	si, 8
     		movsw	; rel_sect
     		movsw
     		movsw	; num_sect
     		movsw
     		pop	si
      		mov	IMPORTED_FLAG, 10h
@@mbr_cont:
		add	si, SIZE mbr_part_rec
		pop	cx
		inc	cx
		cmp	cx, 5
		je	@@break_out
		jmp	@@next_mbr_part
@@break_out:
		cmp	IMPORTED_FLAG, 0
		je	@@end

		mov	ax, Word Ptr ADV_REL_SECT
		mov	dx, Word Ptr ADV_REL_SECT+2

		mov	bx, ADV_DATA_ADDR
		mov	cx, ADV_DATA_SECT
		
		call	WRITE_N_SECT
		jnc	@@end
		lea	si, MesgErrorSaveAdv
		call	SHOW_ERROR
	@@end:
		pop	bp
		ret
IMPORT_NEW_PART	ENDP
		;
		;
		;
SPRINTF_MENU	PROC	NEAR
		;
		;  Input:  AX - Menu number
		;  Output: TMP - Filled with menu line
		;
		PUSH_REGS
		push	ax
		inc	ax
		lea	si, TMP
		mov	cx, 3
		call	SPRINTF_INT
		lea	di, TMP+3
		mov	al, ' '
		stosb
		stosb
		mov	cx, 30
		pop	bx
		shl	bx, 1
		mov	si, MENU_PTR[bx]
		lea	si, [si].m_name
	@@char:
		lodsb
		or	al, al
		jz	@@break
		stosb
		loop	@@char
	@@break:
		mov	al, ' '
		jcxz	@@no_spaces
	@@space:
		rep
		 stosb
	@@no_spaces:
		stosb
		mov	si, PART_PTR[bx]
		cmp	si, 0
		je	@@no_part
		mov	ax, Word Ptr [si].p_num_sect
		mov	dx, Word Ptr [si].p_num_sect+2
		mov	cl, 11
		shr	ax, cl
		mov	cl, 5
		shl	dx, cl
		or	ax, dx
		mov	cx, 6
		mov	si, di
		call	SPRINTF_INT
		add	di, 6
		mov	al, 'M'
		stosb
		mov	al, ' '
		stosb
		jmp	@@end
	@@no_part:
		mov	cx, 8
		mov	al,' '
		rep
		 stosb
	@@end:
		mov	al, 0
		stosb
		POP_REGS
		ret
SPRINTF_MENU	ENDP
		;
		;
		;
SPRINTF_INT	PROC	NEAR
		;
		;  Input:  AX - Integer to print
		;          CX - Field len
		;       DS:SI - String to fill
		;
		PUSH_REGS
		mov	di, cx
		mov	bx, 10
		xor	cx, cx
@@next:
		xor	dx, dx
		div	bx
		push	dx
		inc	cx
		cmp	ax, 0
		jne	@@next
		mov	bx, di
		cmp	cx, bx
		jae	@@digit
		sub	bx, cx
		mov	al, ' '
@@space:
		mov	[si], al
		inc	si
		dec	bx
		jne	@@space
@@digit:
		pop	ax
		add	ax, '0'
		mov	[si], al
		inc	si
		dec	cx
		jne	@@digit

		xor	ax, ax
		mov	[si], al
		POP_REGS
		ret
SPRINTF_INT	ENDP
		;
		;
		;
SHOW_ERROR	PROC	NEAR
		;
		;  DS:SI - String to print
		;
		PUSH_REGS
		
		push	ds
		mov	di, si
		pop	es
		mov	cx, 76
		mov	al, 0
		repne
		  scasb
		inc	cx
		mov	dx, 80
		sub	dx, cx
		mov	dh, 4
		mov	bx, cx
		shr	bx, 1
		mov	bh, 12
		
		push	si
		lea	si, SAV_BUFFER_ERR
		call	_save_window
		mov	ah, Yellow+BakRed
		lea	si, Border
		call	_border_window
		pop	si
		
		push	bx
		push	dx

		mov	ah, BrWhite+BakRed
		add	bx, 0102h
		xor	cx, cx
		mov	cl, dl
		sub	cl, 4

		call _write_string
		
		mov	ah, Black+BakWhite
		mov	bl, 38
		inc	bh
		lea	si, Mesg_OK
		call	_write_string
@@again:
		mov	ah, 0
		int	16h
		cmp	al, 0Dh	; Enter
		je	@@end
		cmp	al, 1Bh	; ESC
		jne	@@again
@@end:
		pop	dx
		pop	bx
		lea	si, SAV_BUFFER_ERR
		call	_load_window
		
		POP_REGS
		ret
SHOW_ERROR	ENDP
		;
		;
		;

GET_PASSWORD	PROC	NEAR
		;
		;  Input:  none
		;  Output: AX - encrypted password
		;
		mov	bl, 20
		mov	bh, DOT_Y
		mov	dl, 28
		mov	dh, 3
		lea	si, SAV_BUFFER_ERR
		call	_save_window
		mov	ah, Yellow+BakWhite
		lea	si, Border
		call	_border_window

		push	bx
		push	dx

		mov	ah, Black+BakWhite
		add	bl, 2
		add	bh, 1
		lea	si, MesgEnterPassword
		call	_write_string

		add	bl, 16
		mov	dl, 8
		mov	dh, 1
		lea	si, TMP
@@clear_it:
		mov	cx, 0
@@next_key:		
		mov	di, si
		add	di, cx
		mov	al, 0
		stosb
		sub	di, 1

		mov	ah, BrWhite+BakBlack
		call	_clear_window

		push	cx
		push	bx
		jcxz	@@empty
		mov	al, '*'
@@next_char:
		call	_write_char
		add	bx, 1
		loop	@@next_char
@@empty:
		call	_move_cursor
		pop	bx
		pop	cx

		mov	ah, 10h
		int	16h
		cmp	al, 0Dh	; Enter
		je	@@break_out
		cmp	al, 1Bh	; ESC
		je	@@clear_it
		cmp	al, 08h	; BkSp
		je	@@bk_sp

		cmp	cx, 8
		je	@@next_key
		stosb
		inc	cx
		jmp	@@next_key
@@bk_sp:
		cmp	cx, 0
		je	@@next_key
		dec	cx
		
		jmp	@@next_key
@@break_out:
		call	_hide_cursor

		pop	dx
		pop	bx

		lea	si, SAV_BUFFER_ERR
		call	_load_window

		push	ds
		lea	si, TMP
		push	si

		call	_encrypt_password
		add	sp, 4
		
		push	ax
		mov	al, 0
		mov	cx, 9
		lea	di, TMP
		rep
		  stosb
		pop	ax

		ret
GET_PASSWORD	ENDP

		;
		;
		;

SETUP_MENU	PROC	NEAR
		PUSH_REGS
		mov	bl, 29
		mov	bh, 12
		mov	dl, 34
		mov	dh, 3
		lea	si, SAV_BUFFER_ERR
		call	_save_window
		mov	ah, Yellow+BakWhite
		lea	si, Border
		call	_border_window
		mov	ah, Black+BakWhite
		add	bx, 0102h
		lea	si, MesgSetTimeout
		call	_write_string

		mov	dl, adv_timeout
	@@while:
		xor	ax, ax
		mov	al, dl
		mov	cx, 3
		lea	si, TMP
		call	SPRINTF_INT
		mov	ah, BrWhite+BakBlack
		mov	bl, 58
		mov	bh, 13
		call	_write_string
	@@keys:
		mov	ah, 10h
		int	16h
		cmp	ax, 011Bh    ; ESC
		je	@@break
		cmp	ax, 1C0Dh    ; Enter
		je	@@save
		cmp	al, 2Bh      ; '+'
		je	@@inc
		cmp	al, 2Dh      ; '-'
		jne	@@keys
		cmp	dl, 0
		jz	@@keys
		dec	dl
		jmp	@@while
	@@inc:
		cmp	dl, 100
		je	@@keys
		inc	dl
		jmp	@@while
	@@save:
		mov	adv_timeout, dl
	@@break:
		mov	bl, 29
		mov	bh, 12
		mov	dl, 34
		mov	dh, 3
		lea	si, SAV_BUFFER_ERR
		call	_load_window

		POP_REGS
		ret
SETUP_MENU	ENDP

		;
		;
		;

FILL_KEYB_BUFFER	PROC	NEAR
		cmp	ALT_ENTER, 1
		je	@@alt_enter
		cmp	FILL_KB_BUFFER, 0
		je	@@end
		mov	bx, ACT_MENU
		shl	bx, 1
		mov	bx, MENU_PTR[bx]
		mov	cx, [bx].m_num_keys
		cmp	cx, 0
		je	@@end

		push	es
		mov	ax, 0
		mov	es, ax
		lea	si, [bx].m_keys
		mov	di, 41Ah
		mov	ax, 01Eh
		stosw
		add	ax, cx
		add	ax, cx
		stosw
		rep
		  movsw
		pop	es
		jmp	@@end
	@@alt_enter:		; We have to wait until user releases ALT key
	@@not_yet:
		mov	ah, 02
		int	16h		; Read keyboard status
		test	al, 8
		jnz	@@not_yet
	@@end:
		ret
FILL_KEYB_BUFFER	ENDP

		;
		;
		;

DOT_BAR		PROC	NEAR
		;
		;  Print dots and check if key is pressed (key in al)
		;
		;  CF - set if key was pressed
		;
		PUSH_REGS

		mov	bl, KEYS_X
		mov	bh, KEYS_Y
		mov	dl, KEYS_W
		mov	dh, 1
		lea	si, SAV_BUFFER_ERR
		call	_save_window

		mov	bl, DOT_X-1
		mov	bh, DOT_Y
		mov	dl, DOT_NUM+2
		mov	dh, 1
		mov	ah, DOTBAR_COLOR
		call	_clear_window

		mov	ah, KEYS_TXT_COLOR
		mov	bl, KEYS_X
		mov	bh, KEYS_Y
		lea	si, BottomKeysText
		call	_write_string
		mov	ah, KEYS_KEY_COLOR
		lea	si, BottomKeysESC
		call	_write_string

		mov	cx, DOT_NUM
		mov	al, DOT1
		mov	ah, DOTBAR_COLOR
		mov	bl, DOT_X
		mov	bh, DOT_Y
	@@L1:
		call	_write_char
		inc	bl
		loop	@@L1

		mov	cx, DOT_NUM
		mov	bl, DOT_X
		mov	bh, DOT_Y
	@@L2:
		mov	ah, 11h		; Check if key is pressed
		int	16h
		jnz	@@key		; There is no key waiting

		mov	al, DOT2
		mov	ah, DOTBAR_COLOR
		call	_write_char
		inc	bl

		push	bx
		push	cx
		mov	cx, TICKS_PER_DOT
	@@L3:
		push	cx
		mov	ah, 0
		int	1Ah		; Read System Timer
		mov	bx, dx

	@@WT1:	int	1Ah		; Wait one timer tick
		cmp	bx, dx
		je 	@@WT1
		pop	cx
		loop	@@L3

		pop	cx
		pop	bx

		loop	@@L2
		clc
		jmp	@@end
	@@key:
		mov	al, DOT2
		mov	ah, DOTBAR_COLOR
		call	_write_char
		inc	bl
		loop	@@key
		stc
	@@end:
		pushf
		mov	bl, KEYS_X
		mov	bh, KEYS_Y
		mov	dl, KEYS_W
		mov	dh, 1
		lea	si, SAV_BUFFER_ERR
		call	_load_window
		popf

		POP_REGS
		ret
DOT_BAR		ENDP

;----------------------------------------------------------------
CHECK_LAST_CYL	PROC	NEAR
		mov	ax, SECT_PER_TRACK
		mov	dx, DISK_NUM_CYLS
		cmp	dx, 1024
		je	@@end
		mul	dx
		mov	cx, 1
		mov	bx, 7C00h
		call	READ_N_SECT
		jc	@@end
		inc	DISK_NUM_CYLS
	@@end:
		ret
CHECK_LAST_CYL	ENDP

;----------------------------------------------------------------

WRITE_N_SECT	PROC	NEAR
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
@@next_sect:
		push	ax
		push	cx
		push	dx
		call	REL_SECT_TO_CHS
		call	WRITE_SECT
		pop	dx
		pop	cx
		pop	ax
		jc	@@end

		add	ax, 1
		adc	dx, 0

		add	bx, SECT_SIZE

		loop	@@next_sect
@@end:
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret
WRITE_N_SECT	ENDP
		;
		;
		;
WRITE_SECT	PROC	NEAR
		;
		;   ES:BX - Address
		;   CX,DX - CHS
		;
		;   Returns: CF set if error
		;
		push	si
		mov	si, 3		; We will try at most three times
@@try_again:
		mov	ax, 0301h	; Write (AH=03) 1 Sector (AL=01)
		int	13h
		jnc	@@end
					; We get here if there was an error
		mov	ah, 0		; We will try to reset device
		int	13h
	
		dec	si
		jnz	@@try_again
		;
		; We have tried three times, so we will give up
		;
		stc
	@@end:
		pop	si
		ret
WRITE_SECT	ENDP
;----------------------------------------------- CONIO ROUTINES -------

_conio_init	PROC	NEAR
		push	ax
		push	bx

		mov	ah,0Fh		; Check current video mode
		int	10h

		cmp	al,07h
		je	@@Mono
		cmp	al,03h
		je	@@Color
    @@Reset:
		mov	ax,03		; If unknown set Color 80x25
		int	10h
    @@Color:  
		mov	Word Ptr _ScreenArea+2, 0B800h
		jmp	@@skip1
    @@Mono:
		mov	Word Ptr _ScreenArea+2, 0B000h
    @@skip1:
		push	ds
		mov	ax, 0
		mov	ds, ax
		mov	bx, 484h
		mov	al, [bx]
		pop	ds
		inc	al
		mov	_ScreenHeight, al
		mov	ah,0Fh
		int	10h
		mov	_ScreenWidth, ah
		mov	al, _ScreenHeight
		mov	ah, _ScreenWidth
		mul	ah
		mov	_ScreenLength, ax
		mov	Word Ptr _ScreenArea, 0h

		pop	bx
		pop	ax		
		ret
_conio_init	ENDP


_conio_exit	PROC	NEAR
		ret
_conio_exit	ENDP


;----------------------------------------------------------------
		
		;
		;  Cursor position:  BL=X  BH=Y
		;
		
_move_cursor	PROC	NEAR
		PUSH_REGS
		mov	dx, bx
		sub	dx, 0101h
		mov	ah, 02h
		mov	bh, 0
		int	10h
		POP_REGS
		ret
_move_cursor	ENDP


_hide_cursor	PROC	NEAR
		push	bx
		mov	bl, 1
		mov	bh, 26
		call	_move_cursor
		pop	bx
		ret
_hide_cursor	ENDP


_save_cursor	PROC	NEAR
		PUSH_REGS
		mov	ah, 3
		mov	bh, 0
		int	10h
		mov	CURSOR_SAVE_XY, dx
		POP_REGS
		ret
_save_cursor	ENDP


_restore_cursor	PROC	NEAR
		PUSH_REGS
		mov	ah, 2
		mov	bh, 0
		mov	dx, CURSOR_SAVE_XY
		int	10h
		POP_REGS
		ret
_restore_cursor	ENDP

;----------------------------------------------------------------
WINDOW_XY	MACRO
		push	ax
		mov	al, _ScreenWidth
		sub	bx, 0101h
		mul	bh
		xor	bh, bh
		add	ax, bx
		shl	ax, 1
		add	di, ax
		pop	ax
ENDM

WINDOW_WH	MACRO
		xor	cx, cx
		xor	bx, bx
		mov	cl, dl
		mov	bl, _ScreenWidth
		sub	bx, cx
		shl	bx, 1
		xor	cx, cx
ENDM
;----------------------------------------------------------------


_write_char	PROC	NEAR
		;
		; Input: AL=Char  BL=X
		;	 AH=Attr  BH=Y
		;
		PUSH_REGS

		les	di, _ScreenArea

		WINDOW_XY

		stosw

		POP_REGS
		ret
_write_char	ENDP



_write_string	PROC	NEAR
		;
		; Input: DS:SI=Str   BL=X
		;	    AH=Attr  BH=Y
		;
		PUSH_REGS

		les	di, _ScreenArea

		WINDOW_XY

		jmp	@@first
    @@next:
		stosw
    @@first:
		lodsb
		cmp	al, 0
		jne	@@next

		POP_REGS
		ret
_write_string	ENDP


;----------------------------------------------------------------


_save_window	PROC	NEAR
		;
		; Input: DS:SI=Buf  BL=X  DL=W
		;	            BH=Y  DH=H
		;
		PUSH_REGS
		
		les	di, _ScreenArea
		
		WINDOW_XY
		WINDOW_WH
		
		push	es
		push	ds
		xchg	si, di
		pop	es
		pop	ds

    @@next_row:
		mov	cl, dl
		rep
		 movsw
		add	si, bx
		dec	dh
		jne	@@next_row

		POP_REGS
		ret
_save_window	ENDP



_load_window	PROC	NEAR
		;
		; Input: DS:SI=Buf  BL=X  DL=W
		;	            BH=Y  DH=H
		;
		PUSH_REGS

		les	di, _ScreenArea
		
		WINDOW_XY
		WINDOW_WH

    @@next_row:
		mov	cl, dl
		rep
		 movsw
		add	di, bx
		dec	dh
		jne	@@next_row

		POP_REGS
		ret
_load_window ENDP


;----------------------------------------------------------------


_clear_window	PROC	NEAR
		;
		; Input: AH=Attr  BL=X  DL=W
		;	          BH=Y  DH=H
		;
		PUSH_REGS

		les	di, _ScreenArea
		
		WINDOW_XY
		WINDOW_WH

		mov	al,' '
    @@next_row:
		mov	cl, dl
		rep
		 stosw
		add	di, bx
		dec	dh
		jne	@@next_row

		POP_REGS
		ret
_clear_window	ENDP



_scroll_window	PROC	NEAR
		;
		; Input: AL=Len   BL=X  DL=W
		;	 AH=Attr  BH=Y  DH=H
		;
		PUSH_REGS

		les	di, _ScreenArea
		
		WINDOW_XY
		WINDOW_WH
		
		push	ax
		imul	Byte Ptr _ScreenWidth
		shl	ax, 1
		mov	si, di
		add	si, ax
		pop	ax
		cmp	si, di
		ja	@@scroll

		push	ax
		mov	al, dh
		dec	al
		mul	Byte Ptr _ScreenWidth
		add	di, ax
		add	si, ax
		mov	bl, dl
		shl	bx, 1
		neg	bx
		pop	ax
		neg	al

    @@scroll:
    		push	es
		pop	ds
    @@next_row:
		mov	cl, dl
		rep
		 movsw
		add	si, bx
		add	di, bx
		dec	dh
		jne	@@next_row

		mov	dh, al
		mov	al,' '
    @@clr_row:
		mov	cl, dl
		rep
		 stosw
		add	si, bx
		add	di, bx
		dec	dh
		jne	@@clr_row

		POP_REGS
		ret
_scroll_window	ENDP



_border_window	PROC	NEAR
		;
		; Input: DS:SI=Brdr  BL=X  DL=W
		;	    AH=Attr  BH=Y  DH=H
		;
		PUSH_REGS

		les	di, _ScreenArea
		
		WINDOW_XY
		WINDOW_WH

		sub	dx, 0202h

		lodsb			; Upper row
		stosw
		mov	cl, dl
		lodsb
		rep
		  stosw
		lodsb
		stosw
		add	di, bx
		cmp	dh, 00
		je	@@NoMiddleRows
	
    @@next_row:
		lodsb			; All rows in the middle
		stosw
		mov	cl, dl
		lodsb
		cmp	al, 00
		je	@@NoFill
		rep
		  stosw
		jmp	@@FillDone
    @@NoFill:
		add	di, cx
		add	di, cx
    @@FillDone:
		lodsb
		stosw
		add	di, bx
		sub	si, 03
		dec	dh
		jne	@@next_row

 @@NoMiddleRows:
		add	si, 03		; Bottom row
		lodsb
		stosw
		mov	cl, dl
		lodsb
		rep
		  stosw
		lodsb
		stosw

		POP_REGS
		ret
_border_window	ENDP

;----------------------------------------------------------------------
_encrypt_password	PROC	FAR
	push	bp
	mov	bp,sp
	push	ds
	push	si
	push	di

	lds	si, dword ptr [bp+6]
	mov	bx, 12345
	mov	di, 0

	jmp	@@check_cond
@@next_char:

	mov	ah, 0
	
	mov	cx, ax
	xor	cx, bx

	shl	ax, 2
	add	ax, 7
	
	shr	bx, 1
	add	bx, 3
	
	mul	bx
	add	ax, cx

	mov	bx, ax
	mov	di, ax

@@check_cond:
	lodsb
	or	al, al
	jne	@@next_char
	
	mov	ax, di

	pop	di
	pop	si
	pop	ds
	pop	bp
	retf
_encrypt_password	ENDP
;----------------------------------------------------------------------
GAP2		PROC
GAPLEN2		EQU	(ADV_CODE_SIZE-(GAP2-_ADV_MANAGER))
IF GAPLEN2
		DB	GAPLEN2 DUP(0)	
ENDIF
GAP2		ENDP
;----------------------------------------------------------------------

ADV_MAN_TEXT	ENDS

		END
