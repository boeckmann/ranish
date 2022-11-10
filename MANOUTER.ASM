
; Source code in the Public Domain

; Assemble to OMF object with:	nasm manouter.asm -t -f obj
; Or assemble to binary with:	nasm manouter.asm -t
;
; Assembly options:
; -DUSEDEBUG		Include debugging entrypoint (padded to length 600h)

%ifidni __OUTPUT_FORMAT__, obj
global _ADV_IPL, _ADV_MANAGER, _encrypt_password
%endif

SECT_SIZE	EQU	  512

ADV_CODE_SIZE	EQU	 8192
ADV_DATA_SIZE	EQU	 2048

ADV_CODE_SECT	EQU	(ADV_CODE_SIZE/SECT_SIZE)
ADV_DATA_SECT	EQU	(ADV_DATA_SIZE/SECT_SIZE)

ADV_CODE_ADDR	EQU	(800h)
ADV_DATA_ADDR	EQU	(800h+ADV_CODE_SIZE)

; ADV_MAN_TEXT	SEGMENT	PARA PRIVATE	'CODE'
%ifidni __OUTPUT_FORMAT__, obj
 %define PRIVATE private
%else
 %define PRIVATE			; empty string
%endif
	section ADV_MAN_TEXT PRIVATE align=16
	section ADV_MAN_TEXT
%ifdef USEDEBUG
_ADV_IPL: equ $ + 600h
%else
_ADV_IPL:
%endif
_ADV_MANAGER: equ _ADV_IPL + 512
	incbin "manage.bin"

;----------------------------------------------------------------------
_encrypt_password:
	push	bp
	mov	bp,sp
	push	ds
	push	si
	push	di

	lds	si, [bp+6]
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

;----------------------------------------------------------------------
GAP2:
GAPLEN2		EQU	(ADV_CODE_SIZE-(GAP2-_ADV_MANAGER))
	times GAPLEN2 db 0

;----------------------------------------------------------------------
