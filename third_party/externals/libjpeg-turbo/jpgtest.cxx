/* Copyright (C)2004 Landmark Graphics Corporation
 * Copyright (C)2005, 2006 Sun Microsystems, Inc.
 * Copyright (C)2009 D. R. Commander
 *
 * This library is free software and may be redistributed and/or modified under
 * the terms of the wxWindows Library License, Version 3.1 or (at your option)
 * any later version.  The full license is in the LICENSE.txt file included
 * with this distribution.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * wxWindows Library License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "./bmp.h"
#include "./rrutil.h"
#include "./rrtimer.h"
#include "./turbojpeg.h"

#define _catch(f) {if((f)==-1) {printf("Error in %s:\n%s\n", #f, tjGetErrorStr());  goto bailout;}}

int forcemmx=0, forcesse=0, forcesse2=0, forcesse3=0, fastupsample=0;
const int _ps[BMPPIXELFORMATS]={3, 4, 3, 4, 4, 4};
const int _flags[BMPPIXELFORMATS]={0, 0, TJ_BGR, TJ_BGR,
	TJ_BGR|TJ_ALPHAFIRST, TJ_ALPHAFIRST};
const int _rindex[BMPPIXELFORMATS]={0, 0, 2, 2, 3, 1};
const int _gindex[BMPPIXELFORMATS]={1, 1, 1, 1, 2, 2};
const int _bindex[BMPPIXELFORMATS]={2, 2, 0, 0, 1, 3};
const char *_pfname[]={"RGB", "RGBA", "BGR", "BGRA", "ABGR", "ARGB"};
const char *_subnamel[NUMSUBOPT]={"4:4:4", "4:2:2", "4:2:0", "GRAY"};
const char *_subnames[NUMSUBOPT]={"444", "422", "420", "GRAY"};

void printsigfig(double val, int figs)
{
	char format[80];
	double _l=log10(val);  int l;
	if(_l<0.)
	{
		l=(int)fabs(_l);
		sprintf(format, "%%%d.%df", figs+l+2, figs+l);
	}
	else
	{
		l=(int)_l+1;
		if(figs<=l) sprintf(format, "%%.0f");
		else sprintf(format, "%%%d.%df", figs+1, figs-l);
	}	
	printf(format, val);
}

void dotest(unsigned char *srcbuf, int w, int h, BMPPIXELFORMAT pf, int bu,
	int jpegsub, int qual, char *filename, int dotile, int useppm, int quiet)
{
	char tempstr[1024];
	FILE *outfile;  tjhandle hnd;
	unsigned char **jpegbuf=NULL, *rgbbuf=NULL;
	rrtimer timer; double elapsed;
	int jpgbufsize=0, i, j, tilesizex, tilesizey, numtilesx, numtilesy, ITER;
	unsigned long *comptilesize=NULL;
	int flags=(forcemmx?TJ_FORCEMMX:0)|(forcesse?TJ_FORCESSE:0)
		|(forcesse2?TJ_FORCESSE2:0)|(forcesse3?TJ_FORCESSE3:0)
		|(fastupsample?TJ_FASTUPSAMPLE:0);
	int ps=_ps[pf];
	int pitch=w*ps;

	flags |= _flags[pf];
	if(bu) flags |= TJ_BOTTOMUP;

	if((rgbbuf=(unsigned char *)malloc(pitch*h)) == NULL)
	{
		puts("ERROR: Could not allocate image buffer.");
		exit(1);
	}

	if(!quiet) printf("\n>>>>>  %s (%s) <--> JPEG %s Q%d  <<<<<\n", _pfname[pf],
		bu?"Bottom-up":"Top-down", _subnamel[jpegsub], qual);
	if(dotile) {tilesizex=tilesizey=4;}  else {tilesizex=w;  tilesizey=h;}

	do
	{
		tilesizex*=2;  if(tilesizex>w) tilesizex=w;
		tilesizey*=2;  if(tilesizey>h) tilesizey=h;
		numtilesx=(w+tilesizex-1)/tilesizex;
		numtilesy=(h+tilesizey-1)/tilesizey;
		if((comptilesize=(unsigned long *)malloc(sizeof(unsigned long)*numtilesx*numtilesy)) == NULL
		|| (jpegbuf=(unsigned char **)malloc(sizeof(unsigned char *)*numtilesx*numtilesy)) == NULL)
		{
			puts("ERROR: Could not allocate image buffers.");
			goto bailout;
		}
		memset(jpegbuf, 0, sizeof(unsigned char *)*numtilesx*numtilesy);
		for(i=0; i<numtilesx*numtilesy; i++)
		{
			if((jpegbuf[i]=(unsigned char *)malloc(TJBUFSIZE(tilesizex, tilesizey))) == NULL)
			{
				puts("ERROR: Could not allocate image buffers.");
				goto bailout;
			}
		}

		// Compression test
		if(quiet) printf("%s\t%s\t%s\t%d\t",  _pfname[pf], bu?"BU":"TD",
			_subnamel[jpegsub], qual);
		for(i=0; i<h; i++) memcpy(&rgbbuf[pitch*i], &srcbuf[w*ps*i], w*ps);
		if((hnd=tjInitCompress())==NULL)
		{
			printf("Error in tjInitCompress():\n%s\n", tjGetErrorStr());
			goto bailout;
		}
		_catch(tjCompress(hnd, rgbbuf, tilesizex, pitch, tilesizey, ps,
			jpegbuf[0], &comptilesize[0], jpegsub, qual, flags));
		ITER=0;
		timer.start();
		do
		{
			jpgbufsize=0;  int tilen=0;
			for(i=0; i<h; i+=tilesizey)
			{
				for(j=0; j<w; j+=tilesizex)
				{
					int tempw=min(tilesizex, w-j), temph=min(tilesizey, h-i);
					_catch(tjCompress(hnd, &rgbbuf[pitch*i+j*ps], tempw, pitch,
						temph, ps, jpegbuf[tilen], &comptilesize[tilen], jpegsub, qual,
						flags));
					jpgbufsize+=comptilesize[tilen];
					tilen++;
				}
			}
			ITER++;
		} while((elapsed=timer.elapsed())<5.);
		_catch(tjDestroy(hnd));
		if(quiet)
		{
			if(tilesizex==w && tilesizey==h) printf("Full     \t");
			else printf("%-4d %-4d\t", tilesizex, tilesizey);
			printsigfig((double)(w*h)/1000000.*(double)ITER/elapsed, 4);
			printf("\t");
			printsigfig((double)(w*h*ps)/(double)jpgbufsize, 4);
			printf("\t");
		}
		else
		{
			if(tilesizex==w && tilesizey==h) printf("\nFull image\n");
			else printf("\nTile size: %d x %d\n", tilesizex, tilesizey);
			printf("C--> Frame rate:           %f fps\n", (double)ITER/elapsed);
			printf("     Output image size:    %d bytes\n", jpgbufsize);
			printf("     Compression ratio:    %f:1\n",
				(double)(w*h*ps)/(double)jpgbufsize);
			printf("     Source throughput:    %f Megapixels/sec\n",
				(double)(w*h)/1000000.*(double)ITER/elapsed);
			printf("     Output bit stream:    %f Megabits/sec\n",
				(double)jpgbufsize*8./1000000.*(double)ITER/elapsed);
		}
		if(tilesizex==w && tilesizey==h)
		{
			sprintf(tempstr, "%s_%sQ%d.jpg", filename, _subnames[jpegsub], qual);
			if((outfile=fopen(tempstr, "wb"))==NULL)
			{
				puts("ERROR: Could not open reference image");
				exit(1);
			}
			if(fwrite(jpegbuf[0], jpgbufsize, 1, outfile)!=1)
			{
				puts("ERROR: Could not write reference image");
				exit(1);
			}
			fclose(outfile);
			if(!quiet) printf("Reference image written to %s\n", tempstr);
		}

		// Decompression test
		memset(rgbbuf, 127, pitch*h);  // Grey image means decompressor did nothing
		if((hnd=tjInitDecompress())==NULL)
		{
			printf("Error in tjInitDecompress():\n%s\n", tjGetErrorStr());
			goto bailout;
		}
		_catch(tjDecompress(hnd, jpegbuf[0], jpgbufsize, rgbbuf, tilesizex, pitch,
			tilesizey, ps, flags));
		ITER=0;
		timer.start();
		do
		{
			int tilen=0;
			for(i=0; i<h; i+=tilesizey)
			{
				for(j=0; j<w; j+=tilesizex)
				{
					int tempw=min(tilesizex, w-j), temph=min(tilesizey, h-i);
					_catch(tjDecompress(hnd, jpegbuf[tilen], comptilesize[tilen],
						&rgbbuf[pitch*i+ps*j], tempw, pitch, temph, ps, flags));
					tilen++;
				}
			}
			ITER++;
		}	while((elapsed=timer.elapsed())<5.);
		_catch(tjDestroy(hnd));
		if(quiet)
		{
			printsigfig((double)(w*h)/1000000.*(double)ITER/elapsed, 4);
			printf("\n");
		}
		else
		{
			printf("D--> Frame rate:           %f fps\n", (double)ITER/elapsed);
			printf("     Dest. throughput:     %f Megapixels/sec\n",
				(double)(w*h)/1000000.*(double)ITER/elapsed);
		}
		if(tilesizex==w && tilesizey==h)
			sprintf(tempstr, "%s_%sQ%d_full.%s", filename, _subnames[jpegsub], qual,
				useppm?"ppm":"bmp");
		else sprintf(tempstr, "%s_%sQ%d_%dx%d.%s", filename, _subnames[jpegsub],
			qual, tilesizex, tilesizey, useppm?"ppm":"bmp");
		if(savebmp(tempstr, rgbbuf, w, h, pf, pitch, bu)==-1)
		{
			printf("ERROR saving bitmap: %s\n", bmpgeterr());
			goto bailout;
		}
		sprintf(strrchr(tempstr, '.'), "-err.%s", useppm?"ppm":"bmp");
		if(!quiet)
			printf("Computing compression error and saving to %s.\n", tempstr);
		if(jpegsub==TJ_GRAYSCALE)
		{
			for(j=0; j<h; j++)
			{
				for(i=0; i<w*ps; i+=ps)
				{
					int y=(int)((double)srcbuf[w*ps*j+i+_rindex[pf]]*0.299
						+ (double)srcbuf[w*ps*j+i+_gindex[pf]]*0.587
						+ (double)srcbuf[w*ps*j+i+_bindex[pf]]*0.114 + 0.5);
					if(y>255) y=255;  if(y<0) y=0;
					rgbbuf[pitch*j+i+_rindex[pf]]=abs(rgbbuf[pitch*j+i+_rindex[pf]]-y);
					rgbbuf[pitch*j+i+_gindex[pf]]=abs(rgbbuf[pitch*j+i+_gindex[pf]]-y);
					rgbbuf[pitch*j+i+_bindex[pf]]=abs(rgbbuf[pitch*j+i+_bindex[pf]]-y);
				}
			}
		}		
		else
		{
			for(j=0; j<h; j++) for(i=0; i<w*ps; i++)
				rgbbuf[pitch*j+i]=abs(rgbbuf[pitch*j+i]-srcbuf[w*ps*j+i]);
		}
		if(savebmp(tempstr, rgbbuf, w, h, pf, pitch, bu)==-1)
		{
			printf("ERROR saving bitmap: %s\n", bmpgeterr());
			goto bailout;
		}

		// Cleanup
		if(jpegbuf)
		{
			for(i=0; i<numtilesx*numtilesy; i++)
				{if(jpegbuf[i]) free(jpegbuf[i]);  jpegbuf[i]=NULL;}
			free(jpegbuf);  jpegbuf=NULL;
		}
		if(comptilesize) {free(comptilesize);  comptilesize=NULL;}
	} while(tilesizex<w || tilesizey<h);

	if(rgbbuf) {free(rgbbuf);  rgbbuf=NULL;}
	return;

	bailout:
	if(jpegbuf)
	{
		for(i=0; i<numtilesx*numtilesy; i++)
			{if(jpegbuf[i]) free(jpegbuf[i]);  jpegbuf[i]=NULL;}
		free(jpegbuf);  jpegbuf=NULL;
	}
	if(comptilesize) {free(comptilesize);  comptilesize=NULL;}
	if(rgbbuf) {free(rgbbuf);  rgbbuf=NULL;}
	return;
}


int main(int argc, char *argv[])
{
	unsigned char *bmpbuf=NULL;  int w, h, i, useppm=0;
	int qual, dotile=0, quiet=0, hiqual=-1;  char *temp;
	BMPPIXELFORMAT pf=BMP_BGR;
	int bu=0;

	printf("\n");

	if(argc<3)
	{
		printf("USAGE: %s <Inputfile (BMP|PPM)> <%% Quality>\n\n", argv[0]);
		printf("       [-tile]\n");
		printf("       Test performance of the codec when the image is encoded\n");
		printf("       as separate tiles of varying sizes.\n\n");
		printf("       [-forcemmx] [-forcesse] [-forcesse2] [-forcesse3]\n");
		printf("       Force MMX, SSE, or SSE2 code paths in Intel codec\n\n");
		printf("       [-rgb | -bgr | -rgba | -bgra | -abgr | -argb]\n");
		printf("       Test the specified color conversion path in the codec (default: BGR)\n\n");
		printf("       [-fastupsample]\n");
		printf("       Use fast, inaccurate upsampling code to perform 4:2:2 and 4:2:0\n");
		printf("       YUV decoding in libjpeg decompressor\n\n");
		printf("       [-quiet]\n");
		printf("       Output in tabular rather than verbose format\n\n");
		printf("       NOTE: If the quality is specified as a range, i.e. 90-100, a separate\n");
		printf("       test will be performed for all quality values in the range.\n");
		exit(1);
	}
	if((qual=atoi(argv[2]))<1 || qual>100)
	{
		puts("ERROR: Quality must be between 1 and 100.");
		exit(1);
	}
	if((temp=strchr(argv[2], '-'))!=NULL && strlen(temp)>1
		&& sscanf(&temp[1], "%d", &hiqual)==1 && hiqual>qual && hiqual>=1
		&& hiqual<=100) {}
	else hiqual=qual;

	if(argc>3)
	{
		for(i=3; i<argc; i++)
		{
			if(!stricmp(argv[i], "-tile")) dotile=1;
			if(!stricmp(argv[i], "-forcesse3"))
			{
				printf("Using SSE3 code\n");
				forcesse3=1;
			}
			if(!stricmp(argv[i], "-forcesse2"))
			{
				printf("Using SSE2 code\n");
				forcesse2=1;
			}
			if(!stricmp(argv[i], "-forcesse"))
			{
				printf("Using SSE code\n");
				forcesse=1;
			}
			if(!stricmp(argv[i], "-forcemmx"))
			{
				printf("Using MMX code\n");
				forcemmx=1;
			}
			if(!stricmp(argv[i], "-fastupsample"))
			{
				printf("Using fast upsampling code\n");
				fastupsample=1;
			}
			if(!stricmp(argv[i], "-rgb")) pf=BMP_RGB;
			if(!stricmp(argv[i], "-rgba")) pf=BMP_RGBA;
			if(!stricmp(argv[i], "-bgr")) pf=BMP_BGR;
			if(!stricmp(argv[i], "-bgra")) pf=BMP_BGRA;
			if(!stricmp(argv[i], "-abgr")) pf=BMP_ABGR;
			if(!stricmp(argv[i], "-argb")) pf=BMP_ARGB;
			if(!stricmp(argv[i], "-bottomup")) bu=1;
			if(!stricmp(argv[i], "-quiet")) quiet=1;
		}
	}

	if(loadbmp(argv[1], &bmpbuf, &w, &h, pf, 1, bu)==-1)
	{
		printf("ERROR loading bitmap: %s\n", bmpgeterr());  exit(1);
	}

	temp=strrchr(argv[1], '.');
	if(temp!=NULL)
	{
		if(!stricmp(temp, ".ppm")) useppm=1;
		*temp='\0';
	}

	if(quiet)
	{
		printf("All performance values in Mpixels/sec\n\n");
		printf("Bitmap\tBitmap\tJPEG\tJPEG\tTile Size\tCompr\tCompr\tDecomp\n");
		printf("Format\tOrder\tFormat\tQual\t X    Y  \tPerf \tRatio\tPerf\n\n");
	}

	for(i=hiqual; i>=qual; i--)
		dotest(bmpbuf, w, h, pf, bu, TJ_GRAYSCALE, i, argv[1], dotile, useppm, quiet);
	if(quiet) printf("\n");
	for(i=hiqual; i>=qual; i--)
		dotest(bmpbuf, w, h, pf, bu, TJ_420, i, argv[1], dotile, useppm, quiet);
	if(quiet) printf("\n");
	for(i=hiqual; i>=qual; i--)
		dotest(bmpbuf, w, h, pf, bu, TJ_422, i, argv[1], dotile, useppm, quiet);
	if(quiet) printf("\n");
	for(i=hiqual; i>=qual; i--)
		dotest(bmpbuf, w, h, pf, bu, TJ_444, i, argv[1], dotile, useppm, quiet);

	if(bmpbuf) free(bmpbuf);
	return 0;
}
