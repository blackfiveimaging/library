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

#include "imagesource.h"
#include "imagesaver.h"

#define TIFFSAVE_STRIPHEIGHT 64

class TIFFSaver : public ImageSaver
{
	public:
	TIFFSaver(const char *filename,ImageSource *is,bool deep=false,int bitsperpixel=0,int compression=COMPRESSION_NONE);
	virtual ~TIFFSaver();
	virtual void Save();
	private:
	int width,height;
	int bitsperpixel;
	bool deep;
	float xres;
	float yres;
	int stripheight;
	int stripsize;
	int bytesperrow;
	TIFF *file;
	unsigned char *tmpbuffer;
	struct ImageSource *imagesource;
};

#endif
