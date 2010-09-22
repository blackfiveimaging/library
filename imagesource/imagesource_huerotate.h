#ifndef IMAGESOURCE_HUEROTATE_H
#define IMAGESOURCE_HUEROTATE_H

// ImageSource class to rotate an image's hue by a specified number of degrees.
// Only supports HSV or HSVA input

// Copyright (c) 2010 Alastair M. Robinson
// Released under the terms of the GNU General Public License -
// See the file "COPYING" for more details.

#include "imagesource.h"

class ImageSource_HueRotate : public ImageSource
{
	public:
	ImageSource_HueRotate(ImageSource *src,int degrees) : ImageSource(src), source(src), offset(0)
	{
		if(STRIP_ALPHA(type)!=IS_TYPE_HSV)
			throw "Hue rotation only works with HSV data!";
		offset=(0xc000*degrees)/360;
		MakeRowBuffer();
	}
	~ImageSource_HueRotate()
	{
		if(source)
			delete source;
	}
	ISDataType *GetRow(int row)
	{
		if(row==currentrow)
			return(rowbuffer);
		ISDataType *src=source->GetRow(row);
		switch(type)
		{
			case IS_TYPE_HSV:
				for(int x=0;x<width;++x)
				{
					int h=src[x*samplesperpixel];
					int s=src[x*samplesperpixel+1];
					int v=src[x*samplesperpixel+2];
					h+=offset;
					if(h>=0xc000)
						h-=0xc000;
					rowbuffer[x*samplesperpixel]=h;
					rowbuffer[x*samplesperpixel+1]=s;
					rowbuffer[x*samplesperpixel+2]=v;
				}
				break;
			case IS_TYPE_HSVA:
				for(int x=0;x<width;++x)
				{
					int h=src[x*samplesperpixel];
					int s=src[x*samplesperpixel+1];
					int v=src[x*samplesperpixel+2];
					int a=src[x*samplesperpixel+3];
					h+=offset;
					if(h>=0xc000)
						h-=0xc000;
					rowbuffer[x*samplesperpixel]=h;
					rowbuffer[x*samplesperpixel+1]=s;
					rowbuffer[x*samplesperpixel+2]=v;
					rowbuffer[x*samplesperpixel+3]=a;
				}
				break;
				break;
			default:
				throw "ImageSource_HueRotate: Bad image type!";
				break;
		}
		currentrow=row;
		return(rowbuffer);
	}
	protected:
	ImageSource *source;
	int offset;
};

#endif

