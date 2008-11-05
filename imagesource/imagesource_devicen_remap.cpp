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

	// For each pixel we remap each source channel to a channe in the rowbuffer, using the translation table.
	if(HAS_ALPHA(type))
	{
		for(int x=0;x<width;++x)
		{
			for(int s=0;s<source->samplesperpixel-1;++s)
			{
				if(table[s]>-1)
					rowbuffer[x*samplesperpixel+table[s]]=srcdata[x*source->samplesperpixel+s];
			}
			rowbuffer[x*samplesperpixel+source->samplesperpixel-1]=srcdata[x*samplesperpixel+source->samplesperpixel-1];
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
	for(int i=0;i<source->samplesperpixel;++i)
	{
		table[i]=maptable[i];
		if(table[i]>=samplesperpixel)
			throw "Error: not enough output channels for remapping table";
	}
	MakeRowBuffer();
}

