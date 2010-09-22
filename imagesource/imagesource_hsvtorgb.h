#ifndef IMAGESOURCE_HSVTORGB
#define IMAGESOURCE_HSVTORGB

// ImageSource class to convert HSV data to RGB.

// Copyright (c) 2010 Alastair M. Robinson
// Released under the terms of the GNU General Public License -
// See the file "COPYING" for more details.

#include "imagesource.h"

class ImageSource_HSVToRGB : public ImageSource
{
	public:
	ImageSource_HSVToRGB(ImageSource *src) : ImageSource(src), source(src)
	{
		switch(source->type)
		{
			case IS_TYPE_HSV:
				type=IS_TYPE_RGB;
				break;
			case IS_TYPE_HSVA:
				type=IS_TYPE_RGBA;
				break;
			default:
				throw "HSV conversion only works with HSV or HSVA input!";
				break;
		}
		MakeRowBuffer();
	}
	~ImageSource_HSVToRGB()
	{
		if(source)
			delete source;
	}
	ISDataType *GetRow(int row)
	{
		if(row==currentrow)
			return(rowbuffer);
		ISDataType *src=source->GetRow(row);

		switch(source->type)
		{
			case IS_TYPE_HSV:
				for(int x=0;x<width;++x)
				{
					int h=src[3*x];
					int s=src[3*x+1];
					int v=src[3*x+2];

					int c=((s/2)*(v/2))/(IS_SAMPLEMAX/4);

					int r=0;
					int g=0;
					int b=0;
					int t=(h % 0x4000) - 0x2000;
					if(t<0)
						t=-t;
					t=(c*(0x2000-t))/0x2000;

					if(h<0x2000)
					{
						r=c; g=t; b=0;
					}
					else if(h<0x4000)
					{
						r=t; g=c; b=0;
					}
					else if(h<0x6000)
					{
						r=0; g=c; b=t;
					}
					else if(h<0x8000)
					{
						r=0; g=t; b=c;
					}
					else if(h<0xa000)
					{
						r=t; g=0; b=c;
					}
					else if(h<0xc000)
					{
						r=c; g=0; b=t;
					}
					r+=v-c;
					g+=v-c;
					b+=v-c;
					if(r>IS_SAMPLEMAX) r=IS_SAMPLEMAX;
					if(r<0) r=0;
					if(g>IS_SAMPLEMAX) g=IS_SAMPLEMAX;
					if(g<0) g=0;
					if(b>IS_SAMPLEMAX) b=IS_SAMPLEMAX;
					if(b<0) b=0;
					rowbuffer[3*x]=r;
					rowbuffer[3*x+1]=g;
					rowbuffer[3*x+2]=b;
				}
				break;
			case IS_TYPE_HSVA:
				for(int x=0;x<width;++x)
				{
					int h=src[4*x];
					int s=src[4*x+1];
					int v=src[4*x+2];
					int a=src[4*x+3];

					int c=((s/2)*(v/2))/(IS_SAMPLEMAX/4);

					int r=0;
					int g=0;
					int b=0;
					int t=(h % 0x4000) - 0x2000;
					if(t<0)
						t=-t;
					t=(c*(0x2000-t))/0x2000;

					if(h<0x2000)
					{
						r=c; g=t; b=0;
					}
					else if(h<0x4000)
					{
						r=t; g=c; b=0;
					}
					else if(h<0x6000)
					{
						r=0; g=c; b=t;
					}
					else if(h<0x8000)
					{
						r=0; g=t; b=c;
					}
					else if(h<0xa000)
					{
						r=t; g=0; b=c;
					}
					else if(h<0xc000)
					{
						r=c; g=0; b=t;
					}
					r+=v-c;
					g+=v-c;
					b+=v-c;
					if(r>IS_SAMPLEMAX) r=IS_SAMPLEMAX;
					if(r<0) r=0;
					if(g>IS_SAMPLEMAX) g=IS_SAMPLEMAX;
					if(g<0) g=0;
					if(b>IS_SAMPLEMAX) b=IS_SAMPLEMAX;
					if(b<0) b=0;
					rowbuffer[4*x]=r;
					rowbuffer[4*x+1]=g;
					rowbuffer[4*x+2]=b;
					rowbuffer[4*x+3]=a;
				}
				break;
			default:
				throw "Bad type - how did that happen?";
				break;
		}
		currentrow=row;
		return(rowbuffer);
	}
	protected:
	ImageSource *source;
};

#endif

