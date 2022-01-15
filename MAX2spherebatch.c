#include "MAX2spherebatch.h"

/*
	Convert a sequence of pairs of frames from the GoPro Max camera to an equirectangular
	Sept 08: First version based upon cube2sphere
	Sept 10: Added output file mask
   Dec  14: This version which is a batch converter based upon a lookup table
*/

PARAMS params;

// These are known frame templates
// The appropriate one to use will be auto detected, error is none match
#define NTEMPLATE 2
FRAMESPECS template[NTEMPLATE] = {{4096,1344,1376,1344,32,5376},{2272,736,768,736,16,2944}};
int whichtemplate = -1;    // Which frame template do we thnk we have

// Lookup table
typedef struct {
   UV uv;
   short int face;
} LLTABLE;
LLTABLE *lltable = NULL;
int ntable = 0;
int itable = 0;

int main(int argc,char **argv)
{
   int i,j,index,face,aj,ai,nframe,n=0;
   char fname1[256],fname2[256],tablename[256];
   double x,y,dx,dy,x0,y0,longitude,latitude;
	UV uv;
   COLOUR16 csum,czero = {0,0,0};
	BITMAP4 c,black = {0,0,0};;
   double starttime;
	FILE *fptr;

   // Memory for images, 2 input frames and one output equirectangular
   BITMAP4 *frame1 = NULL,*frame2 = NULL,*spherical = NULL;

   // Default settings
   Init();

   // Check and parse command line
   if (argc < 2) 
      GiveUsage(argv[0]);
   for (i=1;i<argc-1;i++) {
      if (strcmp(argv[i],"-w") == 0) {
         params.outwidth = atoi(argv[i+1]);
			params.outwidth = 4 * (params.outwidth / 4);    // Make factor of 4
			params.outheight = params.outwidth / 2;         // Will be even
      } else if (strcmp(argv[i],"-a") == 0) {
         params.antialias = MAX(1,atoi(argv[i+1]));
			params.antialias2 = params.antialias * params.antialias;
      } else if (strcmp(argv[i],"-o") == 0) {
			strcpy(params.outfilename,argv[i+1]);
      } else if (strcmp(argv[i],"-n") == 0) {
         params.nstart = atoi(argv[i+1]);
      } else if (strcmp(argv[i],"-m") == 0) {
         params.nstop = atoi(argv[i+1]);
      } else if (strcmp(argv[i],"-d") == 0) {
         params.debug = TRUE;
      }
   }

	// Check filename templates
	if (!CheckTemplate(argv[argc-1],2))              // Fatal
		exit(-1);
	if (strlen(params.outfilename) > 2) {
		if (!CheckTemplate(params.outfilename,1))     // Delete user selected output filename template
			params.outfilename[0] = '\0';
	}

   // Check the first frame to determine template and frame sizes
	sprintf(fname1,argv[argc-1],0,params.nstart);
	sprintf(fname2,argv[argc-1],5,params.nstart);
	if ((whichtemplate = CheckFrames(fname1,fname2,&params.framewidth,&params.frameheight)) < 0)
		exit(-1);
   if (params.debug) {
      fprintf(stderr,"%s() - frame dimensions: %d x %d\n",argv[0],params.framewidth,params.frameheight);
		fprintf(stderr,"%s() - Expect frame template %d\n",argv[0],whichtemplate+1);
	}

   // Malloc images
	frame1 = Create_Bitmap(params.framewidth,params.frameheight);
	frame2 = Create_Bitmap(params.framewidth,params.frameheight);
	if (params.outwidth < 0) {
		params.outwidth = template[whichtemplate].equiwidth;
   	params.outheight = params.outwidth / 2;
	}
	spherical = Create_Bitmap(params.outwidth,params.outheight);
   if (frame1 == NULL || frame2 == NULL || spherical == NULL) {
      fprintf(stderr,"%s() - Failed to malloc memory for the images\n",argv[0]);
      exit(-1);
   }

   // Does a table exist? If it does load it, if not create it and save it
	ntable = params.outheight * params.outwidth * params.antialias * params.antialias;
	lltable = malloc(ntable*sizeof(LLTABLE));
	sprintf(tablename,"%d_%d_%d_%d.data",whichtemplate,params.outwidth,params.outheight,params.antialias);
	if ((fptr = fopen(tablename,"r")) != NULL) {
		if (params.debug)
			fprintf(stderr,"%s() - Reading lookup table\n",argv[0]);
		if ((n = fread(lltable,sizeof(LLTABLE),ntable,fptr)) != ntable) {
			fprintf(stderr,"%s() - Failed to read lookup table \"%s\" (%d != %d)\n",argv[0],tablename,n,ntable);
		}
		fclose(fptr);
	}
	if (n != ntable) {
		if (params.debug)
         fprintf(stderr,"%s() - Generating lookup table\n",argv[0]);
   	dx = params.antialias * params.outwidth;
   	dy = params.antialias * params.outheight;
		itable = 0;
   	for (j=0;j<params.outheight;j++) {
   	   y0 = j / (double)params.outheight;
   	   for (i=0;i<params.outwidth;i++) {
   	      x0 = i / (double)params.outwidth;
   	      for (aj=0;aj<params.antialias;aj++) {
   	         y = y0 + aj / dy; // 0 ... 1
   	         for (ai=0;ai<params.antialias;ai++) {
   	            x = x0 + ai / dx; // 0 ... 1
   	            longitude = x * TWOPI - M_PI;    // -pi ... pi
   	            latitude = y * M_PI - M_PI/2;    // -pi/2 ... pi/2
						lltable[itable].face = FindFaceUV(longitude,latitude,&(lltable[itable].uv));
						itable++;
					}
				}
			}
		}
      if (params.debug)
         fprintf(stderr,"%s() - Saving lookup table\n",argv[0]);
		fptr = fopen(tablename,"w");
		fwrite(lltable,ntable,sizeof(LLTABLE),fptr);
		fclose(fptr);
	}

   // Process each frame of the sequence
	for (nframe=params.nstart;nframe<=params.nstop;nframe++) {

		// Form the spherical map
      if (params.debug)
         fprintf(stderr,"%s() - Creating spherical map for frame %d\n",argv[0],nframe);
		Erase_Bitmap(spherical,params.outwidth,params.outheight,black);

	   // Read both frames
	   sprintf(fname1,argv[argc-1],0,nframe);
	   sprintf(fname2,argv[argc-1],5,nframe);
	   if (!ReadFrame(frame1,fname1,params.framewidth,params.frameheight))
	      exit(-1);
	   if (!ReadFrame(frame2,fname2,params.framewidth,params.frameheight))
	      exit(-1);

	   starttime = GetRunTime(); 
		itable = 0;
	   for (j=0;j<params.outheight;j++) {
	      //y0 = j / (double)params.outheight;
	      //if (params.debug && j % (params.outheight/32) == 0)
	        //fprintf(stderr,"%s() - Scan line %d\n",argv[0],j);
	
	      for (i=0;i<params.outwidth;i++) {
	         //x0 = i / (double)params.outwidth;
	         csum = czero; // Supersampling antialising sum
	
				// Antialiasing loops
	         for (aj=0;aj<params.antialias;aj++) {
	            //y = y0 + aj / dy; // 0 ... 1
	
	         	// Antialiasing loops
	         	for (ai=0;ai<params.antialias;ai++) {
	            	//x = x0 + ai / dx; // 0 ... 1
	
	               // Calculate latitude and longitude
	               //longitude = x * TWOPI - M_PI;    // -pi ... pi
						//latitude = y * M_PI - M_PI/2;    // -pi/2 ... pi/2
	               face = lltable[itable].face;
	               uv = lltable[itable].uv;
						itable++;
	
	               // Sum over the supersampling set 
						c = GetColour(face,uv,frame1,frame2);
						csum.r += c.r;
   	            csum.g += c.g;
   	            csum.b += c.b;
   	         }
   	      }
	
	         // Finally update the spherical image
	         index = j * params.outwidth + i; 
	         spherical[index].r = csum.r / params.antialias2;
	         spherical[index].g = csum.g / params.antialias2;
	         spherical[index].b = csum.b / params.antialias2;
	
	      }
	   }
   	if (params.debug)
      	fprintf(stderr,"%s() - Processing time: %g seconds\n",argv[0],GetRunTime()-starttime);

   	// Write out the equirectangular
		// Base the name on the name of the first frame
   	if (params.debug)
   	   fprintf(stderr,"%s() - Saving equirectangular\n",argv[0]);
   	WriteSpherical(fname1,nframe,spherical,params.outwidth,params.outheight);
	}

   exit(0);
}

/*
	Check the frames
	- do they exist
	- are they jpeg
	- are they the same size
	- determine which frame template we are using
*/
int CheckFrames(char *fname1,char *fname2,int *width,int *height)
{
	int i,n=-1;
	int w1,h1,w2,h2,depth;
	FILE *fptr;

   if (!IsJPEG(fname1) || !IsJPEG(fname2)) {
      fprintf(stderr,"CheckFrames() - frame name does not look like a jpeg file\n");
      return(-1);
   }

   // Frame 1
   if ((fptr = fopen(fname1,"rb")) == NULL) {
      fprintf(stderr,"CheckFrames() - Failed to open first frame \"%s\"\n",fname1);
      return(-1);
   }
   JPEG_Info(fptr,&w1,&h1,&depth);
   fclose(fptr);

	// Frame 2
   if ((fptr = fopen(fname2,"rb")) == NULL) {
      fprintf(stderr,"CheckFrames() - Failed to open second frame \"%s\"\n",fname2);
      return(-1);
   }
   JPEG_Info(fptr,&w2,&h2,&depth);
   fclose(fptr);

	// Are they the same size
   if (w1 != w2 || h1 != h2) {
      fprintf(stderr,"CheckFrames() - Frame sizes don't match, %d != %d or %d != %d\n",w1,h1,w2,h2);
      return(-1);
   }
	
	// Is it a known template?
	for (i=0;i<NTEMPLATE;i++) {
		if (w1 == template[i].width && h1 == template[i].height) {
			n = i;
			break;
		}
	}
	if (n < 0) {
		fprintf(stderr,"CheckFrames() - No recognised frame template\n");
		return(-1);
	}

	*width = w1;
	*height = h1;

	return(n);
}

/*
   Write spherical image
	The file name is either using the mask params.outfilename which should have a %d for the frame number
	or based upon the basename provided which will have two %d locations for track and framenumber
*/
int WriteSpherical(char *basename,int nframe,BITMAP4 *img,int w,int h)
{
	int i;
   FILE *fptr;
	char fname[256];

	// Create the output file name
	if (strlen(params.outfilename) < 2) {
		sprintf(fname,basename,0,nframe);
		for (i=strlen(fname)-1;i>0;i--) {
			if (fname[i] == '.') {
				fname[i] = '\0';
				break;
			}
		}
		strcat(fname,"_sphere.jpg");
	} else {
		sprintf(fname,params.outfilename,nframe);
	}

	if (params.debug)
		fprintf(stderr,"WriteSpherical() - Saving file \"%s\"\n",fname);

	// Save
   if ((fptr = fopen(fname,"wb")) == NULL) {
      fprintf(stderr,"WriteSpherical() - Failed to open output file \"%s\"\n",fname);
      return(FALSE);
   }
   JPEG_Write(fptr,img,w,h,100);
   fclose(fptr);

   return(TRUE);
}

/*
   Read a frame
*/
int ReadFrame(BITMAP4 *img,char *fname,int w,int h)
{
   FILE *fptr;

   if (params.debug)
      fprintf(stderr,"ReadFrame() - Reading image \"%s\"\n",fname);

   // Attempt to open file
   if ((fptr = fopen(fname,"rb")) == NULL) {
      fprintf(stderr,"ReadFrame() - Failed to open \"%s\"\n",fname);
      return(FALSE);
   }

   // Read image data
   if (JPEG_Read(fptr,img,&w,&h) != 0) { 
      fprintf(stderr,"ReadFrame() - Failed to correctly read JPG file \"%s\"\n",fname);
		return(FALSE);
   }
   fclose(fptr);

   return(TRUE);
}

/*
   Given longitude and latitude find corresponding face id and (u,v) coordinate on the face
   Return -1 if something went wrong, shouldn't
*/
int FindFaceUV(double longitude,double latitude,UV *uv)
{
   int k,found = -1;
   double mu,denom,coslatitude,fourdivpi;
   UV fuv;
   XYZ p,q;

	fourdivpi = 4.0 / M_PI;

   // p is the ray from the camera position into the scene
	coslatitude = cos(latitude);
   p.x = coslatitude * sin(longitude);
   p.y = coslatitude * cos(longitude);
   p.z = sin(latitude);

   // Find which face the vector intersects
   for (k=0;k<6;k++) {
      denom = -(params.faces[k].a * p.x + params.faces[k].b * p.y + params.faces[k].c * p.z);

      // Is p parallel to face? Shouldn't ever happen.
      //if (ABS(denom) < 0.000001)
      //   continue;

      // Find position q along ray and ignore intersections on the back pointing ray?
      if ((mu = params.faces[k].d / denom) < 0)
         continue;
      q.x = mu * p.x;
      q.y = mu * p.y;
      q.z = mu * p.z;

      // Find out which face it is on
      switch (k) {
      case LEFT:
      case RIGHT:
         if (q.y <= 1 && q.y >= -1 && q.z <= 1 && q.z >= -1)
            found = k;
         q.y = atan(q.y) * fourdivpi;
         q.z = atan(q.z) * fourdivpi;
         break;
      case FRONT:
      case BACK:
         if (q.x <= 1 && q.x >= -1 && q.z <= 1 && q.z >= -1)
            found = k;
         q.x = atan(q.x) * fourdivpi;
         q.z = atan(q.z) * fourdivpi;
         break;
      case TOP:
      case DOWN:
         if (q.x <= 1 && q.x >= -1 && q.y <= 1 && q.y >= -1)
            found = k;
         q.x = atan(q.x) * fourdivpi;
         q.y = atan(q.y) * fourdivpi;
         break;
      }
      if (found >= 0)
          break;
   }
   if (found < 0 || found > 5) {
      fprintf(stderr,"FindFaceUV() - Didn't find an intersecting face, shouldn't happen!\n");
      return(-1);
   }

   // Determine the u,v coordinate
   switch (found) {
   case LEFT:
       fuv.u = q.y + 1;
       fuv.v = q.z + 1;
       break;
   case RIGHT:
       fuv.u = 1 - q.y;
       fuv.v = q.z + 1;
       break;
   case FRONT:
       fuv.u = q.x + 1;
       fuv.v = q.z + 1;
       break;
   case BACK:
       fuv.u = 1 - q.x;
       fuv.v = q.z + 1;
       break;
   case DOWN:
		 fuv.u = 1 - q.x;
       fuv.v = 1 - q.y;
       break;
   case TOP:
       fuv.u = 1 - q.x;
       fuv.v = q.y + 1;
       break;
   }
	fuv.u *= 0.5;
	fuv.v *= 0.5;

	// Need to understand this at some stage
	if (fuv.u >= 1)
		fuv.u = NEARLYONE;
   if (fuv.v >= 1)
      fuv.v = NEARLYONE;

   if (fuv.u < 0 || fuv.v < 0 || fuv.u >= 1 || fuv.v >= 1) {
      fprintf(stderr,"FindFaceUV() - Illegal (u,v) coordinate (%g,%g) on face %d\n",fuv.u,fuv.v,found);
      return(-1);
   }

   *uv = fuv;

   return(found);
}

/*
	Given a face and a (u,v) in that face, determine colour from the two frames
	This is largely a mapping excercise from (u,v) of each face to the two frames
	For faces left, right, down and top a blend is required between the two halves
	Relies on the values from the frame template
*/
BITMAP4 GetColour(int face,UV uv,BITMAP4 *frame1,BITMAP4 *frame2)
{
	int ix,iy,index;
	int x0,w;
	double alpha,duv;
	UV uvleft,uvright;
	BITMAP4 c = {0,0,0},c1,c2;

	// Rotate u,v counterclockwise by 90 degrees for lower frame
	if (face == DOWN || face == BACK || face == TOP)
		RotateUV90(&uv);

	// v doesn't change
	uvleft.v = uv.v;
	uvright.v = uv.v;

	switch (face) {
	// Frame 1
	case FRONT:
	case BACK:
		x0 = template[whichtemplate].sidewidth;
		w = template[whichtemplate].centerwidth;
		ix = x0 + uv.u * w;
		iy = uv.v * template[whichtemplate].height;
		index = iy * template[whichtemplate].width + ix;
		c = (face == FRONT) ? frame1[index] : frame2[index];
		break;
   case LEFT:
	case DOWN:
		w = template[whichtemplate].sidewidth;
		duv = template[whichtemplate].blendwidth / (double)w;
		uvleft.u = 2 * (0.5 - duv) * uv.u;
		uvright.u = 2 * (0.5 - duv) * (uv.u - 0.5) + 0.5 + duv;
		if (uvleft.u <= 0.5 - 2*duv) {
      	ix = uvleft.u * w;
      	iy = uvleft.v * template[whichtemplate].height;
      	index = iy * template[whichtemplate].width + ix;
			c = (face == LEFT) ? frame1[index] : frame2[index];
		} else if (uvright.u >= 0.5 + 2*duv) {
         ix = uvright.u * w;
         iy = uvright.v * template[whichtemplate].height; 
         index = iy * template[whichtemplate].width + ix;
			c = (face == LEFT) ? frame1[index] : frame2[index];
		} else {
         ix = uvleft.u * w;
         iy = uvleft.v * template[whichtemplate].height;
			index = iy * template[whichtemplate].width + ix;
			c1 = (face == LEFT) ? frame1[index] : frame2[index];
         ix = uvright.u * w;
         iy = uvright.v * template[whichtemplate].height;
         index = iy * template[whichtemplate].width + ix;
         c2 = (face == LEFT) ? frame1[index] : frame2[index];
			alpha = (uvleft.u - 0.5 + 2 * duv) / (2 * duv);
			c = ColourBlend(c1,c2,alpha);
		}
      break;
   case RIGHT:
	case TOP:
		x0 = template[whichtemplate].sidewidth + template[whichtemplate].centerwidth;
		w = template[whichtemplate].sidewidth;
		duv = template[whichtemplate].blendwidth / (double)w;
      uvleft.u = 2 * (0.5 - duv) * uv.u;
      uvright.u = 2 * (0.5 - duv) * (uv.u - 0.5) + 0.5 + duv;
		if (uvleft.u <= 0.5 - 2*duv) {
      	ix = x0 + uvleft.u * w;
      	iy = uv.v * template[whichtemplate].height;
      	index = iy * template[whichtemplate].width + ix;
     		c = (face == RIGHT) ? frame1[index] : frame2[index]; 
		} else if (uvright.u >= 0.5 + 2*duv) {
         ix = x0 + uvright.u * w;
         iy = uvright.v * template[whichtemplate].height;
         index = iy * template[whichtemplate].width + ix;
			c = (face == RIGHT) ? frame1[index] : frame2[index]; 
      } else {
         ix = x0 + uvleft.u * w;
         iy = uvleft.v * template[whichtemplate].height;
         index = iy * template[whichtemplate].width + ix;
         c1 = (face == RIGHT) ? frame1[index] : frame2[index];
         ix = x0 + uvright.u * w;
         iy = uvright.v * template[whichtemplate].height;
         index = iy * template[whichtemplate].width + ix;
         c2 = (face == RIGHT) ? frame1[index] : frame2[index];
         alpha = (uvleft.u - 0.5 + 2 * duv) / (2 * duv);
       	c = ColourBlend(c1,c2,alpha);
      }
      break;
	}

	return(c);
}

/*
	Blend two colours
*/
BITMAP4 ColourBlend(BITMAP4 c1,BITMAP4 c2,double alpha)
{
	double m1;
	BITMAP4 c;

	m1 = 1 - alpha;
	c.r = m1 * c1.r + alpha * c2.r;
   c.g = m1 * c1.g + alpha * c2.g;
   c.b = m1 * c1.b + alpha * c2.b;

	return(c);
}

/*
	Rotate a uv by 90 degrees counterclockwise
*/
void RotateUV90(UV *uv)
{
	UV tmp;

	tmp = *uv;
	uv->u = tmp.v;
	uv->v = NEARLYONE - tmp.u;
}

/*
	Initialise parameters structure
*/
void Init(void)
{
   params.outwidth = -1;
   params.outheight = -1;
   params.framewidth = -1;
   params.frameheight = -1;
   params.antialias = 2;
	params.antialias2 = 4; // antialias squared
	params.nstart = 0;
	params.nstop = 100000;
	params.outfilename[0] = '\0';
   params.debug = FALSE;

   // Parameters for the 6 cube planes, ax + by + cz + d = 0
   params.faces[LEFT].a   = -1;
   params.faces[LEFT].b   =  0;
   params.faces[LEFT].c   =  0;
   params.faces[LEFT].d   = -1;

   params.faces[RIGHT].a  =  1;
   params.faces[RIGHT].b  =  0;
   params.faces[RIGHT].c  =  0;
   params.faces[RIGHT].d  = -1;

   params.faces[TOP].a    =  0;
   params.faces[TOP].b    =  0;
   params.faces[TOP].c    =  1;
   params.faces[TOP].d    = -1;

   params.faces[DOWN].a   =  0;
   params.faces[DOWN].b   =  0;
   params.faces[DOWN].c   = -1;
   params.faces[DOWN].d   = -1;

   params.faces[FRONT].a  =  0;
   params.faces[FRONT].b  =  1;
   params.faces[FRONT].c  =  0;
   params.faces[FRONT].d  = -1;

   params.faces[BACK].a   =  0;
   params.faces[BACK].b   = -1;
   params.faces[BACK].c   =  0;
   params.faces[BACK].d   = -1;
}

/*
   Time scale at microsecond resolution but returned as seconds
	OS dependent, an alternative will need to be found for non UNIX systems
*/
double GetRunTime(void)
{
   double sec = 0;
   struct timeval tp;

   gettimeofday(&tp,NULL);
   sec = tp.tv_sec + tp.tv_usec / 1000000.0;

   return(sec);
}

/*
	Check that the filename template has the correct number of %d entries
*/
int CheckTemplate(char *s,int nexpect)
{
	int i,n=0;

	for (i=0;i<strlen(s);i++) {
		if (s[i] == '%')
			n++;
	}

	if (n != nexpect) {
		fprintf(stderr,"This filename template \"%s\" does not look like it contains sufficient %%d entries\n",s);
		fprintf(stderr,"Expect %d but found %d\n",nexpect,n);
		return(FALSE);
	}

	return(TRUE);
}

/*
   Standard usage string
*/
void GiveUsage(char *s)
{
   fprintf(stderr,"Usage: %s [options] sequencetemplate\n",s);
	fprintf(stderr,"\n");
	fprintf(stderr,"The sequence filename template should contain two %%d entries. The first will be populated\n");
	fprintf(stderr,"with the track number 0 or 5, the second is the frame sequence number, see -n and -m below.\n");
	fprintf(stderr,"So for example, if there are 1000 frames called track0_frame0001.jpg, track5_0001.jpg, ...\n");
	fprintf(stderr,"then the program might be called as follows:\n");
	fprintf(stderr,"   %s -w 4096 -n 1 -m 1000 track%%d_frame%%04d.jpg\n",s);
	fprintf(stderr,"Or if directories are used with frames track0/frame1.jpg, track5/1000.jpg, ...\n");
   fprintf(stderr,"   %s -w 4096 -n 1 -m 1000 track%%d/frame%%4d.jpg\n",s);
   fprintf(stderr,"\n");
   fprintf(stderr,"Options\n");
   fprintf(stderr,"   -w n      Sets the output image width, default: %d\n",params.outwidth);
   fprintf(stderr,"   -a n      Sets antialiasing level, default = %d\n",params.antialias);
	fprintf(stderr,"   -o s      Specify the output filename template, default is based on track 0 name uses track 2\n");
   fprintf(stderr,"             If specified then it should contain one %%d field for the frame number\n");
	fprintf(stderr,"   -n n      Start index for the sequence, default: %d\n",params.nstart);
   fprintf(stderr,"   -m n      End index for the sequence, default: %d\n",params.nstop);
   fprintf(stderr,"   -d        Enable debug mode, default: off\n");
   exit(-1);
}



