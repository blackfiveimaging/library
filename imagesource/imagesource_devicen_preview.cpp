/*
 * imagesource_ncolour_preview.cpp
 *
 * Renders an RGB preview of a DeviceN Image
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

#include "imagesource_devicen_preview.h"

using namespace std;

ImageSource_DeviceN_Preview::~ImageSource_DeviceN_Preview()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_DeviceN_Preview::GetRow(int row)
{
	if(row==currentrow)
		return(rowbuffer);

	ISDataType *srcdata=source->GetRow(row);

	switch(type)
	{
		case IS_TYPE_RGBA:
			for(int x=0;x<width;++x)
			{
				unsigned int red=IS_SAMPLEMAX,green=IS_SAMPLEMAX,blue=IS_SAMPLEMAX;
				for(int s=0;s<source->samplesperpixel-1;++s)
				{
					unsigned int t=srcdata[x*source->samplesperpixel+s];
					unsigned int tr=IS_SAMPLEMAX-((IS_SAMPLEMAX-colorants[s].red)*t)/IS_SAMPLEMAX;
					unsigned int tg=IS_SAMPLEMAX-((IS_SAMPLEMAX-colorants[s].green)*t)/IS_SAMPLEMAX;
					unsigned int tb=IS_SAMPLEMAX-((IS_SAMPLEMAX-colorants[s].blue)*t)/IS_SAMPLEMAX;
					if(tr<red) red=tr;
					if(tg<green) green=tg;
					if(tb<blue) blue=tb;
				}
				rowbuffer[x*samplesperpixel]=red;
				rowbuffer[x*samplesperpixel+1]=green;
				rowbuffer[x*samplesperpixel+2]=blue;
				rowbuffer[x*samplesperpixel+3]=srcdata[x*source->samplesperpixel+source->samplesperpixel-1];
			}
			break;
		case IS_TYPE_RGB:
			for(int x=0;x<width;++x)
			{
				unsigned int red=IS_SAMPLEMAX,green=IS_SAMPLEMAX,blue=IS_SAMPLEMAX;
				for(int s=0;s<source->samplesperpixel;++s)
				{
					unsigned int t=srcdata[x*source->samplesperpixel+s];
					unsigned int tr=IS_SAMPLEMAX-((IS_SAMPLEMAX-colorants[s].red)*t)/IS_SAMPLEMAX;
					unsigned int tg=IS_SAMPLEMAX-((IS_SAMPLEMAX-colorants[s].green)*t)/IS_SAMPLEMAX;
					unsigned int tb=IS_SAMPLEMAX-((IS_SAMPLEMAX-colorants[s].blue)*t)/IS_SAMPLEMAX;
					if(tr<red) red=tr;
					if(tg<green) green=tg;
					if(tb<blue) blue=tb;
				}
				rowbuffer[x*samplesperpixel]=red;
				rowbuffer[x*samplesperpixel+1]=green;
				rowbuffer[x*samplesperpixel+2]=blue;
			}
			break;
		default:
			throw "DeviceN Preview currently only supports RGB output";
			break;
	}

	currentrow=row;

	return(rowbuffer);
}


ImageSource_DeviceN_Preview::ImageSource_DeviceN_Preview(struct ImageSource *source,ISDeviceN_Colorant_Preview *colorants)
	: ImageSource(source), source(source), colorants(colorants)
{
	if(HAS_ALPHA(type))
	{
		type=IS_TYPE_RGBA;
		samplesperpixel=4;
	}
	else
	{
		type=IS_TYPE_RGB;
		samplesperpixel=3;
	}
	MakeRowBuffer();
}

