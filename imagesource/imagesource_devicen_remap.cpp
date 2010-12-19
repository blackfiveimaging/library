/*
 * imagesource_ncolour_remap.cpp
 *
 * Supports DeviceN
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

#include "../support/debug.h"

#include "imagesource_devicen_remap.h"

using namespace std;

ImageSource_DeviceN_Remap::~ImageSource_DeviceN_Remap()
{
	if(source)
		delete source;
	if(table)
		free(table);
}


ISDataType *ImageSource_DeviceN_Remap::GetRow(int row)
{
	if(row==currentrow)
		return(rowbuffer);

	// Clear the rowbuffer here, since if the channels are mismatched there may be channels we don't set.
	for(int x=0;x<width*samplesperpixel;++x)
		rowbuffer[x]=0;

	ISDataType *srcdata=source->GetRow(row);

	// For each pixel we remap each source channel to a channel in the rowbuffer, using the translation table.
	if(HAS_ALPHA(type))
	{
		for(int x=0;x<width;++x)
		{
			for(int s=0;s<source->samplesperpixel-1;++s)
			{
				if(table[s]>-1)
					rowbuffer[x*samplesperpixel+table[s]]=srcdata[x*source->samplesperpixel+s];
			}
			rowbuffer[x*samplesperpixel+source->samplesperpixel-1]=srcdata[x*source->samplesperpixel+source->samplesperpixel-1];
		}
	}
	else
	{
		for(int x=0;x<width;++x)
		{
			for(int s=0;s<source->samplesperpixel;++s)
			{
				if(table[s]>-1)
					rowbuffer[x*samplesperpixel+table[s]]=srcdata[x*source->samplesperpixel+s];
			}
		}
	}
	currentrow=row;

	return(rowbuffer);
}


ImageSource_DeviceN_Remap::ImageSource_DeviceN_Remap(struct ImageSource *source,const int *maptable,int outchannels)
	: ImageSource(source), source(source)
{
	table=(int *)malloc(sizeof(int)*source->samplesperpixel);

	if(outchannels)
		samplesperpixel=outchannels;
	type=IS_TYPE_DEVICEN;
	for(int i=0;i<source->samplesperpixel;++i)
	{
		table[i]=maptable[i];
		if(table[i]>=samplesperpixel)
			throw "Error: not enough output channels for remapping table";
	}
	MakeRowBuffer();
}


// ImageSource_ToDeviceN - "promotes" data from Greyscale, RGB or CMYK to DeviceN
// RGB data is inverted.

ImageSource_ToDeviceN::ImageSource_ToDeviceN(ImageSource *source) : ImageSource(source), source(source)
{
	if(STRIP_ALPHA(source->type)==IS_TYPE_RGB)
	{
		Debug[COMMENT] << "Original image is RGB - inverting as well as re-interpreting..." << endl;
		MakeRowBuffer();
	}
	type=IS_TYPE_DEVICEN;
}

ImageSource_ToDeviceN::~ImageSource_ToDeviceN()
{
	if(source)
		delete source;
}

ISDataType *ImageSource_ToDeviceN::GetRow(int row)
{
	ISDataType *src=source->GetRow(row);
	switch(source->type)
	{
		case IS_TYPE_RGB:
			if(row==currentrow)
				return(rowbuffer);

			for(int x=0;x<width*samplesperpixel;++x)
			{
				rowbuffer[x]=IS_SAMPLEMAX-src[x];
			}
			currentrow=row;
			return(rowbuffer);
			break;
		case IS_TYPE_RGBA:
			if(row==currentrow)
				return(rowbuffer);

			for(int x=0;x<width;++x)
			{
				for(int s=0;s<samplesperpixel-1;++s)
					rowbuffer[x*samplesperpixel+s]=IS_SAMPLEMAX-src[x*samplesperpixel+s];
				rowbuffer[x*samplesperpixel+samplesperpixel-1]=src[x*samplesperpixel+samplesperpixel-1];
			}
			currentrow=row;
			return(rowbuffer);
			break;
		default:
			return(src);
			break;
	}
}


// Forces an ImageSource from DeviceN to RGB, inverting the data as it goes.
// NOTE this *doesn't* remap the channels or downrender to RGB - it merely
// re-interprets the data as RGB.
// Alpha channel, if present, is not inverted.

ImageSource_DeviceNToRGB::ImageSource_DeviceNToRGB(ImageSource *source) : ImageSource(source), source(source)
{
	switch(samplesperpixel)
	{
		case 3:
			type=IS_TYPE_RGB;
			break;
		case 4:
			type=IS_TYPE_RGBA;
			break;
		default:
			throw "Can't reinterpret DeviceN image as RGB - wrong number of channels!";
	}
	MakeRowBuffer();
}


ImageSource_DeviceNToRGB::~ImageSource_DeviceNToRGB()
{
	if(source)
		delete(source);
}


ISDataType *ImageSource_DeviceNToRGB::GetRow(int row)
{
	if(row==currentrow)
		return(rowbuffer);

	ISDataType *src=source->GetRow(row);

	switch(source->type)
	{
		case IS_TYPE_RGB:
			for(int x=0;x<width*samplesperpixel;++x)
			{
				rowbuffer[x]=IS_SAMPLEMAX-src[x];
			}
			break;
		case IS_TYPE_RGBA:
			for(int x=0;x<width;++x)
			{
				for(int s=0;s<samplesperpixel-1;++s)
					rowbuffer[x*samplesperpixel+s]=IS_SAMPLEMAX-src[x*samplesperpixel+s];
				rowbuffer[x*samplesperpixel+samplesperpixel-1]=src[x*samplesperpixel+samplesperpixel-1];
			}
			break;
		default:
			break;
	}
	currentrow=row;
	return(rowbuffer);
}


// Forces an ImageSource from DeviceN to CMYK
// NOTE this *doesn't* remap the channels or downrender to CMYK - it merely
// re-interprets the data as CMYK.

ImageSource_DeviceNToCMYK::ImageSource_DeviceNToCMYK(ImageSource *source) : ImageSource(source), source(source)
{
	switch(samplesperpixel)
	{
		case 4:
			type=IS_TYPE_CMYK;
			break;
		case 5:
			type=IS_TYPE_CMYKA;
			break;
		default:
			throw "Can't reinterpret DeviceN image as RGB - wrong number of channels!";
	}
	MakeRowBuffer();
}


ImageSource_DeviceNToCMYK::~ImageSource_DeviceNToCMYK()
{
	if(source)
		delete(source);
}


ISDataType *ImageSource_DeviceNToCMYK::GetRow(int row)
{
	return(source->GetRow(row));
}

