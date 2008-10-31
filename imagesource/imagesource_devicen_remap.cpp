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

	ISDataType *srcdata=source->GetRow(row);

	if(HAS_ALPHA(type))
	{
		for(int x=0;x<width;++x)
		{
			for(int s=0;s<source->samplesperpixel-1;++s)
			{
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
				rowbuffer[x*samplesperpixel+table[s]]=srcdata[x*source->samplesperpixel+s];
			}
		}
	}
	currentrow=row;

	return(rowbuffer);
}


ImageSource_DeviceN_Remap::ImageSource_DeviceN_Remap(struct ImageSource *source,const int *maptable)
	: ImageSource(source), source(source)
{
	table=(int *)malloc(sizeof(int)*source->samplesperpixel);
	for(int i=0;i<source->samplesperpixel;++i)
	{
		table[i]=maptable[i];
	}
	MakeRowBuffer();
}

