#include <stdio.h>
#include <malloc.h>

#include "conio.h"

void key_codes(void);

void main(void)
{
 struct event ev;

 conio_init();
 cursor_size(16,14);
 show_mouse();

 write_string(  Green, 10, ScreenHeight, "Use arrows to move window.  Press ESC twice to exit." );

 key_codes();
 
 hide_mouse();
 cursor_size(14,16);
}/* main */



void key_codes(void)
{
 char *WindowBuff, *event, temp1[40], temp2[40];
 int last_key=0;
 int flag=1;
 static int x=20, y=6, w=25, h=9,  px, py;
 int Attr1=Yellow+BakRed;
 int Attr2=BrCyan+BakRed;
 struct event ev;
 
 if( (WindowBuff=(char*)malloc(w*h*2))==0 ) return;
 
 get_event(&ev,EV_NONBLOCK);

 while(1)
    {
     if( flag==1 )
       {
        px=x;
        py=y;
        save_window( x,y,w,h, WindowBuff );
        clear_window(  Attr1, x,y,w,h );
        border_window(  Attr1, x,y,w,h, Border22f );
        flag=0;
       }

     event="NONE";
     if( ev.ev_type & EV_TIMER ) event="(EV_TIMER)";
     if( ev.ev_type & EV_MOUSE ) event="(EV_MOUSE)";
     if( ev.ev_type & EV_SHIFT ) event="(EV_SHIFT)";
     if( ev.ev_type & EV_KEY ) 
       {
        event="(EV_KEY)  ";

        if( ev.key!=0 )  sprintf(temp1,"Ascii code: 0x%02X ('%c')",ev.key,ev.key);
                 else    sprintf(temp1,"Ascii code: 0x%02X      ",ev.key);
                         sprintf(temp2,"Scan  code: 0x%04X    ",ev.scan);
       }
     else
       {
        sprintf(temp1,"Ascii code: undefined ");
        sprintf(temp2,"Scan  code: undefined ");
       }

     write_string(Attr2,x+2,y+2,temp1);
     write_string(Attr2,x+2,y+3,temp2);

     sprintf(temp1,"Event: %2d %10s", ev.ev_type,event);
     write_string(Attr2,x+2,y+1,temp1);
     
     sprintf(temp1,"Shifts status:   %04X",ev.shift); write_string(Attr2,x+2,y+4,temp1);
     sprintf(temp1,"Shifts changed:  %04X",ev.shiftX); write_string(Attr2,x+2,y+5,temp1);

  if( MouseInstalled )
    {
     sprintf(temp1,"Mouse X: %2d",ev.x); write_string(Attr2,x+2,y+6,temp1);
     sprintf(temp1,"Mouse Y: %2d",ev.y); write_string(Attr2,x+2,y+7,temp1);
     sprintf(temp1,"Left : %d",ev.left); write_string(Attr2,x+15,y+6,temp1);
     sprintf(temp1,"Right: %d",ev.right); write_string(Attr2,x+15,y+7,temp1);
    }
  else
    {
     write_string(Attr2,x+2,y+7,"Mouse is not detected");
    }

     ev.timer=0x100;
     
     get_event( &ev, EV_KEY | EV_SHIFT | EV_MOUSE | EV_TIMER );

     if( ev.ev_type & EV_KEY )
       {
        if( ev.key==0x1B && last_key==0x1B ) break;
 
        last_key=ev.key;

        switch(ev.scan&0xFF00)
            {
             case 0x4800: if( y>1 ) y--;           /* UP */
                                  break;
             case 0x4B00: if( x>1 ) x--;           /* LEFT */
                                  break;
             case 0x4D00: if( x+w-1 < ScreenWidth ) x++;    /* RIGHT */
                                  break;
             case 0x5000: if( y+h-1 < ScreenHeight ) y++;    /* DOWN */
                                   break;
             default    : break;
            }
       }/* EV_KEY */
       
     if( x!=px || y!=py )
       {
        load_window( px,py,w,h, WindowBuff );
        flag=1;
       }
    }/* while(1) */

 load_window( x,y,w,h, WindowBuff );
 free(WindowBuff);
}/* key_codes */
