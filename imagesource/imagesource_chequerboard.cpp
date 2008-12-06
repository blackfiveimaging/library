/* 
 * imagesource_Chequerboard.cpp - renders a chequerboard pattern.
 * Supports Random Access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


#include <iostream.h>

#include "imagesource_chequerboard.h"

ISDataType *ImageSource_Chequerboard::GetRow(int row)
{
	if(currentrow==row)
		return(rowbuffer);

	int vphase=(row/size)&1;

	for(int x=0;x<width;++x)
	{
		int hphase=(x/size)&1;

		if(hphase^vphase)
		{
			for(int s=0;s<samplesperpixel;++s)
				rowbuffer[x*samplesperpixel+s]=fg[s];
		}
		else
		{
			for(int s=0;s<samplesperpixel;++s)
				rowbuffer[x*samplesperpixel+s]=bg[s];
		}
	}
	
	currentrow=row;
	return(rowbuffer);
}


ImageSource_Chequerboard::ImageSource_Chequerboard(int width,int height,IS_TYPE type,int samplesperpixel,int size)
	: ImageSource(), size(size), fg(samplesperpixel,IS_SAMPLEMAX), bg(samplesperpixel,0)
{
	this->type=type;
	this->width=width;
	this->height=height;
	this->samplesperpixel=samplesperpixel;
	randomaccess=true;
	MakeRowBuffer();
}

ImageSource_Chequerboard::~ImageSource_Chequerboard()
{
}


void ImageSource_Chequerboard::SetFG(ISDeviceNValue &col)
{
	fg=col;
}


void ImageSource_Chequerboard::SetBG(ISDeviceNValue &col)
{
	bg=col;
}

