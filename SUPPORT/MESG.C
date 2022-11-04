#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* mesg.c  - a little tool to import messages for
 *           specific language into part_msg file
 */

#define USAGE     "Usage: mesg lng [input_file]"

#define SRC_FILE  "part_msg.c"
#define DEST_FILE "part_msg.new"

#define MAX 300

FILE *sf, *df, *mf;

int n;

char *msg[MAX];
int idlen[MAX];
char tmp[4096];
char tmp2[100];

void cmd_error(char *err)
{
 fprintf(stderr,"%s\n",err);
 exit(1);
}


int xread_line(FILE *f)		/* reads extended line - strings that */
{                               /* use multiple lines as one line */
 int i, l;
 fgets(tmp,4096,f);
 if( feof(f) ) return 0;
 
 if( strlen(tmp)<2 || tmp[2]!='(' ) return 1;

 while(1)
    {
     l=strlen(tmp);
     for( i=l-1 ; i>=0 ; i-- )
      if( tmp[i]!='\r' && tmp[i]!='\n' && tmp[i]!='\t' && tmp[i]!=' ' ) break;
     if( i>=0 && tmp[i]==')' ) break;
     if( l>=4095 ) break;
     fgets(tmp+l,4096-l,f);
     if( feof(f) ) break;
    }

 return 1;
}/* xread_line */


void main(int argc, char **argv)
{
 int i, j, k, flag=0, xp;
 
 if( argc!=2 && argc!=3 ) cmd_error(USAGE);

 sf=fopen(SRC_FILE,"r");
 df=fopen(DEST_FILE,"w");

 if( sf==0 || df==0 ) cmd_error("Error opening file");

 if( argc==2 )
   {
    mf=sf;
    sf=0;
   }
 else
   mf=fopen(argv[2],"r");

 if( mf==0 ) cmd_error("Error opening file");
 
 sprintf(tmp2,"%s( ",argv[1]);

#if 0 
 xp=0;
 
 while(1)	/* reading message file --- export */
    {
     if( xread_line(mf)==0 ) break;
     
     if(  tmp[0]==' ' || tmp[0]=='\t' || tmp[0]=='\r' ||
          tmp[0]=='\n' || tmp[0]==0 ) 
        {
         xp=0;
         continue;
        }

     if( tmp[2]=='(' )
       {
        char *p;
        int l=strchr(tmp+4,' ')-(tmp+4)+1;
        if( xp==0 ) { printf("\n%.*s\n\n", l, tmp+4); xp=1; }
        p=tmp+strlen(tmp);
        while( *p==0 || *p=='\r' || *p=='\n' || *p=='\t' || *p==' ' || *p==')' )
             {
              *p=0; p--;
             }
        printf("[%c%c]%s\n",tmp[0],tmp[1],strchr(tmp,'\"'));
       }
    }

 exit(0);
#endif

 while(1)	/* reading message file */
    {
     if( xread_line(mf)==0 ) break;
     
     if( strncmp(tmp,tmp2,4)==0 )
       {
        if( (msg[n]=malloc(strlen(tmp)+1))==0 ) cmd_error("Malloc failed");
        strcpy(msg[n],tmp);
        idlen[n]=strchr(msg[n]+4,' ')-(msg[n]+4)+1;
        n++;
       }
    }
 
 
if( sf!=0 )
 while(1)	/* now lets insert those messages */
    {
     if( xread_line(sf)==0 ) break;
     
     
     for( i=0 ; i<n ; i++ )
      if( msg[i]!=0 && strncmp(tmp,msg[i],idlen[i]+4)==0 )
        {
         fputs(msg[i],df);
         free(msg[i]);
         msg[i]=0;
         flag=1;
         break;
        }
     if( flag==1 ) { flag=0; continue; }
     
     if( flag==2 && ( tmp[0]==' ' || tmp[0]=='\t' ||
                      tmp[0]=='\r' || tmp[0]=='\n' || tmp[0]==0 ) )
       {
        fputs(msg[k],df);
        free(msg[k]);
        msg[k]=0;
        flag=0;
        fputs(tmp,df);
        continue;
       }
     
     for( i=0 ; i<n ; i++ )
      if( msg[i]!=0 && strncmp(tmp+4,msg[i]+4,idlen[i])==0 )
        {
         flag=2;
         k=i;
         break;
        }
     fputs(tmp,df);
    }

 for( i=0 ; i<n ; i++ )   /* now we dump all unimported messages */
  if( msg[i]!=0 )
   fputs(msg[i],df);

 exit(0);
}/* main */
