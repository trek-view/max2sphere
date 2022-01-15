#include <stdio.h>
# include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bitmaplib.h"

/*
   Create a bitmap structure
*/
BITMAP4 *Create_Bitmap(int nx,int ny)
{
   return((BITMAP4 *)malloc(nx*ny*sizeof(BITMAP4)));
}

/*
   Destroy the bitmap structure
*/
void Destroy_Bitmap(BITMAP4 *bm)
{
   free(bm);
}

/*
   Compare two pixels
*/
int Same_BitmapPixel(BITMAP4 p1,BITMAP4 p2)
{
   if (p1.r != p2.r) return(FALSE);
   if (p1.g != p2.g) return(FALSE);
   if (p1.b != p2.b) return(FALSE);
   if (p1.a != p2.a) return(FALSE);
   return(TRUE);
}

/*
   Write a bitmap to a file
   The format is as follows
     1 == tga 
    11 == tga with alpha
    12 == compressed tga
    13 == compressed tga with alpha
     2 == ppm
     3 == rgb
     4 == raw grey scale
     5 == tiff
     6 == EPS colour (Encapsulated PostScript)
     7 == EPS black and white
      8 == raw
     9 == BMP
   A negative format indicates a vertical flip
*/
void Write_Bitmap(FILE *fptr,BITMAP4 *bm,int nx,int ny,int format)
{
   int i,j,offset;
   long index,rowindex;
   int linelength = 0,size;
   char buffer[1024];

   /* Write the header */
   switch (ABS(format)) {
   case 1:
   case 11:
   case 12:
   case 13:
      putc(0,fptr);  /* Length of ID */
      putc(0,fptr);  /* No colour map */
      if (ABS(format) == 12 || ABS(format) == 13) 
         putc(10,fptr); /* compressed RGB */
      else
         putc(2,fptr); /* uncompressed RGB  */ 
      putc(0,fptr); /* Index of colour map entry */
      putc(0,fptr);
      putc(0,fptr); /* Colour map length */
      putc(0,fptr);
      putc(0,fptr); /* Colour map size */
      putc(0,fptr); /* X origin */
      putc(0,fptr);
      putc(0,fptr); /* Y origin */
      putc(0,fptr);
      putc((nx & 0x00ff),fptr); /* X width */
      putc((nx & 0xff00) / 256,fptr);
      putc((ny & 0x00ff),fptr); /* Y width */
      putc((ny & 0xff00) / 256,fptr);
      if (ABS(format) == 11 || ABS(format) == 13) {
         putc(32,fptr);                      /* 32 bit bitmap     */
         putc(0x08,fptr);
      } else {
         putc(24,fptr);                       /* 24 bit bitmap       */
         putc(0x00,fptr);
      }
      break;
   case 2:
      fprintf(fptr,"P6\n# bitmaplib (Paul Bourke)\n%d %d\n255\n",nx,ny);
      break;
   case 3:
      putc(0x01,fptr);
      putc(0xda,fptr);
      putc(0x00,fptr);
      putc(0x01,fptr);
      putc(0x00,fptr);
      putc(0x03,fptr);
      putc((nx & 0xFF00) / 256,fptr);
      putc((nx & 0x00FF),fptr);
      putc((ny & 0xFF00) / 256,fptr);
      putc((ny & 0x00FF),fptr);
      BM_WriteHexString(fptr,"000300000000000000ff00000000");
      fprintf(fptr,"WriteBitmap, pdb");
      putc(0x00,fptr);
      putc(0x00,fptr);
      putc(0x00,fptr);
      putc(0x00,fptr);
      putc(0x00,fptr);
      putc(0x00,fptr);
      putc(0x00,fptr);
      putc(0x00,fptr);
      break;
   case 4:
      break;
   case 5:
      BM_WriteHexString(fptr,"4d4d002a");   /* Little endian & TIFF identifier */
      offset = nx * ny * 3 + 8;
      BM_WriteLongInt(fptr,buffer,offset);
      break;
   case 6:
      fprintf(fptr,"%%!PS-Adobe-3.0 EPSF-3.0\n");
      fprintf(fptr,"%%%%Creator: Created from bitmaplib by Paul Bourke\n");
      fprintf(fptr,"%%%%BoundingBox: %d %d %d %d\n",0,0,nx,ny);
      fprintf(fptr,"%%%%LanguageLevel: 2\n");
      fprintf(fptr,"%%%%Pages: 1\n");
      fprintf(fptr,"%%%%DocumentData: Clean7Bit\n");
      fprintf(fptr,"%d %d scale\n",nx,ny);
      fprintf(fptr,"%d %d 8 [%d 0 0 -%d 0 %d]\n",nx,ny,nx,ny,ny);
      fprintf(fptr,"{currentfile 3 %d mul string readhexstring pop} bind\n",nx);
      fprintf(fptr,"false 3 colorimage\n");
      break;
   case 7:
      fprintf(fptr,"%%!PS-Adobe-3.0 EPSF-3.0\n");
      fprintf(fptr,"%%%%Creator: Created from bitmaplib by Paul Bourke\n");
      fprintf(fptr,"%%%%BoundingBox: %d %d %d %d\n",0,0,nx,ny);
      fprintf(fptr,"%%%%LanguageLevel: 2\n");
      fprintf(fptr,"%%%%Pages: 1\n");
      fprintf(fptr,"%%%%DocumentData: Clean7Bit\n");
      fprintf(fptr,"%d %d scale\n",nx,ny);
      fprintf(fptr,"%d %d 8 [%d 0 0 -%d 0 %d]\n",nx,ny,nx,ny,ny);
      fprintf(fptr,"{currentfile %d string readhexstring pop} bind\n",nx);
      fprintf(fptr,"false 1 colorimage\n");
      break;
   case 8:
      break;
   case 9:
      /* Header 10 bytes */
      putc('B',fptr);
      putc('M',fptr);
      size = nx * ny * 3 + 14 + 40;
      putc((size) % 256,fptr);
      putc((size / 256) % 256,fptr);
      putc((size / 65536) % 256,fptr);
      putc((size / 16777216),fptr);
      putc(0,fptr); putc(0,fptr); 
      putc(0,fptr); putc(0,fptr);
      /* Offset to image data */
      putc(14+40,fptr); putc(0,fptr); putc(0,fptr); putc(0,fptr); 
      /* Information header 40 bytes */
      putc(0x28,fptr); putc(0,fptr); putc(0,fptr); putc(0,fptr); 
      putc((nx) % 256,fptr);
      putc((nx / 256) % 256,fptr);
      putc((nx / 65536) % 256,fptr);
      putc((nx / 16777216),fptr);
      putc((ny) % 256,fptr);
      putc((ny / 256) % 256,fptr);
      putc((ny / 65536) % 256,fptr);
      putc((ny / 16777216),fptr);
      putc(1,fptr); putc(0,fptr); /* One plane */
      putc(24,fptr); putc(0,fptr); /* 24 bits */
      /* Compression type == 0 */
      putc(0,fptr); putc(0,fptr); putc(0,fptr); putc(0,fptr); 
      size = nx * ny * 3;
      putc((size) % 256,fptr);
      putc((size / 256) % 256,fptr);
      putc((size / 65536) % 256,fptr);
      putc((size / 16777216),fptr);
      putc(1,fptr); putc(0,fptr); putc(0,fptr); putc(0,fptr); 
      putc(1,fptr); putc(0,fptr); putc(0,fptr); putc(0,fptr); 
      putc(0,fptr); putc(0,fptr); putc(0,fptr); putc(0,fptr); /* No palette */
      putc(0,fptr); putc(0,fptr); putc(0,fptr); putc(0,fptr); 
      break;
   }

   // Write the binary data 
   for (j=0;j<ny;j++) {
      if (format > 0)
         rowindex = j * nx;
      else
         rowindex = (ny - 1 - j) * nx;
      switch (ABS(format)) {
      case 12:
         WriteTGACompressedRow(fptr,&(bm[rowindex]),nx,3);
         break;
      case 13:
         WriteTGACompressedRow(fptr,&(bm[rowindex]),nx,4);
         break;
      }   
      for (i=0;i<nx;i++) {
         if (format > 0) 
            index = rowindex + i;
         else
            index = rowindex + i;
         switch (ABS(format)) {
         case 1:
         case 11:
         case 9:
            putc(bm[index].b,fptr);
            putc(bm[index].g,fptr);
            putc(bm[index].r,fptr);
            if (ABS(format) == 11)
               putc(bm[index].a,fptr);
            break;
         case 2:
         case 3:
         case 5:
         case 8:
            putc(bm[index].r,fptr);
            putc(bm[index].g,fptr);
            putc(bm[index].b,fptr);
            break;
         case 4:
            putc((bm[index].r+bm[index].g+bm[index].b)/3,fptr);
            break;
         case 6:
            fprintf(fptr,"%02x%02x%02x",bm[index].r,bm[index].g,bm[index].b);
            linelength += 6;
            if (linelength >= 72 || linelength >= nx) {
               fprintf(fptr,"\n");
               linelength = 0;
            }   
            break;
         case 7:
            fprintf(fptr,"%02x",(bm[index].r+bm[index].g+bm[index].b)/3);
            linelength += 2;
            if (linelength >= 72 || linelength >= nx) {
               fprintf(fptr,"\n");
               linelength = 0;
            } 
            break;
         }
      }
   }

   /* Write the footer */
   switch (ABS(format)) {
   case 1:
   case 11:
   case 12:
   case 13:
   case 2:
   case 3:
   case 4:
      break;
   case 5:
      putc(0x00,fptr); /* The number of directory entries (14) */
      putc(0x0e,fptr);

      /* Width tag, short int */
      BM_WriteHexString(fptr,"0100000300000001");
      putc((nx & 0xff00) / 256,fptr);      /* Image width */
      putc((nx & 0x00ff),fptr);
      putc(0x00,fptr);
      putc(0x00,fptr);

      /* Height tag, short int */
      BM_WriteHexString(fptr,"0101000300000001");
      putc((ny & 0xff00) / 256,fptr);    /* Image height */
      putc((ny & 0x00ff),fptr);
      putc(0x00,fptr);
      putc(0x00,fptr);

      /* bits per sample tag, short int */
      BM_WriteHexString(fptr,"0102000300000003");
      offset = nx * ny * 3 + 182;
      BM_WriteLongInt(fptr,buffer,offset);

      /* Compression flag, short int */
      BM_WriteHexString(fptr,"010300030000000100010000");

      /* Photometric interpolation tag, short int */
      BM_WriteHexString(fptr,"010600030000000100020000");

      /* Strip offset tag, long int */
      BM_WriteHexString(fptr,"011100040000000100000008");

      /* Orientation flag, short int */
      BM_WriteHexString(fptr,"011200030000000100010000");

      /* Sample per pixel tag, short int */
      BM_WriteHexString(fptr,"011500030000000100030000");

      /* Rows per strip tag, short int */
      BM_WriteHexString(fptr,"0116000300000001");
      putc((ny & 0xff00) / 256,fptr); 
      putc((ny & 0x00ff),fptr);
      putc(0x00,fptr);
      putc(0x00,fptr);

      /* Strip byte count flag, long int */
      BM_WriteHexString(fptr,"0117000400000001");
      offset = nx * ny * 3;
      BM_WriteLongInt(fptr,buffer,offset);

      /* Minimum sample value flag, short int */
      BM_WriteHexString(fptr,"0118000300000003");
      offset = nx * ny * 3 + 188;
      BM_WriteLongInt(fptr,buffer,offset);

      /* Maximum sample value tag, short int */
      BM_WriteHexString(fptr,"0119000300000003");
      offset = nx * ny * 3 + 194;
      BM_WriteLongInt(fptr,buffer,offset);

      /* Planar configuration tag, short int */
      BM_WriteHexString(fptr,"011c00030000000100010000");

      /* Sample format tag, short int */
      BM_WriteHexString(fptr,"0153000300000003");
      offset = nx * ny * 3 + 200;
      BM_WriteLongInt(fptr,buffer,offset);

      /* End of the directory entry */
      BM_WriteHexString(fptr,"00000000");

      /* Bits for each colour channel */
      BM_WriteHexString(fptr,"000800080008");

      /* Minimum value for each component */
      BM_WriteHexString(fptr,"000000000000");

      /* Maximum value per channel */
      BM_WriteHexString(fptr,"00ff00ff00ff");

      /* Samples per pixel for each channel */
      BM_WriteHexString(fptr,"000100010001");

      break;
   case 6:
   case 7:
      fprintf(fptr,"\n%%%%EOF\n");
      break;
   case 8:
   case 9:
      break;
   }
}

/*
   Write a compressed TGA row
   Depth is either 3 or 4
*/
void WriteTGACompressedRow(FILE *fptr,BITMAP4 *bm,int width,int depth)
{
   int i;
   int counter = 1;
   int pixelstart = 0;
   int packettype = 0;
   int readytowrite = FALSE;
   BITMAP4 currentpixel,nextpixel = {0,0,0,0};

   currentpixel = bm[0];
   for (;;) {
      if (pixelstart+counter >= width)  // Added April to fix strange bug
         readytowrite = TRUE;
      else
         nextpixel = bm[pixelstart+counter];

      if (!readytowrite) {
         if (Same_BitmapPixel(currentpixel,nextpixel)) {
            if (packettype == 0) {
               counter++;
               if (counter >= 128 || (pixelstart + counter) >= width) 
                  readytowrite = TRUE;
            } else {
               counter--;
               readytowrite = TRUE;
            }
         } else {
            if (packettype == 1 || counter <= 1) {
               packettype = 1;
               currentpixel = nextpixel;
               counter++;
               if (counter >= 128 || (pixelstart + counter) >= width)
                  readytowrite = TRUE;
            } else {
               readytowrite = TRUE;
            }
         }
      }

      if (readytowrite) {
         if (pixelstart + counter > width)
            counter = width - pixelstart;
         if (packettype == 0) {
            putc(((counter-1) | 0x80),fptr);
            putc(currentpixel.b,fptr);
            putc(currentpixel.g,fptr);
            putc(currentpixel.r,fptr);
            if (depth == 4)
               putc(currentpixel.a,fptr);
            currentpixel = nextpixel;
         } else {
            putc(counter-1,fptr);
            for (i=0;i<counter;i++) {
               putc(bm[pixelstart+i].b,fptr);
               putc(bm[pixelstart+i].g,fptr);
               putc(bm[pixelstart+i].r,fptr);
               if (depth == 4)
                  putc(bm[pixelstart+i].a,fptr);
            }
         }
         if ((pixelstart = pixelstart + counter) >= width)
            break; /* From for (;;) */
         readytowrite = FALSE;
         packettype = 0;
         counter = 1;
      }
   }
}

void BM_WriteLongInt(FILE *fptr,char *s,long n)
{
   int i;

   s[0] = (n & 0xff000000) / 16777216;
   s[1] = (n & 0x00ff0000) / 65536;
   s[2] = (n & 0x0000ff00) / 256;
   s[3] = (n & 0x000000ff);

   for (i=0;i<4;i++)
      putc(s[i],fptr);
}

void BM_WriteHexString(FILE *fptr,char *s)
{
   unsigned int i,c;
   char hex[3];

   for (i=0;i<strlen(s);i+=2) {
      hex[0] = s[i];
      hex[1] = s[i+1];
      hex[2] = '\0';
      sscanf(hex,"%X",&c);
      putc(c,fptr);
   }
}

/*
   Clear the bitmap to a particular colour
*/
void Erase_Bitmap(BITMAP4 *bm, int nx, int ny, BITMAP4 col)
{
   int i,j;
   long index;

   for (i=0;i<nx;i++) {
      for (j=0;j<ny;j++) {
         index = j * nx + i;
         bm[index] = col;
      }
   }
}

/*
   Scale an image using bicubic interpolation
*/
void BiCubicScale(
   BITMAP4 *bm_in,int nx,int ny,
   BITMAP4 *bm_out,int nnx,int nny)
{
   int i_out,j_out,i_in,j_in,ii,jj;
   int n,m;
   long index;
   double cx,cy,dx,dy,weight;
   double red,green,blue,alpha;
   BITMAP4 col;
 
   for (i_out=0;i_out<nnx;i_out++) {
      for (j_out=0;j_out<nny;j_out++) {
         i_in = (i_out * nx) / nnx;
         j_in = (j_out * ny) / nny;
         cx = i_out * nx / (double)nnx;
         cy = j_out * ny / (double)nny;
         dx = cx - i_in;
         dy = cy - j_in;
         red   = 0;
         green = 0;
         blue  = 0;
         alpha = 0;
         for (m=-1;m<=2;m++) {
            for (n=-1;n<=2;n++) {
               ii = i_in + m;
               jj = j_in + n;
               if (ii < 0)   ii = 0;
               if (ii >= nx) ii = nx-1;
               if (jj < 0)   jj = 0;
               if (jj >= ny) jj = ny-1;
               index = jj * nx + ii;
               weight = BiCubicR(m-dx) * BiCubicR(n-dy);
               // weight = BiCubicR(m-dx) * BiCubicR(dy-n);
               red   += weight * bm_in[index].r;
               green += weight * bm_in[index].g;
               blue  += weight * bm_in[index].b;
               alpha += weight * bm_in[index].a;
            }
         }
         col.r = (int)red;
         col.g = (int)green;
         col.b = (int)blue;
         col.a = (int)alpha;
         bm_out[j_out * nnx + i_out] = col;
      }
   }
}

double BiCubicR(double x)
{
   double xp2,xp1,xm1;
   double r = 0;

   xp2 = x + 2;
   xp1 = x + 1;
   xm1 = x - 1;

   if (xp2 > 0)
      r += xp2 * xp2 * xp2;
   if (xp1 > 0)
      r -= 4 * xp1 * xp1 * xp1;
   if (x > 0)
      r += 6 * x * x * x;
   if (xm1 > 0)
      r -= 4 * xm1 * xm1 * xm1;

   return(r / 6.0);
}

/*
   Scale a bitmap
   Apply a gaussian radial average if r > 0
   r is in units of the input image
*/
void GaussianScale(
   BITMAP4 *bm_in,int nx,int ny,
   BITMAP4 *bm_out,int nnx,int nny,double r)
{
   int i,j,ii,jj,ci,cj;
   long index;
   double x,y,cx,cy,red,green,blue,alpha,dist2,r2,weight,sum;
   BITMAP4 col,black = {0,0,0,255};

   r2 = r*r;

   for (i=0;i<nnx;i++) {
      for (j=0;j<nny;j++) {
         col = black;
         if (r2 <= 0) {
            ci = (i * nx) / nnx;
            cj = (j * ny) / nny;
            index = cj * nx + ci;
            col = bm_in[index];
         } else {
            cx = i * nx / (double)nnx;
            cy = j * ny / (double)nny;
            red   = 0;
            green = 0;
            blue  = 0;
            alpha = 0;
            sum = 0;
            for (x=cx-4*r;x<=cx+4*r+0.01;x++) {
               for (y=cy-4*r;y<=cy+4*r+0.01;y++) {
                  ii = (int)x;
                  jj = (int)y;
                  if (ii < 0)
                     ii = 0;
                  if (ii >= nx)
                     ii = nx-1;;
                  if (jj < 0)
                     jj = 0;
                  if (jj >= ny) 
                     jj = ny-1;
                  dist2 = (cx-x)*(cx-x) + (cy-y)*(cy-y);
                  weight = exp(-0.5*dist2/r2) / (r2*TWOPI);
                  index = jj * nx + ii;
                  red   += weight * bm_in[index].r;
                  green += weight * bm_in[index].g;
                  blue  += weight * bm_in[index].b;
                  alpha += weight * bm_in[index].a;
                  sum += weight;
               }
            }
            col.r = (int)red;
            col.g = (int)green;
            col.b = (int)blue;
            col.a = (int)alpha;
         }
         bm_out[j * nnx + i] = col;
      }
   }
}

/*
   Draw a marker, add new types at will
   Centered at cx,cy of colour col, radius and type
*/
int Draw_Marker(BITMAP4 *bm,int nx,int ny,int cx,int cy,BITMAP4 col,int type,int radius)
{
   int i,j;
   int x,y;
   double theta;

   switch (type) {
   case 0: // Cube
      for (i=-radius;i<=radius;i++) {
         for (j=-radius;j<=radius;j++) {
            x = cx + i;
            y = cy + j;
            Draw_Pixel(bm,nx,ny,x,y,col);
         }
      }
      break;
   case 1: // Filled circle
      for (i=-radius;i<=radius;i++) {
         for (j=-radius;j<=radius;j++) {
            if (sqrt(i*i+(double)j*j) > radius)
               continue;
            x = cx + i;
            y = cy + j;
              Draw_Pixel(bm,nx,ny,x,y,col);
         }
      }
      break;
   case 2: // Outline
      for (i=0;i<=360;i++) { // better ways of doing this
         theta = i * M_PI / 180;
         x = cx + radius*cos(theta);
         y = cy + radius*sin(theta);
         Draw_Pixel(bm,nx,ny,x,y,col);
      }
      break;
   }

   return(TRUE);
}

/*
   Turn on a pixel of a bitmap
*/
int Draw_Pixel(BITMAP4 *bm, int nx, int ny, int x, int y, BITMAP4 col)
{
   long index;

   if (x < 0 || y < 0 || x >= nx || y >= ny)
      return(FALSE);
   index = y * nx + x;
   bm[index] = col;
   return(TRUE);
}

/*
   Fill a recatngle with a colour
	Clipping done in Draw_Pixel()
*/
int Draw_Box(BITMAP4 *bm, int nx, int ny, int x1, int y1, int x2, int y2, BITMAP4 col)
{
   int i,j;

	for (i=x1;i<x2;i++)
		for (j=y1;j<y2;j++)
			Draw_Pixel(bm,nx,ny,i,j,col);

   return(TRUE);
}

/*
   Return the value of a pixel
*/
BITMAP4 Get_Pixel(BITMAP4 *bm, int nx, int ny, int x, int y)
{
   long index;
   BITMAP4 black = {0,0,0,255};

   if (x < 0 || y < 0 || x >= nx || y >= ny)
      return(black);
   index = y * nx + x;
   return(bm[index]);
}

/*
   Draw a line from (x1,y1) to (x2,y2)
   Use colour col
*/
void Draw_Line(BITMAP4 *bm,int nx,int ny,int x1,int y1,int x2,int y2,BITMAP4 col)
{
   int i,j;
   long index;
   double mu,dx,dy,dxy;

   dx = x2 - x1;
   dy = y2 - y1;
   dxy = sqrt(dx*dx + dy*dy);
   if (dxy <= 0) {
      Draw_Pixel(bm,nx,ny,x1,y1,col); 
      return;
   }
   for (mu=0;mu<=2*dxy;mu++) {
      i = (int)(x1 + 0.5 * mu * dx / dxy);
      j = (int)(y1 + 0.5 * mu * dy / dxy);
      if (i < 0 || j < 0 || i >= nx || j >= ny)
         continue;
      index = j * nx + i;
      bm[index] = col;
   }
}
/*
   Line with modifier
   0 = none
   1 = lighter
   2 = darker
*/
void Draw_ModLine(BITMAP4 *bm,int nx,int ny,int x1,int y1,int x2,int y2,BITMAP4 col,int mod)
{
   int i,j,ilast = -1,jlast = -1;
   long index;
   double mu,dx,dy,dxy;
   BITMAP4 col1;

   dx = x2 - x1;
   dy = y2 - y1;
   dxy = MAX(dx,dy);
   if (dxy <= 0) {
      Draw_Pixel(bm,nx,ny,x1,y1,col);
      return;
   }

   for (mu=0;mu<=1;mu+=1/dxy) {
      i = (int)(x1 + mu * dx);
      j = (int)(y1 + mu * dy);
      if (i == ilast && i == jlast) 
         break;
      if (i < 0 || j < 0 || i >= nx || j >= ny)
         continue;
      index = j * nx + i;

      col1 = bm[index]; // Existing colour
      switch (mod) {
      case 1: // Lighter
         if (col1.r*col1.r + col1.g*col1.g + col1.b*col1.b <= col.r*col.r + col.g*col.g + col.b*col.b)
            bm[index] = col;
         break;
      case 2: // Darker
         if (col1.r*col1.r + col1.g*col1.g + col1.b*col1.b >= col.r*col.r + col.g*col.g + col.b*col.b)
            bm[index] = col;
         break;
      case 0:
      default:
         bm[index] = col;
         break;
      }
      ilast = i;
      jlast = j;
   }
}

/*
   Scale a RGB value, dealing with clipping issues
*/
BITMAP4 Scale_Pixel(BITMAP4 pixelin,double scale)
{
   BITMAP4 pixelout;
   double r,g,b,a=0;

   r = pixelin.r * scale;
   g = pixelin.g * scale;
   b = pixelin.b * scale;
   a = pixelin.a * scale;
   
   if (r < 000) r = 0;
   if (r > 255) r = 255;
   if (g < 000) g = 0;
   if (g > 255) g = 255;
   if (b < 000) b = 0;
   if (b > 255) b = 255;
   if (a < 000) a = 0;
   if (a > 255) a = 255;

   pixelout.r = (int)r;
   pixelout.g = (int)g;
   pixelout.b = (int)b;
   pixelout.a = (int)a;

   return(pixelout);
}

/*
   Flip an image about an axis
   mode == 1 for horizontal
   mode == 0 for vertical
   This library assumes the (0,0) coordinate is top left
*/
void Flip_Bitmap(BITMAP4 *image,int width,int height,int mode)
{
   int i,j;
   long index1,index2;
   BITMAP4 p;

   switch (mode) {
   case 0:
      for (j=0;j<height/2;j++) {
         for (i=0;i<width;i++) {
            index1 = j * width + i;
            index2 = (height-1-j) * width + i;
            p = image[index1];
            image[index1] = image[index2];
            image[index2] = p;
         }
      }
      break;
   case 1:
      for (j=0;j<height;j++) {
         for (i=0;i<width/2;i++) {
            index1 = j * width + i;
            index2 = j * width + (width-1-i);
            p = image[index1];
            image[index1] = image[index2];
            image[index2] = p;
         }
      }
      break;
   }
}

int IsTGA(char *fname)
{
   int i;
   char s[256];

   strcpy(s,fname);
   for (i=0;i<strlen(s);i++)
      if (s[i] >= 'A' && s[i] <= 'Z')
         s[i] += ('a' - 'A');

   if (strstr(s,".tga") != NULL)
      return(TRUE);
   if (strstr(s,".TGA") != NULL)
      return(TRUE);

   return(FALSE);
}

/*
   Get the size and depth of a TGA file
*/
void TGA_Info(FILE *fptr,int *width,int *height,int *depth)
{
   int lo,hi;
   TGAHEADER header;

   *width = 0;
   *height = 0;
   *depth = 0;

   header.idlength = fgetc(fptr);
   header.colourmaptype = fgetc(fptr);
   header.datatypecode = fgetc(fptr);
   if (fread(&header.colourmaporigin,2,1,fptr) != 1)
      return;
   if (fread(&header.colourmaplength,2,1,fptr) != 1)
      return;
   header.colourmapdepth = fgetc(fptr);
   if (fread(&header.x_origin,2,1,fptr) != 1)
      return;
   if (fread(&header.y_origin,2,1,fptr) != 1)
      return;
   lo = fgetc(fptr);
   hi = fgetc(fptr);
   header.width = hi*256 + lo;
   lo = fgetc(fptr);
   hi = fgetc(fptr);
   header.height = hi*256 + lo;
   header.bitsperpixel = fgetc(fptr);
   header.imagedescriptor = fgetc(fptr);

   *width  = header.width;
   *height = header.height;
   *depth  = header.bitsperpixel;

   rewind(fptr);
}

/*
   Read the TGA image data
   Return 0 on success
   Error codes
      1 - Failed to get legal colour type code
      2 - Failed to get legal bits per pixel
      3 - Failed to get legal colour map type
      4 - Failed to read colour data
      5 - Failed to read colour table
     10 - fread failures
*/
int TGA_Read(FILE *fptr,BITMAP4 *image,int *width,int *height)
{
   int n=0,i,j;
   int lo,hi,index;
   int bytes2read,skipover = 0;  
   TGAHEADER header;
   unsigned char p[5];
   BITMAP4 *ctable = NULL;

   // Read the header 
   header.idlength = fgetc(fptr);
   header.colourmaptype = fgetc(fptr);
   header.datatypecode = fgetc(fptr);
   lo = fgetc(fptr);
   hi = fgetc(fptr);
   header.colourmaporigin = hi*256 + lo;
   lo = fgetc(fptr);
   hi = fgetc(fptr);
   header.colourmaplength = hi*256 + lo;
   header.colourmapdepth = fgetc(fptr);
   if (fread(&header.x_origin,2,1,fptr) != 1)
      return(10);
   if (fread(&header.y_origin,2,1,fptr) != 1)
      return(10);
   lo = fgetc(fptr);
   hi = fgetc(fptr);
   header.width = hi*256 + lo;
   *width = header.width;
   lo = fgetc(fptr);
   hi = fgetc(fptr);
   header.height = hi*256 + lo;
   *height = header.height;
   header.bitsperpixel = fgetc(fptr);
   header.imagedescriptor = fgetc(fptr);
   
   /* 
      Can only handle image type 1, 2, 3 and 10 
       1 - index colour uncompressed
       2 - rgb uncompressed
      10 - rgb rle comrpessed
       3 - grey scale uncompressed
       9 - rle index colour (unsupported)
      11 - rle black and white
   */ 
   if (header.datatypecode != 1 &&
       header.datatypecode != 2 && 
       header.datatypecode != 3 &&
       header.datatypecode != 11 &&
       header.datatypecode != 10) {
      return(1);
   }

   /* Can only handle pixel depths of 8, 16, 24, and 32 */
   if (header.bitsperpixel != 8 &&
       header.bitsperpixel != 16 &&
       header.bitsperpixel != 24 && 
       header.bitsperpixel != 32) {
      return(2);
   }

   /* 
      Can only handle colour map types of 0 and 1
      Ignore the colour map case (1) for RGB images!
   */
   if (header.colourmaptype != 0 && header.colourmaptype != 1) {
      return(3);
   }

   /* Read the colour index table */
   if (header.datatypecode == 1) {
      ctable = (BITMAP4 *)malloc(header.colourmaplength*sizeof(BITMAP4));
      bytes2read = header.colourmapdepth / 8;
      for (i=0;i<header.colourmaplength;i++) {
         if ((int)fread(p,1,bytes2read,fptr) != bytes2read) 
            return(5);
         TGA_MergeBytes(&(ctable[i]),p,bytes2read);
      }
   } 

   /* Go to the start of the image data */
   skipover = 18;
   skipover += header.idlength;
   skipover += header.colourmaptype * header.colourmaplength * header.colourmapdepth / 8;
   fseek(fptr,skipover,SEEK_SET);

   /* Read the image */
   bytes2read = header.bitsperpixel / 8;
   while (n < header.width * header.height) {
      if (header.datatypecode == 1) {                     /* Indexed uncompressed */
         if ((index = fgetc(fptr)) == EOF) 
            return(4);
         if (index < 0) 
            index = 0;
         if (index >= header.colourmaplength)
            index = header.colourmaplength-1;
         image[n] = ctable[index];
         n++;
      } else if (header.datatypecode == 2) {              /* RGB Uncompressed */
         if ((int)fread(p,1,bytes2read,fptr) != bytes2read)
            return(4);
         TGA_MergeBytes(&(image[n]),p,bytes2read);
         n++;
      } else if (header.datatypecode == 3) {              /* Grey Uncompressed */
         if ((int)fread(p,1,bytes2read,fptr) != bytes2read) 
            return(4);
         TGA_MergeBytes(&(image[n]),p,bytes2read);
         n++;
      } else if (header.datatypecode == 10) {             /* RGB Compressed */
         if ((int)fread(p,1,bytes2read+1,fptr) != bytes2read+1) 
            return(4);
         j = p[0] & 0x7f;
         TGA_MergeBytes(&(image[n]),&(p[1]),bytes2read);
         n++;
         if (p[0] & 0x80) {         /* RLE chunk */
            for (i=0;i<j;i++) {
               TGA_MergeBytes(&(image[n]),&(p[1]),bytes2read);
               n++;
            }
         } else {                   /* Normal chunk */
            for (i=0;i<j;i++) {
               if ((int)fread(p,1,bytes2read,fptr) != bytes2read) 
                  return(6);
               TGA_MergeBytes(&(image[n]),p,bytes2read);
               n++;
            }
         }
      } else if (header.datatypecode == 11) {             /* Compressed black and white */
         if ((int)fread(p,1,bytes2read+1,fptr) != bytes2read+1)
            return(4);
         j = p[0] & 0x7f;
         TGA_MergeBytes(&(image[n]),&(p[1]),bytes2read);
         n++;
         if (p[0] & 0x80) {         /* RLE chunk */       
            for (i=0;i<j;i++) {
               TGA_MergeBytes(&(image[n]),&(p[1]),bytes2read);
               n++;
            }
         } else {                   /* Normal chunk */
            for (i=0;i<j;i++) {
               if ((int)fread(p,1,bytes2read,fptr) != bytes2read)
                  return(6);
               TGA_MergeBytes(&(image[n]),p,bytes2read);
               n++;
            }
         }
      }
   }

   /* Flip the image ? */
   if ((header.imagedescriptor & 0x20) == 32) 
      Flip_Bitmap(image,header.width,header.height,0);

   return(0);
}

void TGA_MergeBytes(BITMAP4 *pixel,unsigned char *p,int bytes)
{
   if (bytes == 4) {
      pixel->r = p[2];
      pixel->g = p[1];
      pixel->b = p[0];
      pixel->a = p[3];
   } else if (bytes == 3) {
      pixel->r = p[2];
      pixel->g = p[1];
      pixel->b = p[0];
      pixel->a = 255; 
   } else if (bytes == 2) {
      pixel->r = (p[1] & 0x7c) << 1;
      pixel->g = ((p[1] & 0x03) << 6) | ((p[0] & 0xe0) >> 2);
      pixel->b = (p[0] & 0x1f) << 3;
      pixel->a = (p[1] & 0x80);
   } else if (bytes == 1) {
      pixel->r = p[0];
      pixel->g = p[0];
      pixel->b = p[0];
      pixel->a = 255;
   }
}

/*
   Is this a PPM file
*/
int IsPPM(char *fname)
{  
   int i;
   char s[256];
   
   strcpy(s,fname);
   for (i=0;i<strlen(s);i++)
      if (s[i] >= 'A' && s[i] <= 'Z')
         s[i] += ('a' - 'A');
   
   if (strstr(s,".ppm") != NULL)
      return(TRUE);
   if (strstr(s,".PPM") != NULL)
      return(TRUE);
   
   return(FALSE);
}

/*
   Get the size and depth of a PPM file
*/
int PPM_Info(FILE *fptr,int *width,int *height,int *depth)
{
   char s[256];

   *width = 0;
   *height = 0;
   *depth = 0;

   // Magic number
   if (fgets(s,254,fptr) == NULL)
      return(FALSE);

   // Header and/or dimensions
   if (fgets(s,254,fptr) == NULL)
      return(FALSE);
   while (s[0] == '#') {
      if (fgets(s,254,fptr) == NULL)
         return(FALSE);
   }
   if (sscanf(s,"%d %d",width,height) != 2)
      return(FALSE);

   // Maximum value
   if (fgets(s,254,fptr) == NULL)
      return(FALSE);
   if (sscanf(s,"%d",depth) != 1)
      return(FALSE);
   
   rewind(fptr);
   return(TRUE);
}

/*
   Read PPM file, does not handle all cases!
   Intended mainly for 16 bit, depth parameter 65535
*/
int PPM_Read(FILE *fptr,COLOUR16 *img,int *width,int *height,int *depth)
{
   int i,j,index;
   unsigned short r,g,b;
   char s[256];

   // Magic number
   if (fgets(s,254,fptr) == NULL)
      return(FALSE);

   // Header and/or dimensions
   if (fgets(s,254,fptr) == NULL)
      return(FALSE);
   while (s[0] == '#') {
      if (fgets(s,254,fptr) == NULL)
         return(FALSE);
   }
   if (sscanf(s,"%d %d",width,height) != 2)
      return(FALSE);

   // Maximum value
   if (fgets(s,254,fptr) == NULL)
      return(FALSE);
   if (sscanf(s,"%d",depth) != 1)
      return(FALSE);
 
   for (j=0;j<(*height);j++) {
      for (i=0;i<(*width);i++) {
         index = (*height-1-j) * (*width) + i;
         if (!Read_UShort(fptr,&r,TRUE))
            return(FALSE);
         if (!Read_UShort(fptr,&g,TRUE))
            return(FALSE);
         if (!Read_UShort(fptr,&b,TRUE))
            return(FALSE);
         img[index].r = r;
         img[index].g = g;
         img[index].b = b;
      }
   }

   return(TRUE);
}

/*
   Write a PPM file
*/
int PPM_Write(FILE *fptr,COLOUR16 *img,int width,int height,int depth)
{
   int i,j,index;

   fprintf(fptr,"P6\n");
   fprintf(fptr,"# Saved from bitmaplib (Paul Bourke)\n");
   fprintf(fptr,"%d %d\n",width,height);
   fprintf(fptr,"%d\n",depth);
  
   for (j=height-1;j>=0;j--) {
      for (i=0;i<width;i++) {
         index = j * width + i;
         if (!Write_UShort(fptr,img[index].r,TRUE))
            return(FALSE);
         if (!Write_UShort(fptr,img[index].g,TRUE))
            return(FALSE);
         if (!Write_UShort(fptr,img[index].b,TRUE))
            return(FALSE);
      }
   }

   return(TRUE);
}

BITMAP4 YUV_to_Bitmap(int y,int u,int v)
{  
   int r,g,b; 
   BITMAP4 bm = {0,0,0,0};
   
   // u and v are +-0.5
   u -= 128;
   v -= 128;
   
   r = y + 1.370705 * v;
   g = y - 0.698001 * v - 0.337633 * u;
   b = y + 1.732446 * u;

/*
   r = y + 1.402 * v;
   g = y - 0.344 * u - 0.714 * v;
   b = y + 1.772 * u;
*/
/*
   y -= 16;
   r = 1.164 * y + 1.596 * v;
   g = 1.164 * y - 0.392 * u - 0.813 * v;
   b = 1.164 * y + 2.017 * u;
*/

   if (r < 0) r = 0;
   if (g < 0) g = 0;
   if (b < 0) b = 0;
   if (r > 255) r = 255;
   if (g > 255) g = 255;
   if (b > 255) b = 255;
   bm.r = r;
   bm.g = g;
   bm.b = b;
   bm.a = 0;

   return(bm);
}

/*
   Get the size and depth of a BMP file
*/
int BMP_Info(FILE *fptr,int *width,int *height,int *depth)
{
   HEADER header;
   INFOHEADER infoheader;

   *width = 0;
   *height = 0;

   // Read header 
   Read_UShort(fptr,&header.type,FALSE);
   Read_UInt(fptr,&header.size,FALSE);
   Read_UShort(fptr,&header.reserved1,FALSE);
   Read_UShort(fptr,&header.reserved2,FALSE);
   Read_UInt(fptr,&header.offset,FALSE);

   /*
      fprintf(stderr,"ID is: %d, should be %d\n",header.type,'M'*256+'B');
      fprintf(stderr,"File size is %d bytes\n",header.size);
      fprintf(stderr,"Offset to image data is %d bytes\n",header.offset);
   */

   // Read information header 
   if (fread(&infoheader,sizeof(INFOHEADER),1,fptr) != 1) {
      fprintf(stderr,"Failed to read BMP info header\n");
      return(FALSE);
   }
   /*
      fprintf(stderr,"header size = %d\n",infoheader.size);
      fprintf(stderr,"Image size = %d x %d\n",infoheader.width,infoheader.height);
      fprintf(stderr,"Number of colour planes is %d\n",infoheader.planes);
      fprintf(stderr,"Bits per pixel is %d\n",infoheader.bits);
      fprintf(stderr,"Compression type is %d\n",infoheader.compression);
      fprintf(stderr,"Number of colours is %d\n",infoheader.ncolours);
      fprintf(stderr,"Number of required colours is %d\n",infoheader.importantcolours);
      fprintf(stderr,"Horizontal resolution is %d\n",infoheader.xresolution);
      fprintf(stderr,"Vertical resolution is %d\n",infoheader.xresolution);
   */

   *width = ABS(infoheader.width);
   *height = ABS(infoheader.height); // Supposedly supports image flipping
   *depth = infoheader.bits;

   rewind(fptr);
   return(TRUE);
}

/*
   Read the BMP image data
   Return 0 on success
   Error codes
      1 - Failed to read header
      2 - Failed to read colour information
      3 - Failed to read image data
*/
int BMP_Read(FILE *fptr,BITMAP4 *image,int *width,int *height)
{
   int i,j;
   unsigned char grey,r,g,b,a;
   HEADER header;
   INFOHEADER infoheader;
   COLOURINDEX colourindex[256];
   int gotindex = FALSE;
   int flip = FALSE;

   // Read header
   Read_UShort(fptr,&header.type,FALSE);
   Read_UInt(fptr,&header.size,FALSE);
   Read_UShort(fptr,&header.reserved1,FALSE);
   Read_UShort(fptr,&header.reserved2,FALSE);
   Read_UInt(fptr,&header.offset,FALSE);

   // Read information header
   if (fread(&infoheader,sizeof(INFOHEADER),1,fptr) != 1) {
      return(1);
   }
   *width = ABS(infoheader.width);
   *height = ABS(infoheader.height); // Supposedly supports image flipping
   if (infoheader.height < 0)
      flip = TRUE;

   // Read the lookup table if there is one 
   for (i=0;i<255;i++) {
      colourindex[i].r = rand() % 256;
      colourindex[i].g = rand() % 256;
      colourindex[i].b = rand() % 256;
      colourindex[i].junk = rand() % 256;
   }
   if (infoheader.ncolours > 0) {
      for (i=0;i<(int)infoheader.ncolours;i++) {
         if (fread(&colourindex[i].b,sizeof(unsigned char),1,fptr) != 1) 
            return(1);
         if (fread(&colourindex[i].g,sizeof(unsigned char),1,fptr) != 1) 
            return(1);
         if (fread(&colourindex[i].r,sizeof(unsigned char),1,fptr) != 1) 
            return(1);
         if (fread(&colourindex[i].junk,sizeof(unsigned char),1,fptr) != 1) 
            return(1);
      }
      gotindex = TRUE;
   }

   // Seek to the start of the image data 
   fseek(fptr,header.offset,SEEK_SET);

   // Read the image 
   for (j=0;j<(*height);j++) {
      for (i=0;i<(*width);i++) {

         switch (infoheader.bits) {
         case 1:
            break;
         case 4:
            break;
         case 8:
            if (fread(&grey,sizeof(unsigned char),1,fptr) != 1) 
               return(2);
            if (gotindex) {
               image[j*(*width)+i].r = colourindex[grey].r;
               image[j*(*width)+i].g = colourindex[grey].g;
               image[j*(*width)+i].b = colourindex[grey].b;
            } else {
               image[j*(*width)+i].r = grey;
               image[j*(*width)+i].g = grey;
               image[j*(*width)+i].b = grey;
            }
            break;
         case 24:
         case 32:
            if (fread(&b,sizeof(unsigned char),1,fptr) != 1) 
               return(2);
            if (fread(&g,sizeof(unsigned char),1,fptr) != 1) 
               return(2);
            if (fread(&r,sizeof(unsigned char),1,fptr) != 1) 
               return(2);
            if (infoheader.bits == 32) {
               if (fread(&a,sizeof(unsigned char),1,fptr) != 1)
                  return(2);
            }
            image[j*(*width)+i].r = r;
            image[j*(*width)+i].g = g;
            image[j*(*width)+i].b = b;
            break;
         }

      } /* i */
   } /* j */

   if (flip)
      Flip_Bitmap(image,(*width),(*height),0);

   return(0);
}

int IsRAW(char *fname)
{
   int i;
   char s[256];

   strcpy(s,fname);
   for (i=0;i<strlen(s);i++)
      if (s[i] >= 'A' && s[i] <= 'Z')
         s[i] += ('a' - 'A');

   if (strstr(s,".raw") != NULL)
      return(TRUE);
   if (strstr(s,".rgb") != NULL)
      return(TRUE);
   if (strstr(s,".RAW") != NULL)
      return(TRUE);
   if (strstr(s,".RGB") != NULL)
      return(TRUE);

   return(FALSE);
}

/*
   Read raw file
   Note, sticks to convention for other functions that (0,0) is bottom left
*/
int RAW_Read(FILE *fptr,COLOUR16 *image,int w,int h,int swap)
{
   int i,j,index;
   short unsigned r,g,b;

   for (j=0;j<h;j++) {
      for (i=0;i<w;i++) {
         index = (h-1-j) * w + i;
         if (!Read_UShort(fptr,&r,swap))
            return(FALSE);
         if (!Read_UShort(fptr,&g,swap))
            return(FALSE);
         if (!Read_UShort(fptr,&b,swap))
            return(FALSE);
         image[index].r = r;
         image[index].g = g;
         image[index].b = b;
      }
   }

   return(TRUE);
}

/*
   Write raw file
   In native byte swap order !?
*/
int RAW_Write(FILE *fptr,COLOUR16 *image,int w,int h)
{
   int i,j,index;

   for (j=0;j<h;j++) {
      for (i=0;i<w;i++) {
         index = (h-1-j) * w + i;
         fwrite(&image[index],sizeof(COLOUR16),1,fptr);
      }
   }

   return(TRUE);
}

/*
   Read a possibly byte swapped unsigned short integer
*/
int Read_UShort(FILE *fptr,short unsigned *n,int swap)
{
   unsigned char *cptr,tmp;

   if (fread(n,2,1,fptr) != 1)
      return(FALSE);
   if (swap) {
      cptr = (unsigned char *)n;
      tmp = cptr[0];
      cptr[0] = cptr[1];
      cptr[1] =tmp;
   }
   return(TRUE);
}

/*
   Write a possibly byte swapped unsigned short integer
*/
int Write_UShort(FILE *fptr,short unsigned n,int swap)
{
   unsigned char *cptr,tmp;

   if (!swap) {
      if (fwrite(&n,2,1,fptr) != 1)
         return(FALSE);
   } else {
      cptr = (unsigned char *)(&n);
      tmp = cptr[0];
      cptr[0] = cptr[1];
      cptr[1] =tmp;
      if (fwrite(&n,2,1,fptr) != 1)
         return(FALSE);
   }
   return(TRUE);
}

/*
   Read a possibly byte swapped unsigned integer
*/
int Read_UInt(FILE *fptr,unsigned int *n,int swap)
{
   unsigned char *cptr,tmp;

   if (fread(n,4,1,fptr) != 1)
      return(FALSE);
   if (swap) {
      cptr = (unsigned char *)n;
      tmp = cptr[0];
      cptr[0] = cptr[3];
      cptr[3] = tmp;
      tmp = cptr[1];
      cptr[1] = cptr[2];
      cptr[2] = tmp;
   }
   return(TRUE);
}

#ifdef ADDJPEG
int IsJPEG(char *fname)
{
   int i;
   char s[256];

   strcpy(s,fname);
   for (i=0;i<strlen(s);i++)
      if (s[i] >= 'A' && s[i] <= 'Z')
         s[i] += ('a' - 'A');

   if (strstr(s,".jpg") != NULL)
      return(TRUE);
   if (strstr(s,".jpeg") != NULL)
      return(TRUE);
   if (strstr(s,".JPG") != NULL)
      return(TRUE);
   if (strstr(s,".JPEG") != NULL)
      return(TRUE);

   return(FALSE);
}

/*
   Write a JPEG file
   Quality is 0 to 100
   Negative means flip vertically
*/
int JPEG_Write(FILE *fptr,BITMAP4 *image,int width,int height,int quality)
{
   int index;
   int i,j,flip=FALSE;
   struct jpeg_compress_struct cinfo;
   JSAMPROW row_pointer[1];
   JSAMPLE *jimage = NULL;
   struct jpeg_error_mgr jerr;

   if (quality > 0) // Historical
      flip = TRUE;
   quality = ABS(quality);

   if ((jimage = malloc(width*3)) == NULL)
      return(1);

   // Error handler
   cinfo.err = jpeg_std_error(&jerr);

   // Initialize JPEG compression object.
   jpeg_create_compress(&cinfo);
   
   // Associate with output stream
   jpeg_stdio_dest(&cinfo,fptr);

   // Fill out values
   cinfo.image_width = width;
   cinfo.image_height = height;
   cinfo.input_components = 3;
   cinfo.in_color_space = JCS_RGB;

   // Default compression settings
   jpeg_set_defaults(&cinfo);
   jpeg_set_quality(&cinfo, quality, TRUE); // limit to baseline-JPEG values

   // Start cmpressor
   jpeg_start_compress(&cinfo, TRUE);

   row_pointer[0] = jimage;

   j = 0;
   while (cinfo.next_scanline < cinfo.image_height) {
      for (i=0;i<width;i++) {
         if (flip)
            index = (height-1-j) * width + i;
         else
            index = j * width + i;
         jimage[3*i  ] = image[index].r;
         jimage[3*i+1] = image[index].g;
         jimage[3*i+2] = image[index].b;
      }
      jpeg_write_scanlines(&cinfo,row_pointer,1);
      j++;
   }

   jpeg_finish_compress(&cinfo);
   jpeg_destroy_compress(&cinfo);

   free(jimage);
   return(TRUE);
}

/*
   Get dimensions of a JPEG image
*/
int JPEG_Info(FILE *fptr,int *width,int *height,int *depth)
{
   int row_stride;
   struct jpeg_decompress_struct cinfo;
   struct jpeg_error_mgr jerr;
   JSAMPARRAY buffer;   

   // Error handler
   cinfo.err = jpeg_std_error(&jerr);

   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, fptr);

   // Read header
   jpeg_read_header(&cinfo, TRUE);

   // Read one scan line
   jpeg_start_decompress(&cinfo);
   row_stride = cinfo.output_width * cinfo.output_components;
   buffer = (*cinfo.mem->alloc_sarray)
      ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
   while (cinfo.output_scanline < cinfo.output_height) 
      jpeg_read_scanlines(&cinfo,(JSAMPARRAY)buffer,1);

   *width = cinfo.output_width;
   *height = cinfo.output_height;
   *depth = 8*cinfo.output_components;

   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);

   rewind(fptr);

   return(TRUE);
}

/*
   Read a JPEG image
*/
int JPEG_Read(FILE *fptr,BITMAP4 *image,int *width,int *height)
{
   int i,j;
   int row_stride;
   struct jpeg_decompress_struct cinfo;
   struct jpeg_error_mgr jerr;
   JSAMPLE *buffer;

   // Error handler
   cinfo.err = jpeg_std_error(&jerr);

   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, fptr);

   // Read header
   jpeg_read_header(&cinfo, TRUE);
   jpeg_start_decompress(&cinfo);

   *width = cinfo.output_width;
   *height = cinfo.output_height;

   // Can only handle RGB JPEG images at this stage
   if (cinfo.output_components != 3) 
      return(1);

   // buffer for one scan line
   row_stride = cinfo.output_width * cinfo.output_components;
   if ((buffer = malloc(row_stride * sizeof(JSAMPLE))) == NULL) 
      return(2);

   j = cinfo.output_height-1;
   while (cinfo.output_scanline < cinfo.output_height) {
      jpeg_read_scanlines(&cinfo,&buffer,1);
      for (i=0;i<cinfo.output_width;i++) {
         image[j*cinfo.output_width+i].r = buffer[3*i];
         image[j*cinfo.output_width+i].g = buffer[3*i+1];
         image[j*cinfo.output_width+i].b = buffer[3*i+2];
         image[j*cinfo.output_width+i].a = 255;
      }
      j--;
   }

   // Finish
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   free(buffer);

   return(0);
}
#endif

#ifdef ADDPNG
int IsPNG(char *fname)
{
   int i;
   char s[256];

   strcpy(s,fname);
   for (i=0;i<strlen(s);i++)
      if (s[i] >= 'A' && s[i] <= 'Z')
         s[i] += ('a' - 'A');

   if (strstr(s,".png") != NULL)
      return(TRUE);
   if (strstr(s,".PNG") != NULL)
      return(TRUE);

   return(FALSE);
}

/*
   Read the PNG image specs
   Return 0 on success
*/   
int PNG_Info(FILE *fptr,int *width,int *height,int *depth)
{
   png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png)
      abort();

   png_infop info = png_create_info_struct(png);
   if (!info)
      abort();

   if(setjmp(png_jmpbuf(png))) 
      abort();

   png_init_io(png, fptr);
   png_read_info(png, info);

   *width = png_get_image_width(png, info);
   *height = png_get_image_height(png, info);
   *depth = png_get_bit_depth(png, info);
   rewind(fptr);

   return(0);
}

/*
   Read the PNG image data 
   Return 0 on success
   Error codes
      1 - Failed to read header
      2 - Failed to read colour information
      3 - Failed to read image data
*/
int PNG_Read(FILE *fptr,BITMAP4 *image,int *owidth,int *oheight)
{
   int index; 

   png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png)
      return(1);

   png_infop info = png_create_info_struct(png);
   if (!info)
      return(1);

   if (setjmp(png_jmpbuf(png)))
      return(1);

   png_init_io(png,fptr);
   png_read_info(png,info);

   int width = png_get_image_width(png, info);
   int height = png_get_image_height(png, info);
   png_byte color_type = png_get_color_type(png, info);
   png_byte bit_depth = png_get_bit_depth(png, info);

   // Read any color_type into 8bit depth, RGBA format.
   // See http://www.libpng.org/pub/png/libpng-manual.txt
   if (bit_depth == 16)
      png_set_strip_16(png);

   if (color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_palette_to_rgb(png);

   // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
   if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
      png_set_expand_gray_1_2_4_to_8(png);

   if (png_get_valid(png, info, PNG_INFO_tRNS))
      png_set_tRNS_to_alpha(png);

   // These color_type don't have an alpha channel then fill it with 0xff.
   if (color_type == PNG_COLOR_TYPE_RGB ||
       color_type == PNG_COLOR_TYPE_GRAY ||
       color_type == PNG_COLOR_TYPE_PALETTE){
      png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
   }
   if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) 
      png_set_gray_to_rgb(png);

   png_read_update_info(png, info);

   int x,y;
   png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
   for (y=0;y<height;y++) 
      row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));

   png_read_image(png, row_pointers);

   for (y=0;y<height;y++) {
      png_byte* row = row_pointers[y];
      for (x=0;x<width;x++) {
         index = (height-1-y) * width + x; // bitmaplib convention has 0 at top left
         png_byte* ptr = &(row[x*4]);
         image[index].r = ptr[0];
         image[index].g = ptr[1];
         image[index].b = ptr[2];
         image[index].a = ptr[3];
      }
   }

   for (y=0;y<height;y++)
      free(row_pointers[y]);
   free(row_pointers);

   *owidth = width;
   *oheight = height;
   return(0);
}

int PNG_Write(FILE *fptr,BITMAP4 *image,int width,int height,int flip)
{
   int index;

   png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png)
      return(1);

   png_infop info = png_create_info_struct(png);
   if (!info)
      return(1);

   if (setjmp(png_jmpbuf(png)))
      abort();

   png_init_io(png, fptr);

   // Output is 8bit depth, RGBA format.
   png_set_IHDR(
      png,
      info,
      width, height,
      8,
      PNG_COLOR_TYPE_RGBA,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT
   );
   png_write_info(png, info);

   int x,y;
   png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
   for (y=0;y<height;y++) 
      row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));

   for (y=0;y<height;y++) {
      png_byte *row = row_pointers[y];
      for (x=0;x<width;x++) {
         if (!flip)
            index = (height-1-y) * width + x;
         else
            index = y * width + x;
         png_byte *ptr = &(row[x*4]);
         ptr[0] = image[index].r;
         ptr[1] = image[index].g;
         ptr[2] = image[index].b;
         ptr[3] = image[index].a;
      }   
   }
   png_write_image(png, row_pointers);
   png_write_end(png, NULL);

   for (y=0;y<height;y++) 
      free(row_pointers[y]);
   free(row_pointers);

   return(0);
}
#endif

#ifdef ADDTIFF
int IsTIFF(char *fname)
{
   int i;
   char s[256];

   strcpy(s,fname);
   for (i=0;i<strlen(s);i++)
      if (s[i] >= 'A' && s[i] <= 'Z')
         s[i] += ('a' - 'A');

   if (strstr(s,".tif") != NULL)
      return(TRUE);
   if (strstr(s,".TIF") != NULL)
      return(TRUE);
   if (strstr(s,".tiff") != NULL)
      return(TRUE);
   if (strstr(s,".TIFF") != NULL)
      return(TRUE);

   return(FALSE);
}

/*
   Write a TIFF file
*/
int TIFF_Write(char *fname,BITMAP4 *image,int width,int height)
{
   TIFF *tptr = NULL;
   uint32 rowsperstrip = 0;
   uint32 row;
   int rows_to_write;

   if ((tptr = TIFFOpen(fname,"w")) == NULL)
      return(FALSE);

   TIFFSetField(tptr,TIFFTAG_IMAGEWIDTH,width);
   TIFFSetField(tptr,TIFFTAG_IMAGELENGTH,height);
   TIFFSetField(tptr,TIFFTAG_BITSPERSAMPLE,8);
   TIFFSetField(tptr,TIFFTAG_COMPRESSION,COMPRESSION_LZW);
   TIFFSetField(tptr,TIFFTAG_SAMPLESPERPIXEL,4);
   TIFFSetField(tptr,TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_RGB);
   TIFFSetField(tptr,TIFFTAG_ORIENTATION,ORIENTATION_TOPLEFT);
   TIFFSetField(tptr,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
   TIFFSetField(tptr,TIFFTAG_ORIENTATION,ORIENTATION_BOTLEFT);
   TIFFSetField(tptr,TIFFTAG_SOFTWARE,TIFFGetVersion());
   rowsperstrip = TIFFDefaultStripSize(tptr,rowsperstrip);
   TIFFSetField(tptr,TIFFTAG_ROWSPERSTRIP,rowsperstrip);

   for (row=0;row<height;row+=rowsperstrip) {
      if (row+rowsperstrip > height )
         rows_to_write = height - row;
      else
         rows_to_write = rowsperstrip;
      TIFFWriteEncodedStrip(tptr,row/rowsperstrip,&(image[row*width]),4*rows_to_write*width);
   }

   TIFFClose(tptr);

   return(TRUE);
}

/*
   Read a 16 bit TIFF image
*/
int TIFF_Read16(char *fname,COLOUR16 *image)
{
   int i,j;
   int index;
   TIFF *tptr;
   uint32 w,h,config,nsamples,sample=0;
   tdata_t raster;

   // Open file
   TIFFSetErrorHandler(NULL);
   TIFFSetWarningHandler(NULL);
   if ((tptr = TIFFOpen(fname,"r")) == NULL)
      return(FALSE);

   TIFFGetField(tptr,TIFFTAG_IMAGEWIDTH,&w);
   TIFFGetField(tptr,TIFFTAG_IMAGELENGTH,&h);
   TIFFGetField(tptr,TIFFTAG_PLANARCONFIG,&config);
   TIFFGetField(tptr,TIFFTAG_SAMPLESPERPIXEL,&nsamples);

   // Only support contiguous
   if (config != PLANARCONFIG_CONTIG) {
      TIFFClose(tptr);
      return(FALSE);
   }

   // Only support 1 and 3 samples per pixel
   if (nsamples != 1 && nsamples != 3) {
      TIFFClose(tptr);
      return(FALSE);
   }

   if ((raster = _TIFFmalloc(TIFFScanlineSize(tptr))) == NULL) {
      TIFFClose(tptr);
      return(FALSE);
   }

   for (j=0;j<h;j++) {
      TIFFReadScanline(tptr,raster,j,sample);
      for (i=0;i<w;i++) {
         index = (h-1-j) * w + i;
         if (nsamples == 1) {
            image[index].r = ((unsigned short *)raster)[i*nsamples+0];
            image[index].g = image[index].r;
            image[index].b = image[index].r;
         } else if (nsamples == 3) {
            image[index].r = ((unsigned short *)raster)[i*nsamples+0];
            image[index].g = ((unsigned short *)raster)[i*nsamples+1];
            image[index].b = ((unsigned short *)raster)[i*nsamples+2];
         }
      }
   }

   // Close
   _TIFFfree(raster);
   TIFFClose(tptr);

   return(TRUE);
}

int TIFF_Write16(char *fname,COLOUR16 *image,int width,int height)
{
   TIFF *tptr = NULL;
   uint32 row,rowsperstrip = 1;

   if ((tptr = TIFFOpen(fname,"w")) == NULL)
      return(FALSE);

   TIFFSetField(tptr,TIFFTAG_IMAGEWIDTH,width);
   TIFFSetField(tptr,TIFFTAG_IMAGELENGTH,height);
   TIFFSetField(tptr,TIFFTAG_BITSPERSAMPLE,16);
   TIFFSetField(tptr,TIFFTAG_COMPRESSION,COMPRESSION_NONE); // Fastest for big files
   TIFFSetField(tptr,TIFFTAG_SAMPLESPERPIXEL,3);
   TIFFSetField(tptr,TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_RGB);
   TIFFSetField(tptr,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
   //TIFFSetField(tptr,TIFFTAG_ORIENTATION,ORIENTATION_TOPLEFT);
   //TIFFSetField(tptr,TIFFTAG_ORIENTATION,ORIENTATION_BOTLEFT);
   TIFFSetField(tptr,TIFFTAG_SOFTWARE,TIFFGetVersion());
   //rowsperstrip = TIFFDefaultStripSize(tptr,rowsperstrip);
   TIFFSetField(tptr,TIFFTAG_ROWSPERSTRIP,rowsperstrip);

   for (row=0;row<height;row++) 
      TIFFWriteEncodedStrip(tptr,row,&(image[row*width]),3*2*width);

   TIFFClose(tptr);

   return(TRUE);
}

int TIFF_Write32(char *fname,COLOUR32 *image,int width,int height)
{  
   TIFF *tptr = NULL;
   uint32 rowsperstrip = 0;
   uint32 row;
   int rows_to_write;
   
   if ((tptr = TIFFOpen(fname,"w")) == NULL)
      return(FALSE);
   
   TIFFSetField(tptr,TIFFTAG_IMAGEWIDTH,width);
   TIFFSetField(tptr,TIFFTAG_IMAGELENGTH,height);
   TIFFSetField(tptr,TIFFTAG_BITSPERSAMPLE,32);
   TIFFSetField(tptr,TIFFTAG_COMPRESSION,COMPRESSION_NONE);
   TIFFSetField(tptr,TIFFTAG_SAMPLESPERPIXEL,3);
   TIFFSetField(tptr,TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_RGB);
   TIFFSetField(tptr,TIFFTAG_ORIENTATION,ORIENTATION_TOPLEFT);
   TIFFSetField(tptr,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
   TIFFSetField(tptr,TIFFTAG_ORIENTATION,ORIENTATION_BOTLEFT);
   TIFFSetField(tptr,TIFFTAG_SOFTWARE,TIFFGetVersion());
   rowsperstrip = TIFFDefaultStripSize(tptr,rowsperstrip);
   TIFFSetField(tptr,TIFFTAG_ROWSPERSTRIP,rowsperstrip);
   
   for (row=0;row<height;row+=rowsperstrip) {
      if (row+rowsperstrip > height )
         rows_to_write = height - row;
      else
         rows_to_write = rowsperstrip;
      TIFFWriteEncodedStrip(tptr,row/rowsperstrip,&(image[row*width]),3*4*rows_to_write*width);
   }
   
   TIFFClose(tptr);
   
   return(TRUE);
}

/*
   Get dimensions of a TIFF image
*/
int TIFF_Info(char *fname,int *width,int *height,int *depth,int *bits)
{
   uint32 w,h;
   uint16 d,s;
   TIFF *tptr;

   TIFFSetErrorHandler(NULL);
   TIFFSetWarningHandler(NULL);
   if ((tptr = TIFFOpen(fname,"r")) == NULL)
      return(FALSE);

   TIFFGetField(tptr,TIFFTAG_IMAGEWIDTH,&w);
   TIFFGetField(tptr,TIFFTAG_IMAGELENGTH,&h);
   TIFFGetField(tptr,TIFFTAG_BITSPERSAMPLE,&d);
   TIFFGetField(tptr,TIFFTAG_SAMPLESPERPIXEL,&s);
   
   *width  = w;
   *height = h;
   *depth  = d * s;
   *bits   = d;

   TIFFClose(tptr);
   return(TRUE);
}

/*
   Read a TIFF image
*/
int TIFF_Read(char *fname,BITMAP4 *image)
{
   int i,j;
   int index;
   TIFF *tptr;
   uint32 w,h;
   uint32* raster = NULL;

   // Open file
   if ((tptr = TIFFOpen(fname,"r")) == NULL)
      return(FALSE);

   TIFFGetField(tptr,TIFFTAG_IMAGEWIDTH,&w);
   TIFFGetField(tptr,TIFFTAG_IMAGELENGTH,&h);

   if ((raster = (uint32*)_TIFFmalloc(w*h*sizeof(uint32))) == NULL)
      return(FALSE);

   if (TIFFReadRGBAImage(tptr,w,h,raster,0) == 1) {
      for (j=0;j<h;j++) {
         for (i=0;i<w;i++) {
            index = j * w + i;
            image[index].r = (raster[index]) % 256;
            image[index].g = (raster[index] >> 8) % 256;
            image[index].b = (raster[index] >> 16) % 256;
            image[index].a = (raster[index] >> 24) % 256;
         }
      }
   } else {
      _TIFFfree(raster);
      return(FALSE);
   }

   // Close
   _TIFFfree(raster);
   TIFFClose(tptr);

   return(TRUE);
}
#endif

#ifdef ADDEXR
// Note alpha=1 is opaque, 0 is transparent
int IsEXR(char *fname)
{
   int i;
   char s[256];

   strcpy(s,fname);
   for (i=0;i<strlen(s);i++)
      if (s[i] >= 'A' && s[i] <= 'Z')
         s[i] += ('a' - 'A');

   if (strstr(s,".exr") != NULL)
      return(TRUE);
   if (strstr(s,".EXR") != NULL)
      return(TRUE);

   return(FALSE);
}

int EXR_Write(char *fname,ImfRgba *image,int width,int height)
{
   int i,j,index1,index2;
   ImfHeader *outheader = NULL;
   ImfOutputFile *outfile = NULL;
   ImfRgba p;

   // Flip
   for (j=0;j<height/2;j++) {
      for (i=0;i<width;i++) {
         index1 = j * width + i;
         index2 = (height-1-j) * width + i;
         p = image[index1];
         image[index1] = image[index2];
         image[index2] = p;
      }
   }

   outheader = ImfNewHeader();
   ImfHeaderSetDisplayWindow(outheader,0,0,width-1,height-1);
   ImfHeaderSetDataWindow(outheader,0,0,width-1,height-1);
   ImfHeaderSetScreenWindowWidth(outheader,(float)width);

   outfile = ImfOpenOutputFile(fname,outheader,IMF_WRITE_RGBA);
   ImfOutputSetFrameBuffer(outfile,image,1,width);
   ImfOutputWritePixels(outfile,height);
   ImfCloseOutputFile(outfile);

   return(TRUE);
}

int EXR_Info(char *fname,int *width,int *height)
{
   int xmin,xmax,ymin,ymax;
   ImfInputFile *infile;
   ImfHeader *inheader = NULL;

   if ((infile = ImfOpenInputFile(fname)) == NULL) 
      return(FALSE);
   
   inheader = (ImfHeader *)ImfInputHeader(infile);
   ImfHeaderDataWindow(inheader,&xmin,&ymin,&xmax,&ymax); // Get display or data window?
   *width = xmax - xmin + 1;
   *height = ymax - ymin + 1;
   ImfCloseInputFile(infile);

   return(TRUE);
}

int EXR_Read(char *fname,ImfRgba *image,int *width,int *height)
{
   int xmin,xmax,ymin,ymax;
   int i,j,index1,index2;
   ImfRgba p;
   ImfInputFile *infile; 
   ImfHeader *inheader = NULL;
   
   if ((infile = ImfOpenInputFile(fname)) == NULL)
      return(FALSE);
   
   inheader = (ImfHeader *)ImfInputHeader(infile);
   ImfHeaderDataWindow(inheader,&xmin,&ymin,&xmax,&ymax); // Get display or data window?
   *width = xmax - xmin + 1;
   *height = ymax - ymin + 1;
   
   ImfInputSetFrameBuffer(infile,image - xmin - ymin*(*width),1,*width);
   ImfInputReadPixels(infile,ymin,ymax);
   ImfCloseInputFile(infile);

   // Flip
   for (j=0;j<(*height)/2;j++) {
      for (i=0;i<(*width);i++) {
         index1 = j * (*width) + i;
         index2 = ((*height)-1-j) * (*width) + i;
         p = image[index1];
         image[index1] = image[index2];
         image[index2] = p;
      }
   }

   return(TRUE);
}

void EXR_Erase(ImfRgba *image,int width,int height,float r,float g,float b,float a)
{
   int i,j,index;

   for (i=0;i<width;i++) {
      for (j=0;j<height;j++) {
         index = j * width + i;
         ImfFloatToHalf(r,&(image[index].r));
         ImfFloatToHalf(g,&(image[index].g));
         ImfFloatToHalf(b,&(image[index].b));
         ImfFloatToHalf(a,&(image[index].a));
      }
   }
}

#endif

