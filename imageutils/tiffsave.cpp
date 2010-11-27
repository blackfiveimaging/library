/*
 * tiffsave.cpp - class for saving an ImageSource to disk as a TIFF file.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * TODO: Add support for 16-bit output, and other colour-spaces
 *
 */


#include <iostream>

#include <stdlib.h>
#include <tiff.h>
#include <tiffio.h>
#include <sys/stat.h>

#include "lcmswrapper.h"
#include "binaryblob.h"
#include "util.h"

#include "tiffsave.h"

using namespace std;


TIFFSaver::~TIFFSaver()
{
	if(file)
	{
		TIFFWriteDirectory(file);
		TIFFClose(file);
	}
	
	if(tmpbuffer)
		free(tmpbuffer);
	if(embprofile)
		delete embprofile;
}


void TIFFSaver::Save()
{
	if(file)
	{
		int firstrow,i;
		unsigned char *dst;
		ISDataType *src;

		for(firstrow=0;firstrow<height;firstrow+=stripheight)
		{
			int lastrow=firstrow+stripheight;
			int row;
			if(lastrow>(height))
				lastrow=height;

			for(row=firstrow;row<lastrow;++row)
			{
				if(progress && !(row&31))
				{
					if(!progress->DoProgress(row,height))
						return;
				}

				dst=tmpbuffer+(row-firstrow)*(deep ? bytesperrow*2 : bytesperrow);
				
				src=imagesource->GetRow(row);
				
				switch(bitsperpixel)
				{
					case 1:		// 1 bit per sample - pack 8 samples into each byte.
						for(i=0;i<width;i+=8)
						{
							int j;
							unsigned int t,t2=0;
							int l=(width-i)>8 ? 7 : width-(i+1);
							for(j=0;j<=l;++j)
							{
								t=int(src[i+l-j]);
								t2>>=1;
								t2|=t&0x8000;
							}
							dst[i/8]=t2;
						}
						break;
					default:
						if(deep)	// Save with 16 bits per sample?
						{
							unsigned short *dst16=(unsigned short *)dst;
							for(i=0;i<bytesperrow;++i)
							{
								dst16[i]=src[i];
							}
						}
						else	// 8 bits per sample
						{
							for(i=0;i<bytesperrow;++i)
							{
								unsigned int t;
								t=int(src[i]);
								t=ISTOEIGHT(t);
								if(t>255) t=255;
								if(t<0) t=0;
								dst[i]=t;
							}
						}
						break;
				}
			}
			TIFFWriteEncodedStrip(file, firstrow/stripheight, tmpbuffer, stripsize);
		}
		if(progress)
			progress->DoProgress(height,height);
	}
}


TIFFSaver::TIFFSaver(const char *filename,struct ImageSource *is,bool deep,int bpp,int compression)
	: ImageSaver(), deep(deep), imagesource(is), embprofile(NULL)
{
	switch(is->type)
	{
		case IS_TYPE_BW:
			bitsperpixel=1;
			break;
		default:
			bitsperpixel=is->samplesperpixel*8;
			break;
	}
	if(bpp && (bpp!=bitsperpixel))
	{
		if((bitsperpixel!=8)||(bpp!=1))
			throw "Currently only 8-bit -> 1-bit conversion is supported.";
	}

	this->width=is->width;
	this->height=is->height;

	this->xres=is->xres;
	this->yres=is->yres;
	
	stripheight=TIFFSAVE_STRIPHEIGHT;

	tmpbuffer=NULL;
#if WIN32
	wchar_t *tmpfn=UTF8ToWChar(filename);
	if(!(file = TIFFOpenW(tmpfn, "w")))
	{
		throw "Can't open file";
	}
	free(tmpfn);
#else
	if(!(file = TIFFOpen(filename, "w")))
	{
		throw "Can't open file";
	}
#endif
	switch(bitsperpixel)
	{
		case 40:
			if(HAS_ALPHA(is->type))
			{
				TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED);
				TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 5);
				TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
				TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, deep ? 16 : 8);
			}
			else
				throw "DeviceN TIFF output is not yet complete";
			break;
		case 32:
			if(HAS_ALPHA(is->type))
			{
				TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
				TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 4);
				TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
				TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, deep ? 16 : 8);
			}
			else
			{
				TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED);
				TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 4);
				TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
				TIFFSetField(file, TIFFTAG_INKSET, INKSET_CMYK);
				TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, deep ? 16 : 8);
			}
			break;

		case 24:
			TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
			TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 3);
			TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, deep ? 16 : 8);
			break;

		case 8:
			TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
			TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 1);
			TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, deep ? 16 : 8);
			break;
		
		case 1:
			TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
			TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 1);
			TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, 1);
			break;
	
		default:
			fprintf(stderr,"FIXME: unsupported bitspersample: %d\n",bitsperpixel);
			TIFFClose(file);
			throw "Unsupported image type";
			break;
	}
	TIFFSetField(file, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(file, TIFFTAG_IMAGELENGTH, height);
	TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, stripheight);
	
	double xr=xres;
	double yr=yres;
	
	TIFFSetField(file, TIFFTAG_XRESOLUTION, xr);
	TIFFSetField(file, TIFFTAG_YRESOLUTION, yr);
	TIFFSetField(file, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	TIFFSetField(file, TIFFTAG_COMPRESSION, compression);

	if(is->GetEmbeddedProfile())
	{
		if(embprofile)
			delete embprofile;
		embprofile=is->GetEmbeddedProfile()->GetBlob();
		TIFFSetField(file, TIFFTAG_ICCPROFILE, embprofile->GetSize(),embprofile->GetPointer());
	}

	stripsize = TIFFStripSize(file);
	bytesperrow = (width*bitsperpixel+7)/8;

	if(!(tmpbuffer=(unsigned char *)malloc(stripsize)))
	{
		TIFFClose(file);
		throw "No memory for tmpbuffer";
	}
}

