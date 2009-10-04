/* 
 * imagesource_hsweep.cpp - renders a horizontal gradient pattern.
 * Supports Random Access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


#include <iostream>

#include "imagesource_hsweep.h"

ISDataType *ImageSource_HSweep::GetRow(int row)
{
	if(currentrow==row)
		return(rowbuffer);
	for(int x=0;x<width;++x)
	{
		for(int s=0;s<samplesperpixel;++s)
		{
			int t=(x*(right[s]-left[s]))/width;
			rowbuffer[x*samplesperpixel+s]=left[s]+t;
		}
	}
	
	currentrow=row;
	return(rowbuffer);
}


ImageSource_HSweep::ImageSource_HSweep(int width,int height,ISDeviceNValue &left,ISDeviceNValue &right,IS_TYPE type)
	: ImageSource(), left(left),right(right)
{
	this->type=type;
	this->width=width;
	this->height=height;
	this->samplesperpixel=left.GetChannels();
	randomaccess=true;
	MakeRowBuffer();
}

ImageSource_HSweep::~ImageSource_HSweep()
{
}

