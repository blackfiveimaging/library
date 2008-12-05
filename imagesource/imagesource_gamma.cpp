/*
 * imagesource_gamma.cpp
 * Gamma-corrects an image.
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <stdlib.h>

#include <math.h>

#include "imagesource_gamma.h"


using namespace std;

ImageSource_Gamma::~ImageSource_Gamma()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_Gamma::GetRow(int row)
{
	ISDataType *src;

	if(row==currentrow)
		return(rowbuffer);

	src=source->GetRow(row);

	switch(STRIP_ALPHA(type))
	{
		case IS_TYPE_RGB:	// RGB is the only colourspace in which we treat 0 as black.
			if(HAS_ALPHA(type))	// We don't want to adjust the alpha channel!
			{
				for(int i=0;i<width;++i)
				{
					int j;
					for(j=0;j<samplesperpixel-1;++j)
						rowbuffer[i*samplesperpixel+j]=IS_GAMMA(src[i*samplesperpixel+j],gamma);
					rowbuffer[i*samplesperpixel+j]=src[i*samplesperpixel+j];	// Copy alpha unchanged
				}	
			}
			else
			{
				for(int i=0;i<width;++i)
				{
					for(int j=0;j<samplesperpixel;++j)
						rowbuffer[i*samplesperpixel+j]=IS_GAMMA(src[i*samplesperpixel+j],gamma);
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
						rowbuffer[i*samplesperpixel+j]=IS_SAMPLEMAX-IS_GAMMA(IS_SAMPLEMAX-src[i*samplesperpixel+j],gamma);
					rowbuffer[i*samplesperpixel+j]=src[i*samplesperpixel+j];	// Copy alpha unchanged
				}	
			}
			else
			{
				for(int i=0;i<width;++i)
				{
					for(int j=0;j<samplesperpixel;++j)
						rowbuffer[i*samplesperpixel+j]=IS_SAMPLEMAX-IS_GAMMA(IS_SAMPLEMAX-src[i*samplesperpixel+j],gamma);
				}	
			}		
			break;
	}

	currentrow=row;
	return(rowbuffer);
}


ImageSource_Gamma::ImageSource_Gamma(ImageSource *source,float gamma)
	: ImageSource(source), source(source), gamma(gamma)
{
	MakeRowBuffer();
}

