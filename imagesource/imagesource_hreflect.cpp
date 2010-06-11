/*
 * imagesource_hreflect.h - filter to reflect an image horizontally.
 * supports random access, even if source image doesn't.
 *
 * Copyright (c) 2010 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <stdlib.h>

#include <math.h>

#include "imagesource_hreflect.h"


using namespace std;

ImageSource_HReflect::~ImageSource_HReflect()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_HReflect::GetRow(int row)
{
	ISDataType *src;

	if(row==currentrow)
		return(rowbuffer);

	src=source->GetRow(row);

	for(int x=0;x<width;++x)
	{
		for(int s=0;s<samplesperpixel;++s)
		{
			rowbuffer[x*samplesperpixel+s]=src[(width-x-1)*samplesperpixel+s];
		}
	}
	currentrow=row;
	return(rowbuffer);
}


ImageSource_HReflect::ImageSource_HReflect(ImageSource *source)
	: ImageSource(source), source(source)
{
	MakeRowBuffer();
}

