/*
 * imagesource_invert.cpp
 *
 * Supports all types
 * Supports random access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "imagesource_invert.h"

using namespace std;

ImageSource_Invert::~ImageSource_Invert()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_Invert::GetRow(int row)
{
	if(row==currentrow)
		return(rowbuffer);

	ISDataType *srcdata=source->GetRow(row);

	if(HAS_ALPHA(type))
	{
		for(int x=0;x<width;++x)
		{
			int s=0;
			for(s=0;s<samplesperpixel-1;++s)
			{
				rowbuffer[s]=IS_SAMPLEMAX-srcdata[s];
			}
			rowbuffer[s]=srcdata[s];	// Leave alpha channel unchanged if present
		}
	}
	else
	{
		for(int s=0;s<width*samplesperpixel;++s)
		{
			rowbuffer[s]=IS_SAMPLEMAX-srcdata[s];
		}
	}
	
	currentrow=row;

	return(rowbuffer);
}


ImageSource_Invert::ImageSource_Invert(ImageSource *source)
	: ImageSource(source), source(source)
{
	MakeRowBuffer();
}
