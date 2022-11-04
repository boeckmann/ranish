cpu 8086

section SCREEN_DATA class=DATA align=2
group DGROUP SCREEN_DATA

global   _ScreenWidth
global   _ScreenHeight
global   _MouseInstalled

_ScreenOffset  dw   0   ; 0000h
_ScreenSegment dw   0   ; B800h
_ScreenWidth   db   0   ;  80
_ScreenHeight  db   0   ;  25
_ScreenLength  dw   0   ; 80*25
_MouseInstalled   dw   0   ; 0 or 1
_LastKeyShifts dw   0   ;
_LastMousePosX dw   0  ;
_LastMousePosY dw   0  ;
_LastMouseBtns dw   0   ;
OriginalTimer1 dw   0   ;
OriginalTimer2 dw   0   ;

section CONIO_TEXT class=CODE align=16

global   _screen_init
global   _screen_exit
global   _show_mouse
global   _hide_mouse
global   _move_mouse
global   _cursor_size
global   _move_cursor
global   _get_event
global   _write_char
global   _write_string
global   _load_window
global   _save_window
global   _clear_window
global   _scroll_window
global   _border_window


%macro Show_Mouse 0
      cmp   word [_MouseInstalled], 1
      jne   .nomouse1
      mov   ax, 0001h
      int   33h
    .nomouse1:
%endmacro


%macro Hide_Mouse 0
      cmp   word [_MouseInstalled], 1
      jne   .nomouse2
      mov   ax, 0002h
      int   33h
    .nomouse2:
%endmacro


;----------------------------------------------------------------


_screen_init:
      push  bp
      mov   bp, sp
      push  es
      push  si
      push  di


      mov   ah,0Fh      ; Check current video mode
      int   10h

      cmp   al,07h
      je .Mono
      cmp   al,03h
      je .Color
    .Reset:
      mov   ax,03    ; If unknown set Color 80x25
      int   10h
    .Color:  
      mov   word [_ScreenSegment], 0B800h
      jmp   .skip1
    .Mono:
      mov   word [_ScreenSegment], 0B000h
    .skip1:
      push  ds
      mov   ax, 0
      mov   ds, ax
      mov   bx, 484h
      mov   al, [bx]
      pop   ds
      inc   al
      mov   [_ScreenHeight], al
      mov   ah,0Fh
      int   10h
      mov   [_ScreenWidth], ah
      mov   al, [_ScreenHeight]
      mul   ah
      mov   [_ScreenLength], ax
      mov   word [_ScreenOffset], 0h
      
      mov   ax,0000h ; Check for a mouse
      int   33h
      cmp   ax, 0h
      je .No_Mouse
    ._Mouse:
      mov   word [_MouseInstalled], 1
      mov   ax, 0003h   ; read mouse position
      int   33h
      mov   [_LastMousePosX], cx
      mov   [_LastMousePosY], dx
      mov   [_LastMouseBtns], bx
      jmp   .skip2
    .No_Mouse:
      mov   word [_MouseInstalled], 0
    .skip2: 
      mov   ah, 12h     ; Get keyboard status
      int   16h
      mov   [_LastKeyShifts], ax
      
    .ret:   pop   di
      pop   si
      pop   es
      pop   bp
      retf


_screen_exit:
      retf


;----------------------------------------------------------------

_show_mouse:
      Show_Mouse
      retf


_hide_mouse:
      Hide_Mouse
      retf


_move_mouse:

  .Y     equ   08h
  .X     equ   06h

      push  bp
      mov   bp, sp
      mov   ax, 0004h
      mov   cx, [bp+.X]
      mov   dx, [bp+.Y] 

      dec   cx
      shl   cx, 1
      shl   cx, 1
      shl   cx, 1

      dec   dx
      shl   dx, 1
      shl   dx, 1
      shl   dx, 1

      int   33h
      pop   bp
      retf


;----------------------------------------------------------------


_move_cursor:

  .Y     equ   08h
  .X     equ   06h

      push  bp
      mov   bp, sp
      mov   ah, 02
      mov   bh, 00
      mov   dl, [bp+.X]
      mov   dh, [bp+.Y]
      sub   dx, 0101h
      int   10h
      pop   bp
      retf


_cursor_size:

  .Bottom   equ   08h
  .Top      equ   06h

      push  bp
      mov   bp, sp
      mov   ah, 01
      mov   cl, [bp+.Top]
      mov   ch, [bp+.Bottom]
      and   cx, 1F1Fh
      int   10h
      pop   bp
      retf



;----------------------------------------------------------------


_write_char:

   .C equ   0Ch
   .Y equ   0Ah
   .X equ   08h
    .Attribute equ   06h

      Hide_Mouse

      push  bp
      mov   bp, sp
      push  ds
      push  di
      push  si
      sub   ax, ax
      mov   bx, ax
      mov   cx, bx
      mov   dx, cx
   
      mov   dl, [_ScreenWidth]
      mov   al, [bp+.Y]
      mov   bl, [bp+.X]  
      dec   al
      dec   bl  
      mul   dl
      add   ax, bx
      shl   ax, 1
      les   di, [_ScreenOffset]
      add   di, ax

      mov   al, [bp+.C]
      mov   ah, [bp+.Attribute]
      stosw

      pop   si
      pop   di
      pop   ds
      pop   bp

      Show_Mouse
      retf



_write_string:

   .S equ   0Ch
   .Y equ   0Ah
   .X equ   08h
    .Attribute equ   06h

      Hide_Mouse

      push  bp
      mov   bp, sp
      push  ds
      push  di
      push  si
      sub   ax, ax
      mov   bx, ax
      mov   cx, bx
      mov   dx, cx
   
      mov   dl, [_ScreenWidth]
      mov   al, [bp+.Y]
      mov   bl, [bp+.X]  
      dec   al
      dec   bl  
      mul   dl
      add   ax, bx
      shl   ax, 1
      les   di, [_ScreenOffset]
      lds   si, [bp+.S]
      add   di, ax

      mov   ah, [bp+.Attribute]
      jmp   .first
    .next:
      stosw
    .first:
      lodsb
      cmp   al, 0
      jne   .next
      
      pop   si
      pop   di
      pop   ds
      pop   bp

      Show_Mouse
      retf


;----------------------------------------------------------------


_save_window:

   .Destination   equ   0Eh
   .H equ   0Ch
   .W equ   0Ah
   .Y equ   08h
   .X equ   06h

      Hide_Mouse

      push  bp
      mov   bp, sp
      push  ds
      push  di
      push  si
      sub   ax, ax
      mov   bx, ax
      mov   cx, bx
      mov   dx, cx
   
      mov   dl, [_ScreenWidth]
      mov   al, [bp+.Y]
      mov   bl, [bp+.X]  
      dec   al
      dec   bl  
      mul   dl
      add   ax, bx
      shl   ax, 1
      les   di, [bp+.Destination]
      lds   si, [_ScreenOffset]
      add   si, ax
      mov   bl, [bp+.W]
      mov   bh, [bp+.H]
      sub   dl, bl
      shl   dx, 1
    .next_row:
      mov   cl, bl
      rep   movsw
      add   si, dx
      dec   bh
      jne   .next_row

      pop   si
      pop   di
      pop   ds
      pop   bp

      Show_Mouse
      retf


_load_window:

     .Source   equ   0Eh
   .H equ   0Ch
   .W equ   0Ah
   .Y equ   08h
   .X equ   06h

      Hide_Mouse

      push  bp
      mov   bp, sp
      push  ds
      push  di
      push  si
      sub   ax, ax
      mov   bx, ax
      mov   cx, bx
      mov   dx, cx
   
      mov   dl, [_ScreenWidth]
      mov   al, [bp+.Y]
      mov   bl, [bp+.X]  
      dec   al
      dec   bl  
      mul   dl
      add   ax, bx
      shl   ax, 1
      les   di, [_ScreenOffset]
      lds   si, [bp+.Source]
      add   di, ax

      mov   bl, [bp+.W]
      mov   bh, [bp+.H]
      sub   dl, bl
      shl   dx, 1
      
    .next_row:
      mov   cl, bl
      rep   movsw
      add   di, dx
      dec   bh
      jne   .next_row
            
      pop   si
      pop   di
      pop   ds
      pop   bp

      Show_Mouse
      retf


;----------------------------------------------------------------


_clear_window:

   .H equ   0Eh
   .W equ   0Ch
   .Y equ   0Ah
   .X equ   08h
    .Attribute equ   06h

      Hide_Mouse

      push  bp
      mov   bp, sp
      push  ds
      push  di
      push  si
      sub   ax, ax
      mov   bx, ax
      mov   cx, bx
      mov   dx, cx
   
      mov   dl, [_ScreenWidth]
      mov   al, [bp+.Y]
      mov   bl, [bp+.X] 
      dec   al
      dec   bl  
      mul   dl
      add   ax, bx
      shl   ax, 1
      les   di, [_ScreenOffset]
      add   di, ax

      mov   bl, [bp+.W]
      mov   bh, [bp+.H]
      sub   dl, bl
      shl   dx, 1
      
      mov   ah, [bp+.Attribute]
      mov   al, ' '

    .next_row:
      mov   cl, bl
      rep
      stosw
      add   di, dx
      dec   bh
      jne   .next_row
            
      pop   si
      pop   di
      pop   ds
      pop   bp

      Show_Mouse
      retf



_scroll_window:

       .Len equ   10h
   .H equ   0Eh
   .W equ   0Ch
   .Y equ   0Ah
   .X equ   08h
    .Attribute equ   06h

      Hide_Mouse

      push  bp
      mov   bp, sp
      push  ds
      push  di
      push  si
      sub   ax, ax
      mov   bx, ax
      mov   cx, bx
      mov   dx, cx
   
      mov   dl, [_ScreenWidth]
      les   di, [_ScreenOffset]
      lds   si, [_ScreenOffset]
      mov   cl, dl
      sub   dl, [bp+.W]
      shl   dx, 1
   
      mov   al, [bp+.Len]
      imul  cl
      shl   ax, 1
      add   si, ax
      or ax, ax
   
      mov   al, [bp+.Y]
      mov   ah, [bp+.X]
      mov   bl, [bp+.H]
      mov   bh, [bp+.W]
      jns   .n12

      std
      neg   Byte [bp+.Len]
      neg   dx
      add   ax, bx
      sub   ax, 0101h
    .n12:
      sub   ax, 0101h
      xchg  ah, cl
      mul   ah
      add   ax, cx
      shl   ax, 1
      add   si, ax
      add   di, ax
      sub   bl, [bp+.Len]
   
    .next_row:
      mov   cl, bh
      rep   movsw
      add   di, dx
      add   si, dx
      dec   bl
      jne   .next_row

      mov   bl, [bp+.Len]
      mov   ah, [bp+.Attribute]
      mov   al, ' '

    .clr_row:
      mov   cl, bh
      rep   stosw
      add   di, dx
      dec   bl
      jne   .clr_row

      cld

      pop   si
      pop   di
      pop   ds
      pop   bp

      Show_Mouse
      retf



_border_window:

      .Border  equ   10h
    .H   equ   0Eh
    .W   equ   0Ch
    .Y   equ   0Ah
    .X   equ   08h
     .Attribute   equ   06h

      Hide_Mouse

      push  bp
      mov   bp, sp
      push  ds
      push  di
      push  si
      sub   ax, ax
      mov   bx, ax
      mov   cx, bx
      mov   dx, cx
   
      mov   dl, [_ScreenWidth]
      mov   al, [bp+.Y]
      mov   bl, [bp+.X]  
      dec   al
      dec   bl  
      mul   dl
      add   ax, bx
      shl   ax, 1
      les   di, [_ScreenOffset]
      lds   si, [bp+.Border]
      add   di, ax

      mov   bl, [bp+.W]
      mov   bh, [bp+.H]
      sub   dl, bl
      shl   dx, 1
      sub   bx, 0202h
      
      mov   ah, [bp+.Attribute]

      lodsb       ; Upper row
      stosw
      mov   cl, bl
      lodsb
      rep   stosw
      lodsb
      stosw
      add   di, dx
      cmp   bh, 00
      je .NoMiddleRows
   
    .next_row:
      lodsb       ; All rows in the middle
      stosw
      mov   cl, bl
      lodsb
      cmp   al, 00
      je .NoFill
      rep   stosw
      jmp   .FillDone
    .NoFill:
      add   di, cx
      add   di, cx
    .FillDone:
      lodsb
      stosw
      add   di, dx
      sub   si, 03
      dec   bh
      jne   .next_row

 .NoMiddleRows:
      add   si, 03      ; Bottom row
      lodsb
      stosw
      mov   cl, bl
      lodsb
      rep   stosw
      lodsb
      stosw

      pop   si
      pop   di
      pop   ds
      pop   bp

      Show_Mouse
      retf


;----------------------------------------------------------------


_get_event:

  .flags equ   0Ah
  .event equ   06h

  .ev_type  equ   0

  .key      equ   2
  .scan     equ   4
  .shift equ   6
  .shiftX   equ   8

  .x     equ   10
  .y     equ   12
  .left     equ   14
  .right equ   16
  .middle   equ   18

  .timer1   equ   20
  .timer2   equ   22

  EV_KEY equ    1
  EV_SHIFT  equ    2
  EV_MOUSE  equ    4
  EV_TIMER  equ    8
  EV_NONBLOCK  equ   16

      push  bp
      mov   bp, sp
      push  ds
      push  es
      push  si
      push  di
      les   si, [bp+.event]
      sub   ax, ax
      mov   [es:si+.ev_type], ax
      mov   [es:si+.key], ax
      mov   [es:si+.scan], ax

      test  word [bp+.flags], EV_TIMER
      jz .main_loop
      mov   ah, 0h
      int   1Ah
      mov   [OriginalTimer1], dx
      mov   [OriginalTimer2], cx

    .main_loop:
      test   word [bp+.flags], EV_KEY
      jz .test_shifts

      mov   ah, 11h        ; Check for a key
      int   16h
      jz .test_shifts

      or word [es:si+.ev_type], EV_KEY    ; Key was pressed
      mov   ah, 10h
      int   16h
      mov   [es:si+.scan], ax
      cmp   al, 0E0h    ; Is it an extended key
      jne   .normal_key
      cmp   ah, 0
      je .normal_key
      mov   al, 0
    .normal_key:
      mov   ah, 0
      mov   [es:si+.key], ax
      jmp   .break_out

    .test_shifts:
      test  word [bp+.flags], EV_SHIFT
      jz .test_mouse

      mov   ah, 12h        ; Check if shifts changed
      int   16h
      cmp   ax, [_LastKeyShifts]
      jz .test_mouse

      or word [es:si+.ev_type], EV_SHIFT
      mov   bx, [_LastKeyShifts]
      xor   bx, ax
      mov   [es:si+.shift], ax
      mov   [es:si+.shiftX], bx
      mov   [_LastKeyShifts], ax
      jmp   .break_out2

    .test_mouse:
      test  word [bp+.flags], EV_MOUSE
      jz .test_time
      cmp   word [_MouseInstalled], 1
      jne   .test_time

      mov   ax, 0003h      ; Check mouse status
      int   33h
      cmp   bx, [_LastMouseBtns]
      jne   .mouse
      cmp   cx, [_LastMousePosX]
      jne   .mouse
      cmp   dx, [_LastMousePosY]
      je .test_time
    .mouse:
      or word [es:si+.ev_type], EV_MOUSE

      mov   [_LastMouseBtns], bx
      mov   [_LastMousePosX], cx
      mov   [_LastMousePosY], dx
      jmp   .break_out
    .test_time:
      test  word [bp+.flags], EV_TIMER
      jz .loop_tail

      mov   ah, 0h         ; Check timer
      int   1Ah
      sub   dx, [OriginalTimer1]
      sbb   cx, [OriginalTimer2]
      cmp   cx, [es:si+.timer2]
      jb .loop_tail
      cmp   dx, [es:si+.timer1]
      jb .loop_tail
      or word [es:si+.ev_type], EV_TIMER
      jmp   .break_out
    .loop_tail:
      test  word [bp+.flags], EV_NONBLOCK
      jnz   .break_out
      jmp   .main_loop

    .break_out:
      mov   ah, 12h     ; Updayte shift status
      int   16h
      mov   [es:si+.shift], ax
      mov   bx, [_LastKeyShifts]
      xor   bx, ax
      mov   [es:si+.shiftX], bx

    .break_out2:
         cmp   word [_MouseInstalled], 1
         jne   .end

      mov   ax, 0003h   ; Update mouse status
      int   33h

      shr   cx, 1
      shr   cx, 1
      shr   cx, 1
      inc   cx
      mov   [es:si+.x], cx
      shr   dx, 1
      shr   dx, 1
      shr   dx, 1
      inc   dx
      mov   [es:si+.y], dx

      test  bx, 0001h
      jne   .jleft
      mov   word [es:si+.left], 0h
      jmp   .leftE
   .jleft:
      mov   word [es:si+.left], 1h
   .leftE:
      test  bx, 0002h
      jne   .jright
      mov   word [es:si+.right], 0h
      jmp   .rightE
   .jright:
      mov   word [es:si+.right], 1h
   .rightE:
      test  bx, 0004h
      jne   .jmiddle
      mov   word [es:si+.middle], 0h
      jmp   .middleE
   .jmiddle:
      mov   word [es:si+.middle], 1h
   .middleE:

   .end:
      pop   di
      pop   si
      pop   es
      pop   ds
      pop   bp
      retf

