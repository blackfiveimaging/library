/*
 * imagesource_vreflect.h - filter to reflect an image vertically.
 * supports random access, even if source image doesn't.
 *
 * Copyright (c) 2010 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include <stdlib.h>

#include "../support/debug.h"

#include "imagesource_vreflect.h"

using namespace std;

ImageSource_VReflect::~ImageSource_VReflect()
{
	if(source)
		delete source;
	if(spanbuffer)
		free(spanbuffer);
}


ISDataType *ImageSource_VReflect::GetRow(int row)
{
	int firstrow,lastrow;
	ISDataType *dst;
	ISDataType *src;
	
	if((row<spanfirstrow) || (row>=(spanfirstrow+spanrows)))
	{
		spanfirstrow=row;

		firstrow=row;
		lastrow=row+spanrows;
		if(lastrow>height)
			lastrow=height;

		for(int y=0;y<height;++y)
		{
			src=source->GetRow(y);
			dst=spanbuffer+((height-1)-y)*samplesperrow;
			for(int i=0;i<(width*samplesperpixel);++i)
			{
				dst[i]=src[i];
			}
			if(TestBreak())
				y=source->height;
		}
	}
	row-=spanfirstrow;
	return(spanbuffer+row*samplesperrow);		
}


ImageSource_VReflect::ImageSource_VReflect(ImageSource *source)
	: ImageSource_Interruptible(source), source(source), spanfirstrow(0), spanrows(0), spanbuffer(NULL)
{
	rowbuffer=NULL;
	Debug[COMMENT] << "VReflect: caching entire image for vertical flip" << endl;
	this->spanrows=source->height+1;

	spanfirstrow=-this->spanrows-1;
	samplesperrow=width*samplesperpixel;

	Debug[TRACE] << "Span buffer will be " << this->spanrows << " high" << endl;
	spanbuffer=(ISDataType *)malloc(this->spanrows*(sizeof(ISDataType)*samplesperrow));
	currentrow=-1;
	randomaccess=true;
}

