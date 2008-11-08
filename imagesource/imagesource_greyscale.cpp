/*
 * imagesource_greyscale.cpp
 *
 * Supports RGB data
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

#include "imagesource_greyscale.h"

using namespace std;

ImageSource_Greyscale::~ImageSource_Greyscale()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_Greyscale::GetRow(int row)
{
	int i;

	if(row==currentrow)
		return(rowbuffer);

	ISDataType *srcdata=source->GetRow(row);

	switch(samplesperpixel)
	{
		case 1:
			for(i=0;i<width;++i)
			{
				int a=(srcdata[i*3]+srcdata[i*3+1]+srcdata[i*3+2])/3;
				rowbuffer[i]=a;
			}
			break;
		case 2:
			for(i=0;i<width;++i)
			{
				int a=(srcdata[i*4]+srcdata[i*4+1]+srcdata[i*4+2])/3;
				rowbuffer[i*2]=a;
				rowbuffer[i*2+1]=srcdata[i*4+3];
			}
			break;
	}

	currentrow=row;

	return(rowbuffer);
}


ImageSource_Greyscale::ImageSource_Greyscale(struct ImageSource *source)
	: ImageSource(source), source(source)
{
	switch(type)
	{
		case IS_TYPE_RGB:
			type=IS_TYPE_GREY;
			samplesperpixel=1;
			break;
		case IS_TYPE_RGBA:
			type=IS_TYPE_GREYA;
			samplesperpixel=2;
			break;
		default:
			throw "ImageSource_Greyscale - Unsupported type - only RGB is currently supported";
			break;
	}
	MakeRowBuffer();
}
