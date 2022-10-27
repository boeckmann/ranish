#include "part.h"

#include <bios.h>

/*
 This piece of code was derived from program
 part.zip by gary@brolga.cc.uq.oz.au
 I am still looking for a document which would
 tell me why it works.
*/


char *getascii(unsigned int in_data[], int off_start, int off_end);

void print_ide_info(void)
{
 int i, n, hd;
 unsigned long l;
 unsigned *dd=(unsigned*)buf; /* 2048 bytes */
 unsigned port_base, hd_id;
 struct disk_addr daddr;

 fprintf(stderr,"Querying BIOS.  Wait...\n");
 
 printf("\n");
 
 hd=0x80;
 i=0;
 do
   {
    printf("Int13 0x%02X  -> ",hd);
    if( (n=get_disk_info(hd,&dinfo,buf))==-1 )
       printf("Error!\n");
    else
      {
       printf("%5d cyl x %3d heads x %2d sects = %4ldM = %10s sectors\n",
              dinfo.num_cyls, dinfo.num_heads, dinfo.num_sects,
              dinfo.total_sects/2048, sprintf_long(tmp,dinfo.total_sects) );

       daddr.disk=hd;
       daddr.cyl=0;
       daddr.head=0;
       daddr.sect=1;
     
       disk_read(&daddr,buf,1);  /* Wake up if disk is sleeping */
      }
    hd++;
    i++;
   }
 while( n==-1 && i<4 || i<n );

 if( detected_os==SYS_WIN95 )
   {
    fprintf(stderr,"Please exit from Win95 to query IDE controller.\n");
    return;
   }

 printf("\n");

 fprintf(stderr,"Querying IDE controller.  Wait...\n");

 printf("\n");

 for( hd=0x80 ; hd<=0x83 ; hd++ )
    {
     if( hd==0x80 ) { port_base=0x1F0; hd_id=0xA0; printf("Pri Master  -> ");}
     if( hd==0x81 ) { port_base=0x1F0; hd_id=0xB0; printf("Pri Slave   -> ");}
     if( hd==0x82 ) { port_base=0x170; hd_id=0xA0; printf("Sec Master  -> ");}
     if( hd==0x83 ) { port_base=0x170; hd_id=0xB0; printf("Sec Slave   -> ");}
     

     /* Wait for controller not busy */
     l=0;
     while( inportb(port_base+7)!=0x50 && l<20000) 
        { 
         l++;
         if( l%4000==0 ) sleep(1);
        }

     if( l==20000 ) { printf("Time out!\n\n"); continue; }

     /* Get first/second drive */
     outportb(port_base+6, hd_id);

     /* Get drive info data */
     outportb(port_base+7, 0xEC);         

     /* Wait for data ready */
     l=0;
     while( inportb(port_base+7)!=0x58 && l<20000)
        { 
         l++;
         if( l%4000==0 ) sleep(1);
        }
     if( l==20000 ) { printf("Time out!\n\n"); continue; }
     

     /* Read "sector" */
     for( i=0 ; i<256 ; i++ ) dd[i] = inport(port_base);

     printf("%5d cyl x %3d heads x %2d sects = %4ldM = %10s sectors\n",
	 dd[1], dd[3], dd[6],
	 ((long) dd[1] * (long) dd[3] * (long) dd[6])/2048,
	 sprintf_long(tmp,((long) dd[1] * (long) dd[3] * (long) dd[6]) ) );

     printf("\n          Hard Disk Model: %s\n\n", getascii(dd, 27, 46));
/* 
 printf("Model Number ---------------------> %s\n", getascii(dd, 27, 46));
 printf("Serial Number --------------------> %s\n", getascii(dd, 10, 19));
 printf("Controller Revision Number -------> %s\n", getascii(dd, 23, 26));
 printf("Able to do Double Word Transfer --> %6s\n",
	(dd[48] == 0 ? "No" : "Yes"));
 printf("Controller type ------------------>   %04X\n", dd[20]);
 printf("Controller buffer size (bytes) ---> %6u\n", dd[21] * 512);
 printf("Number of ECC bytes transferred --> %6u\n", dd[22]);
 printf("Number of sectors per interrupt --> %6u\n", dd[47]);
*/
     }
}


char *getascii(unsigned int in_data[], int off_start, int off_end)
{
register int i ;
char *pnt ;
static char ret_val[255];

pnt = ret_val ;

for (i = off_start; i <= off_end; i++) {
	*(pnt++) = (char) (in_data[i] / 256);  /* Get High byte */
	*(pnt++) = (char) (in_data[i] % 256);  /* Get Low byte */
}
*pnt = '\0';  /* Make sure it ends in a NULL character */

return(ret_val);
} /* --- end of the subroutine "getascii" --- */
