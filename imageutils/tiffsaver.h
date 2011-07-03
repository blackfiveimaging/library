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


#ifndef TIFFSAVE_H
#define TIFFSAVE_H

#include <tiffio.h>
#include <cstdlib>

#include "imagesource.h"
#include "imagesaver.h"
#include "binaryblob.h"
#include "util.h"

#define TIFFSAVE_STRIPHEIGHT 64

class TIFFWrapper
{
	public:
	TIFFWrapper(const char *filename,const char *mode) : file(NULL)
	{
#if WIN32
		wchar_t *tmpfn=UTF8ToWChar(filename);
		if(!(file = TIFFOpenW(tmpfn, mode)))
		{
			throw "Can't open file";
		}
		free(tmpfn);
#else
		if(!(file = TIFFOpen(filename, mode)))
		{
			throw "Can't open file";
		}
#endif
	}
	~TIFFWrapper()
	{
		if(file)
			TIFFClose(file);
	}
	TIFF &operator*()
	{
		return(*file);
	}
	TIFF *operator->()
	{
		return(file);
	}
	protected:
	TIFF *file;	
};


class TIFFSaver : public ImageSaver
{
	public:
	TIFFSaver(const char *filename,RefCountPtr<ImageSource> is,bool deep=false,int bitsperpixel=0,int compression=COMPRESSION_NONE);
	virtual ~TIFFSaver();
	virtual void ProcessRow(int row);
	private:
	int width,height;
	bool deep;
	float xres;
	float yres;
	int stripheight;
	int stripsize;
	int bytesperrow;
	TIFFWrapper file;
	unsigned char *tmpbuffer;
	BinaryBlob *embprofile;
	RefCountPtr<ISParasite> psirb;	// PhotoShop Image Resource Block - for clipping path.
};

#endif
