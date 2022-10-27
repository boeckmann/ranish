#include "part.h"

void hex_dump_line(long,unsigned char*);


int print_unknown(struct part_long *p)
{
 int i;
 char tmp1[20], tmp2[20];
 
 if( disk_read_rel(p,0,tmp,1)==-1 )
   {
    fprintf(stderr,"Error reading boot sector.\n");
    return FAILED;
   }

 printf("Boot sector dump of an unknown system:\n\n");
 
 for( i=0 ; i<SECT_SIZE ; i+=16 )
  hex_dump_line(i,tmp+i);

 return OK;
}/* print_unknown */


void hex_dump_line(long l, unsigned char *buf)
{
 int i;
 unsigned char *tmp1=tmp+SECT_SIZE;
 unsigned char *tmp2=tmp1+8;
 
 sprintf(tmp1, "0x%04lX  ",l);

 for( i=0 ; i<16 ; i++ )
    {
     sprintf( tmp2, "%02X ", buf[i]);
     tmp2+=3;
     if( i==3 || i==7 || i==11 ) { *tmp2='|'; tmp2++; *tmp2=' '; tmp2++; }
    }

 *tmp2=' ';
 tmp2++;
 
 for( i=0 ; i<16 ; i++ )
    {
     *tmp2=(buf[i]>=32 && buf[i]<128) ? buf[i] : '.';
     tmp2++;
    }

 *tmp2=0;

 puts(tmp1);
 
}/* hex_dump_line */
