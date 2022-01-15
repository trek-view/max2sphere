#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Choose formats with external support required
#define ADDJPEG 
//#define ADDPNG 
//#define ADDTIFF 
//#define ADDEXR

#ifdef ADDJPEG
#include <jpeglib.h>
#endif
#ifdef ADDPNG
#include <png.h>
#endif
#ifdef ADDTIFF
#include <tiffio.h>
#endif
#ifdef ADDEXR
#include <OpenEXR/ImfCRgbaFile.h>
#endif

#define TGA    0
#define BMP    1
#define PNG    2
#define JPG    3
#define JPEG   3
#define ATIF   4
#define ATIFF  4
#define ATIF16 40
#define RAW16  5
#define PPM    6
#define EXR    7

#define DTOR (M_PI/180)
#define RTOD (180/M_PI)
#define TWOPI (2*M_PI)
#define PID2 (0.5*M_PI)
#define PI (M_PI)

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#define ABS(x) (x < 0 ? -(x) : (x))
#ifndef MIN
#define MIN(x,y) (x < y ? x : y)
#endif
#ifndef MAX
#define MAX(x,y) (x > y ? x : y)
#endif

typedef struct {
	unsigned char r,g,b,a;
} BITMAP4;

typedef struct {
   unsigned char r,g,b;
} BITMAP3;

typedef struct {
	unsigned short r,g,b;
} COLOUR16;

typedef struct {
   unsigned int r,g,b;
} COLOUR32;

/* 18 bytes long */
typedef struct {
   unsigned char  idlength;
   char  colourmaptype;
   char  datatypecode;
   short int colourmaporigin;
   short int colourmaplength;
   char  colourmapdepth;
   short int x_origin;
   short int y_origin;
   unsigned short int width;
   unsigned short int height;
   char  bitsperpixel;
   char  imagedescriptor;
} TGAHEADER;

// *** for BMP
typedef struct {
   unsigned short int type;                 /* Magic identifier            */
   unsigned int size;                       /* File size in bytes          */
   unsigned short int reserved1, reserved2;
   unsigned int offset;                     /* Offset to image data, bytes */
} HEADER;
typedef struct {
   unsigned int size;               /* Header size in bytes      */
   int width,height;                /* Width and height of image */
   unsigned short int planes;       /* Number of colour planes   */
   unsigned short int bits;         /* Bits per pixel            */
   unsigned int compression;        /* Compression type          */
   unsigned int imagesize;          /* Image size in bytes       */
   int xresolution,yresolution;     /* Pixels per meter          */
   unsigned int ncolours;           /* Number of colours         */
   unsigned int importantcolours;   /* Important colours         */
} INFOHEADER;
typedef struct {
   unsigned char r,g,b,junk;
} COLOURINDEX;
// *** end for BMP

BITMAP4 *Create_Bitmap(int,int);
void Destroy_Bitmap(BITMAP4 *);
void Write_Bitmap(FILE *,BITMAP4 *,int,int,int);
void Erase_Bitmap(BITMAP4 *,int,int,BITMAP4);
void GaussianScale(BITMAP4 *,int,int,BITMAP4 *,int,int,double);
void BiCubicScale(BITMAP4 *,int,int,BITMAP4 *,int,int);
double BiCubicR(double);
int Draw_Marker(BITMAP4 *,int,int,int,int,BITMAP4,int,int);
int Draw_Pixel(BITMAP4 *,int,int,int,int,BITMAP4);
int Draw_Box(BITMAP4 *,int,int,int,int,int,int,BITMAP4);
BITMAP4 Get_Pixel(BITMAP4 *,int,int,int,int);
void Draw_Line(BITMAP4 *,int,int,int,int,int,int,BITMAP4);
void Draw_ModLine(BITMAP4 *,int,int,int,int,int,int,BITMAP4,int);
BITMAP4 Scale_Pixel(BITMAP4,double);
void Flip_Bitmap(BITMAP4 *,int,int,int);
int Same_BitmapPixel(BITMAP4,BITMAP4);
BITMAP4 YUV_to_Bitmap(int,int,int);

void BM_WriteLongInt(FILE *,char *,long);
void BM_WriteHexString(FILE *,char *);

int IsTGA(char *);
void TGA_Info(FILE *,int *,int *,int *);
int TGA_Read(FILE *,BITMAP4 *,int *,int *);
void TGA_MergeBytes(BITMAP4 *,unsigned char *,int);
void WriteTGACompressedRow(FILE *,BITMAP4 *,int,int);

int BMP_Info(FILE *,int *,int *,int *);
int BMP_Read(FILE *,BITMAP4 *, int *, int *);

int IsPPM(char *);
int PPM_Info(FILE *,int *,int *,int *);
int PPM_Read(FILE *,COLOUR16 *,int *,int *,int *);
int PPM_Write(FILE *,COLOUR16 *,int,int,int);

int IsRAW(char *);
int RAW_Read(FILE *,COLOUR16 *,int,int,int);
int RAW_Write(FILE *,COLOUR16 *,int,int);

int Read_UShort(FILE *,unsigned short *,int);
int Write_UShort(FILE *,unsigned short,int);
int Read_UInt(FILE *,unsigned int *,int);

#ifdef ADDJPEG
int IsJPEG(char *);
int JPEG_Write(FILE *,BITMAP4 *,int,int,int);
int JPEG_Info(FILE *,int *,int *,int *);
int JPEG_Read(FILE *,BITMAP4 *,int *,int *);
#endif

#ifdef ADDPNG
int IsPNG(char *);
int PNG_Write(FILE *fptr,BITMAP4 *,int,int,int);
int PNG_Info(FILE *,int *,int *,int *);
int PNG_Read(FILE *,BITMAP4 *,int *,int *);
#endif

#ifdef ADDTIFF
int IsTIFF(char *);
int TIFF_Write(char *,BITMAP4 *,int,int);
int TIFF_Read16(char *,COLOUR16 *);
int TIFF_Write16(char *,COLOUR16 *,int,int);
int TIFF_Write32(char *,COLOUR32 *,int,int);
int TIFF_Info(char *,int *,int *,int *,int *);
int TIFF_Read(char *,BITMAP4 *);
#endif

#ifdef ADDEXR
int IsEXR(char *);
int EXR_Write(char *,ImfRgba *,int,int);
int EXR_Info(char *,int *,int *);
int EXR_Read(char *,ImfRgba *,int *,int *);
void EXR_Erase(ImfRgba *,int,int,float,float,float,float);
#endif

