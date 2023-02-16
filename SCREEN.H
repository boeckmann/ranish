#ifndef _SCREEN_H
#define _SCREEN_H

#define Border11f  "⁄ƒø≥ ≥¿ƒŸ" /* The middle character will be  */
#define Border12f  "’Õ∏≥ ≥‘Õæ" /* used to fill the window space */
#define Border22f  "…Õª∫ ∫»Õº"
#define Border21if "«ƒ∂∫ ∫»Õº"

#define Border11e "⁄ƒø≥\0≥¿ƒŸ" /* if it is '\0' window will not */
#define Border12e "’Õ∏≥\0≥‘Õæ" /* be filled and left as it is.  */
#define Border22e "…Õª∫\0∫»Õº"

#define Black   0x00 /* Foreground colors */
#define Blue    0x01
#define Green   0x02
#define Cyan    0x03
#define Red     0x04
#define Magenta 0x05
#define Brown   0x06
#define White   0x07

#define Gray      0x08
#define BrBlue    0x09
#define BrGreen   0x0A
#define BrCyan    0x0B
#define BrRed     0x0C
#define BrMagenta 0x0D
#define Yellow    0x0E
#define BrWhite   0x0F

#define BakBlack   0x00 /* Background colors */
#define BakBlue    0x10
#define BakGreen   0x20
#define BakCyan    0x30
#define BakRed     0x40
#define BakMagenta 0x50
#define BakBrown   0x60
#define BakWhite   0x70

#define Blink 0x80

#define EV_KEY   1
#define EV_SHIFT 2
#define EV_MOUSE 4
#define EV_TIMER 8

#define EV_NONBLOCK 16

#define SCREEN_TICKS_PER_SEC    18.2
#define SCREEN_TIMER( seconds ) ( (seconds)*18.2 )

struct event {
   unsigned int ev_type; /* Type of returned event */

   unsigned int key;    /* Ascii Code of a key */
   unsigned int scan;   /* Scan & Ascii Codes of a key  */
   unsigned int shift;  /* Status of Shift keys */
   unsigned int shiftX; /* Only those flags are set which was changed */

   unsigned int x, y; /* Mouse position */
   unsigned int left;
   unsigned int right; /* 1 - if button pressed, 0 - if not */
   unsigned int middle;

   long timer; /* number of ticks to wait, if EV_TIMER was set */
};

extern unsigned char const ScreenWidth;
extern unsigned char const ScreenHeight;
extern unsigned int const MouseInstalled;

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl screen_init( void );
void __cdecl screen_exit( void );

void __cdecl show_mouse( void );
void __cdecl hide_mouse( void );
void __cdecl move_mouse( int x, int y );

void __cdecl move_cursor( int x, int y );
void __cdecl cursor_size( int top, int bottom );

void __cdecl get_event( struct event *ev, int flags );

void __cdecl write_char( int attr, int x, int y, int ch );
void __cdecl write_string( int attr, int x, int y, char *str );

void __cdecl save_window( int x, int y, int w, int h, char *buf );
void __cdecl load_window( int x, int y, int w, int h, char *buf );

void __cdecl clear_window( int attr, int x, int y, int w, int h );
void __cdecl scroll_window( int attr, int x, int y, int w, int h, int len );
void __cdecl border_window( int attr, int x, int y, int w, int h,
                            char *border );

#ifdef __cplusplus
}
#endif

#define SK_R_SHIFT       0x01
#define SK_L_SHIFT       0x02
#define SK_SHIFT         0x03
#define SK_CTRL          0x04
#define SK_ALT           0x08
#define SK_SCROLL_LOCKED 0x10
#define SK_NUM_LOCKED    0x20
#define SK_CAPS_LOCKED   0x40
#define SK_INSERT        0x80

#define SK_L_CTRL      0x0100
#define SK_L_ALT       0x0200
#define SK_R_CTRL      0x0400
#define SK_R_ALT       0x0800
#define SK_SCROLL_LOCK 0x1000
#define SK_NUM_LOCK    0x2000
#define SK_CAPS_LOCK   0x4000
#define SK_SYS_REQ     0x8000

#endif
