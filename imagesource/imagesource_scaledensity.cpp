/*
 * imagesource_scaledensity.cpp
 * Scales values in an imagesource according to a given value.
 * Scales RGB towards IS_SAMPLEMAX, scales Grey and CMYK towards 0.
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <stdlib.h>

#include <math.h>

#include "imagesource_scaledensity.h"


using namespace std;

ImageSource_ScaleDensity::~ImageSource_ScaleDensity()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_ScaleDensity::GetRow(int row)
{
	ISDataType *src;

	if(row==currentrow)
		return(rowbuffer);

	src=source->GetRow(row);

	switch(STRIP_ALPHA(type))
	{
		case IS_TYPE_RGB:	// RGB is the only colourspace in which we treat 0 as black.
			if(HAS_ALPHA(type))	// We don't want to scale the alpha channel!
			{
				for(int i=0;i<width;++i)
				{
					int j;
					for(j=0;j<samplesperpixel-1;++j)
						rowbuffer[i*samplesperpixel+j]=IS_SAMPLEMAX-ISDataType(density * (IS_SAMPLEMAX-src[i*samplesperpixel+j]));
					rowbuffer[i*samplesperpixel+j]=src[i*samplesperpixel+j];	// Copy alpha unchanged
				}	
			}
			else
			{
				for(int i=0;i<width;++i)
				{
					for(int j=0;j<samplesperpixel;++j)
						rowbuffer[i*samplesperpixel+j]=IS_SAMPLEMAX-ISDataType(density * (IS_SAMPLEMAX-src[i*samplesperpixel+j]));
				}	
			}		
			break;
		default:
			if(HAS_ALPHA(type))	// We don't want to scale the alpha channel!
			{
				for(int i=0;i<width;++i)
				{
					int j;
					for(j=0;j<samplesperpixel-1;++j)
						rowbuffer[i*samplesperpixel+j]=ISDataType(density * src[i*samplesperpixel+j]);
					rowbuffer[i*samplesperpixel+j]=src[i*samplesperpixel+j];	// Copy alpha unchanged
				}	
			}
			else
			{
				for(int i=0;i<width;++i)
				{
					for(int j=0;j<samplesperpixel;++j)
						rowbuffer[i*samplesperpixel+j]=ISDataType(density * src[i*samplesperpixel+j]);
				}	
			}		
			break;
	}

	currentrow=row;
	return(rowbuffer);
}


ImageSource_ScaleDensity::ImageSource_ScaleDensity(ImageSource *source,float density)
	: ImageSource(source), source(source), density(density)
{
	MakeRowBuffer();
}

