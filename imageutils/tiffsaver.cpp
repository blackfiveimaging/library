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

#include "tiffsaver.h"

using namespace std;


TIFFSaver::~TIFFSaver()
{
//	if(file)
//	{
		TIFFWriteDirectory(&*file);
//		TIFFClose(file);
//	}
	
	if(tmpbuffer)
		free(tmpbuffer);
	if(embprofile)
		delete embprofile;
}


void TIFFSaver::ProcessRow(int row)
{
//	if(file)
//	{
		int strip=row/stripheight;
		int firstrow=strip*stripheight;
		int lastrow=firstrow+stripheight-1;
		if(lastrow>=height)
			lastrow=height-1;

//		unsigned char *dst=tmpbuffer+(row-firstrow)*(deep ? bytesperrow*2 : bytesperrow);
		unsigned char *dst=tmpbuffer+(row-firstrow) * bytesperrow;
				
		ISDataType *src=source->GetRow(row);
				
		switch(STRIP_ALPHA(source->type))
		{
			case IS_TYPE_BW:		// 1 bit per sample - pack 8 samples into each byte.
				for(int i=0;i<width;i+=8)
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
					for(int i=0;i<(source->width*source->samplesperpixel);++i)
					{
						dst16[i]=src[i];
					}
				}
				else	// 8 bits per sample
				{
					for(int i=0;i<(source->width*source->samplesperpixel);++i)
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
		if(row==lastrow)
		{
			TIFFWriteEncodedStrip(&*file, strip, tmpbuffer, stripsize);
		}
//	}
}


#if 0
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
				
				src=source->GetRow(row);
				
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
#endif


TIFFSaver::TIFFSaver(const char *filename,RefCountPtr<ImageSource> is,bool deep,int bpp,int compression)
	: ImageSaver(is), deep(deep), file(filename,"w"), tmpbuffer(NULL), embprofile(NULL)
{
	int bitsperpixel;
	switch(is->type)
	{
		case IS_TYPE_BW:
			bitsperpixel=1;
			break;
		default:
			bitsperpixel=is->samplesperpixel* (deep ? 16 : 8);
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
#if 0
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
#endif
	if(HAS_ALPHA(is->type))
	{
	    uint16 v[1];
	    v[0] = EXTRASAMPLE_UNASSALPHA;
	    TIFFSetField(&*file, TIFFTAG_EXTRASAMPLES, 1, v);
	}

	int bps=deep ? 16 : 8;
	switch(STRIP_ALPHA(is->type))
	{
		case IS_TYPE_DEVICEN:
			TIFFSetField(&*file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED);
			break;
		case IS_TYPE_CMYK:
			TIFFSetField(&*file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED);
			TIFFSetField(&*file, TIFFTAG_INKSET, INKSET_CMYK);
			break;
		case IS_TYPE_RGB:
			TIFFSetField(&*file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
			break;
		case IS_TYPE_GREY:
			TIFFSetField(&*file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
			break;
		case IS_TYPE_BW:
			TIFFSetField(&*file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
			bps=1;
			break;
		default:
			Debug[ERROR] << "TIFFSaver: unsupported image type " << is->type << std::endl;
//			TIFFClose(file);
			throw "TIFFSaver: Unsupported image type";
			break;
	}

	TIFFSetField(&*file, TIFFTAG_SAMPLESPERPIXEL, is->samplesperpixel);
	TIFFSetField(&*file, TIFFTAG_BITSPERSAMPLE, bps);
	TIFFSetField(&*file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(&*file, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(&*file, TIFFTAG_IMAGELENGTH, height);
	TIFFSetField(&*file, TIFFTAG_ROWSPERSTRIP, stripheight);
	
	double xr=xres;
	double yr=yres;
	
	TIFFSetField(&*file, TIFFTAG_XRESOLUTION, xr);
	TIFFSetField(&*file, TIFFTAG_YRESOLUTION, yr);
	TIFFSetField(&*file, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	TIFFSetField(&*file, TIFFTAG_COMPRESSION, compression);

	if(is->GetEmbeddedProfile())
	{
		embprofile=is->GetEmbeddedProfile()->GetBlob();
		TIFFSetField(&*file, TIFFTAG_ICCPROFILE, embprofile->GetSize(),embprofile->GetPointer());
	}

	if(psirb=is->GetParasite(ISPARATYPE_PSIMAGERESOURCEBLOCK,ISPARA_TIFF))
	{
		Debug[TRACE] << "FOUND CLIPPING PATH -adding to output TIFF" << std::endl;
		TIFFSetField(&*file,TIFFTAG_PHOTOSHOP, psirb->GetSize(),psirb->GetPointer());
	}

	stripsize = TIFFStripSize(&*file);
	bytesperrow = (width*bitsperpixel+7)/8;

	if(!(tmpbuffer=(unsigned char *)malloc(stripsize)))
	{
//		TIFFClose(file);
		throw "No memory for tmpbuffer";
	}
}

