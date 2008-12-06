/* 
 * imagesource_hticks.cpp - renders a horizontal scale of "tick marks".
 * Supports Random Access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


#include <iostream.h>

#include "imagesource_hticks.h"

ISDataType *ImageSource_HTicks::GetRow(int row)
{
	if(currentrow==row)
		return(rowbuffer);

	int majortick=0;
	int minortick=0;

	for(int x=0;x<width;++x)
	{
		if((((x+1)*majorticks)/width)>majortick)
		{
			for(int s=0;s<samplesperpixel;++s)
				rowbuffer[x*samplesperpixel+s]=fg[s];
			++majortick;
			++minortick;
		}
		else
		{
			if((row>height/2) && (((x+1)*minorticks)/width)>minortick)
			{
				for(int s=0;s<samplesperpixel;++s)
					rowbuffer[x*samplesperpixel+s]=fg[s];
				++minortick;
			}
			else
			{
				for(int s=0;s<samplesperpixel;++s)
					rowbuffer[x*samplesperpixel+s]=bg[s];
			}
		}
	}
	
	currentrow=row;
	return(rowbuffer);
}


ImageSource_HTicks::ImageSource_HTicks(int width,int height,IS_TYPE type,int samplesperpixel,int majorticks,int minorticks)
	: ImageSource(), majorticks(majorticks), minorticks(minorticks), fg(samplesperpixel,IS_SAMPLEMAX), bg(samplesperpixel,0)
{
	this->type=type;
	this->width=width;
	this->height=height;
	this->samplesperpixel=samplesperpixel;
	randomaccess=true;
	MakeRowBuffer();
}

ImageSource_HTicks::~ImageSource_HTicks()
{
}


void ImageSource_HTicks::SetFG(ISDeviceNValue &col)
{
	fg=col;
}


void ImageSource_HTicks::SetBG(ISDeviceNValue &col)
{
	bg=col;
}

