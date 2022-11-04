#include "part.h"

#define X 1
#define Y 1
#define W 80
#define H 25
#define N 21
#define TX (X+10)
#define TY (Y+H-2)
#define TW (W-12)
#define LEN (W-6)
#define BARX (X+W-3)
#define BARY (Y+2)
#define BARLEN (N-2)

#define MAX_BACKS 8

#undef BORDER_COLOR

#define TEXT_COLOR   (Black+BakWhite)
#define LINK_COLOR   (BrBlue+BakWhite)
#define TYPE_COLOR   (White+BakBlack)
#define FOUND_COLOR  (BrRed+BakWhite)
#define BORDER_COLOR (Yellow+BakWhite)

void html_view(char *target_name)
{
 long i, j, k, ch, line, len, maxtop, nchars, first_time, enable_timer;
 char *p, *q, *s, *top, *old_top, *bottom, *seek_base, *bs, *bq;
 char *tmp, *col, *str;  /* LEN+1 */
 struct event ev;
 long force_redraw, link_started, seek_cnt;
 long check_mouse, mline, mpos, left_was_pressed;
 char *link_reference, *link_under_mouse;
 char *link_back[MAX_BACKS];
 long num_backs;
 long barpos, old_barpos, forced_barpos, bar_hooked;
 char *mesg, *buf;
 char *text, *text_buf=0;
 static char err_msg[80];

 long l;
 FILE *f;

 p=base_dir+strlen(base_dir);

 if( target_name==0 || *target_name==0 || *target_name=='#' )
   strcat(p,HELP_FILE_NAME);
 else
   {
    for( q=target_name, s=p ; *q!=0 && *q!='#' ; q++, s++ )
      *s=*q;
    *s=0;
    target_name = (*q!=0) ? q : 0;
   }

 sprintf(err_msg,HTML_ERROR_READ,base_dir);
 f=fopen(base_dir,"r");
 *p=0;

 if( f==0 )
   {
    show_error(err_msg);
    return;
   }
    
 l=filelength(fileno(f));
 l=min(l,65530L);
 if( (text_buf=malloc(l+1))==0 )
   {
    show_error(ERROR_MALLOC);
    return;
   }
 if( fread(text_buf,1,l,f)==0 )
   {
    show_error(err_msg);
    free(text_buf);
    return;
   }
 fclose(f);
 text=text_buf;
 text[l]=0;

 if( (buf=malloc(4000+3*LEN+3+16))==0 )
   {
    show_error(ERROR_MALLOC);
    if( text_buf!=0 ) free(text_buf);
    return;
   }

 tmp=buf+4000;
 col=tmp+LEN+1;
 str=col+LEN+1;
 
 save_window( X, Y, W, H, buf );
 border_window( BORDER_COLOR, X, Y, W, H, Border22f );
 border_window( BORDER_COLOR, X, Y+H-3, W, 3, Border21if );
 write_char( BORDER_COLOR, BARX, BARY-1, '');
 write_char( BORDER_COLOR, BARX, BARY+BARLEN, '');
        
 
 if( (bs=strstr(text,"<BODY>"))==0 || (bq=strstr(bs+=6,"</BODY>"))==0 )
   {
    bs=text;
    bq=bs+strlen(text);
   }

 while( *bs=='\r' || *bs=='\n' ) bs++;
 while( *bq=='\r' || *bq=='\n' ) bq--;

 if( bs>=bq )
   {
    bs=HTML_DOC_EMPTY;
    bq=bs+strlen(bs);
   }

 i=N;
 p=bq-1;
 if( *p=='\n' ) p--;
 for( ; p!=bs ; p-- )
 if( *p=='\n' )
   {
    i--;
    if( i==0 ) { p++; break; }
   }

 maxtop=p-bs;
 
 nchars++; /* no warnings */

 mesg=0;
 
 top=bs;
 nchars=0;
 seek_cnt=0;
 num_backs=0;
 bar_hooked=0;
 check_mouse=0;
 force_redraw=1;
 first_time=1;
 enable_timer=0;
 
 move_cursor(1,26);
 
 while(1)
    {
     if( target_name!=0 )
       {
        p=target_name;
        q=strchr(p,'\"');
        if( q!=0 ) k=q-p;
        else k=strlen(p);
        if( k<0 || k>LEN-12 ) k=LEN-12;
        if( *p=='#' )  /* local reference */
          {
           sprintf(tmp,"<A NAME=\"%.*s\"",(int)(k-1),p+1);
           p=strstr(bs,tmp);
           if( p!=0 && p<bq )
             {
              while( p!=bs && *p!='\n' ) p--;
              if( *p=='\n' ) p++;
              if( num_backs<MAX_BACKS && !first_time )
                  link_back[num_backs++]=top;
              top=p;
             }
           else
             {
              sprintf(err_msg,HTML_NOT_FOUND,tmp+8);
              mesg=err_msg;
             }
         }
       else
         {
          sprintf(tmp,"%.*s",(unsigned int)k,p);
          if( strncmp(tmp,"http://",7)!=0 &&
              strncmp(tmp,"ftp://",6)!=0 &&
              strncmp(tmp,"news:",5)!=0 &&
              strncmp(tmp,"mailto:",7)!=0  )
             {
              html_view(tmp);
             }
         }

        target_name=0;
       }/* target_name */

     first_time=0;

     if( seek_cnt!=0 )
       {
        if( seek_cnt<0 )
          {
           i=-seek_cnt;
           p=seek_base-1;
           if( *p=='\n' ) p--;
           for( ; p!=bs ; p-- )
            if( *p=='\n' )
              {
               i--;
               if( i==0 ) { p++; break; }
              }
           top=p;
          }
        else if( seek_cnt>0 )
          {
           i=seek_cnt;
           p=seek_base;
           for( ; p!=bq ; p++ )
            if( *p=='\n' )
              {
               i--;
               if( i==0 ) { p++; break; }
              }
           if( p==bq ) { seek_base=bq-1; seek_cnt=-1; continue; }
           top=p;
          }
        
        seek_cnt=0;
       }/* seek */

     if( maxtop!=0 )
        barpos=((top-bs)*(BARLEN))/maxtop;
     else
        barpos=BARLEN-1;

     if( barpos>=BARLEN ) barpos=BARLEN-1;
     
     if( bar_hooked )
       {
        seek_base=top;
        if( forced_barpos==0 )
          {
           top=bs;
           barpos=0;
          }
        else if( forced_barpos==BARLEN-1 && barpos!=forced_barpos )
          {
           old_barpos=barpos;
           seek_base=bq;
           seek_cnt=-N;
           continue;
          }
        else if( forced_barpos<barpos && forced_barpos<old_barpos )
          {
           old_barpos=barpos;
           seek_cnt=-1;
           continue;
          }
        else if( forced_barpos>barpos && forced_barpos>old_barpos )
          {
           old_barpos=barpos;
           seek_cnt=+1;
           continue;
          }
       }/* bar_hooked */
     
     old_barpos=barpos;

     if( top!=old_top ) force_redraw=1;

     if( force_redraw==1 || check_mouse==1 )
       {
        p=top;
        k=0;
        len=0;
        line=0;
        link_started=0;
        link_under_mouse=0;

        while(1)
           {
            if( *p=='\n' || len==LEN || p==bq )
              {
               if( *p!='\n' && p!=bq ) while( *p!='\n' && p!=bq ) p++;
               if( *p=='\n' && p!=bq ) p++;
               
               while( len!=LEN )
                  {
                   tmp[len]=' ';
                   col[len] = TEXT_COLOR;
                   len++;
                  }

               if( force_redraw==1 )
                 {
                  for( i=0, j=0 ; i<LEN ; i++ )
                     {
                      str[j++]=tmp[i];
                      str[j++]=col[i];
                     }
              
                  load_window( X+2, Y+1+line, LEN, 1, str );
                 }

               line++;
               len=0;

               if( p==bq || line==N )
                 {
                  if( line!=N )
                     clear_window( TEXT_COLOR, X+2, Y+1+line, LEN, N-line );
                  break;
                 }
               continue;
              }

            if( *p=='\r' ) { p++; continue; }

            if( *p=='<' && ( p[1]>='A' && p[1]<='Z' || p[1]=='/' ||
                             p[1]>='a' && p[1]<='z' || p[1]=='!' ) )
              {
               if( link_started==0 && _strnicmp(p,"<A HREF=",8)==0 )
                 {
                  link_reference=p+8;
                  link_started=1;
                 }
               else if( link_started==1 && _strnicmp(p,"</A>",4)==0 )
                 {
                  link_started=0;
                 }
               else if( _strnicmp(p,"<HR",3)==0 )
                 {
                  while( len!=LEN ) 
                     {
                      tmp[len]='Ä';
                      col[len]=TEXT_COLOR;
                      len++;
                     }
                 }
               while( *p!='>' && p!=bq ) p++;
               if( *p=='>' ) p++;
               continue;
              }
            
            ch=*p;
            
            if( *p=='&' )
              {
                    if( _strnicmp(p,"&gt;",4)==0 && bq-p>4 ) { ch='>'; p+=3; }
               else if( _strnicmp(p,"&lt;",4)==0 && bq-p>4 ) { ch='<'; p+=3; }
               else if( _strnicmp(p,"&quot;",6)==0 && bq-p>6 ) {ch='\"';p+=5;}
               else if( _strnicmp(p,"&nbsp;",6)==0 && bq-p>6 ) {ch=' '; p+=5;}
              }

            tmp[len] = ( ch!='\t' ) ? ch : ' ';
            col[len] = link_started ? LINK_COLOR : TEXT_COLOR;
            
            if( check_mouse && line==mline && len==mpos && link_started )
               link_under_mouse=link_reference;

            len++;
            if( ch!='\t' || len%8==0 ) p++;
           }

        if( force_redraw==1 )
          {
           bottom=p;
      
           write_string(BORDER_COLOR, X+2+LEN-6, Y+1, "\x1B Back");

           for( i=0 ; i<BARLEN ; i++ )
              {
               write_char( BORDER_COLOR, BARX, BARY+i, (i!=barpos)?'°':'²');
              }

           old_top=top;
           force_redraw=0;
          }
       }/* redraw */

     if( link_under_mouse!=0 )
       {
        p=link_under_mouse+1;
        k=strchr(p,'\"')-p;
        if( k<0 || k>LEN-12 ) k=LEN-12;
        sprintf(tmp,"%.*s",(int)k,p);
        if( mesg==0 ) mesg=tmp;
       }
     
     clear_window(TEXT_COLOR,X+2,TY,LEN,1);
     if( mesg!=0 )
       {
        write_string(TEXT_COLOR,X+2,TY,mesg);
        mesg=0;
       }

     if( enable_timer )
       {
        ev.timer=1;
        get_event( &ev, EV_KEY | EV_MOUSE | EV_TIMER );
        enable_timer=0;
       }
     else
       {
        get_event( &ev, EV_KEY | EV_MOUSE );
       }


     if( ev.ev_type==EV_KEY && ev.key==27 ) break;

     if( ev.ev_type & (EV_MOUSE | EV_TIMER) )
       {
        if( ((ev.ev_type & EV_TIMER) || left_was_pressed==0) && ev.left==1 )
          {
           if( ev.x>X+2+LEN-7 && ev.x<X+2+LEN && ev.y==Y+1 )
             {
              if( num_backs==0 ) break;
              top=link_back[--num_backs];
              force_redraw=1;
             }
           else if( link_under_mouse!=0 )
             {
              p=link_under_mouse;
              if( *p=='\"' ) p++;
              target_name=p;
             }
           else if( ev.x==BARX && ev.y>=BARY-1 && ev.y<=BARY+BARLEN )
             {
              if( ev.y==BARY-1 )					 /* up */
                {
                 if( top!=bs ) { seek_base=top; seek_cnt=-1; }
                 enable_timer=1;
                }
              else if( ev.y==BARY+BARLEN )				 /* down */
                {
                 if( bottom!=bq ) { seek_base=top; seek_cnt=+1; }
                 enable_timer=1;
                }
              else
                {
                 if( ev.y==BARY+barpos ) bar_hooked=1;
                }
             }/* bar */
          }/* left_button_pressed */

        if( ev.x>X+1 && ev.x<X+LEN+2 && ev.y>Y && ev.y<Y+N+1 )
          {
           check_mouse=1;
           mline=ev.y-Y-1;
           mpos=ev.x-X-2;
          }
        else
          {
           check_mouse=0;
           link_under_mouse=0;
          }

        left_was_pressed=ev.left;
        if( ev.left==0 ) bar_hooked=0;
        if( bar_hooked )
          {
           if( ev.y<=BARY ) forced_barpos=0;
           else if( ev.y<BARY+BARLEN ) forced_barpos=ev.y-BARY;
           else forced_barpos=BARLEN-1;
          }
       }/* mouse */
     else if( num_backs!=0 && (ev.key==8 || ev.scan==0x9B00) ) /* back */
       {
        top=link_back[--num_backs];
        force_redraw=1;
       }
     else if( ev.scan==0x47E0 || ev.scan==0x4700 ) /* Home */
       {
        top=bs;
       }
     else if( ev.scan==0x48E0 || ev.scan==0x4800 ) /* up */
       {
        if( top!=bs ) { seek_base=top; seek_cnt=-1; }
       }
     else if( ev.scan==0x49E0 || ev.scan==0x4900 ) /* PgUp */
       {
        if( top!=bs ) { seek_base=top; seek_cnt=1-N; }
       }
     else if( ev.scan==0x4FE0 || ev.scan==0x4F00 ) /* End */
       {
        if( bottom!=bq ) { seek_base=bq; seek_cnt=-N; }
       }
     else if( ev.scan==0x50E0 || ev.scan==0x5000 ) /* down */
       {
        if( bottom!=bq ) { seek_base=top; seek_cnt=+1; }
       }
     else if( ev.scan==0x51E0 || ev.scan==0x5100 ) /* PgDn */
       {
        if( bottom!=bq ) { seek_base=top; seek_cnt=N-1; }
       }
     
    }/* while(1) - main loop */

 load_window(X,Y,W,H,buf);
 free(text_buf);
 free(buf);
}/* html_view */
