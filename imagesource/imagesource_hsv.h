#ifndef IMAGESOURCE_HSV
#define IMAGESOURCE_HSV

// ImageSource class to convert RGB data to HSV.

// Copyright (c) 2010 Alastair M. Robinson
// Released under the terms of the GNU General Public License -
// See the file "COPYING" for more details.

#include "imagesource.h"

class ImageSource_HSV : public ImageSource
{
	public:
	ImageSource_HSV(ImageSource *src) : ImageSource(src), source(src)
	{
		switch(source->type)
		{
			case IS_TYPE_RGB:
				type=IS_TYPE_HSV;
				break;
			case IS_TYPE_RGBA:
				type=IS_TYPE_HSVA;
				break;
			default:
				throw "HSV conversion can only take RGB(A) input";
				break;
		}
		MakeRowBuffer();
	}
	~ImageSource_HSV()
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
					int r=src[3*x];
					int g=src[3*x+1];
					int b=src[3*x+2];
					int M=r;
					if(M<g) M=g;
					if(M<b) M=b;

					int m=r;
					if(m>g) m=g;
					if(m>b) m=b;

					int c=M-m;
					int h=0;
					if(c)
					{
						if(M==r)
							h=(((g-b)*8192)/c);
						if(M==g)
							h=(((b-r)*8192)/c)+16384;
						if(M==b)
							h=(((r-g)*8192)/c)+32768;
						if(h<0)
							h+=(6*8192);
					}

					int s=0;
					if(M>4)
						s=((IS_SAMPLEMAX/4)*c)/(M/4);
					if(s>IS_SAMPLEMAX)
						s=IS_SAMPLEMAX;
					rowbuffer[3*x]=h;
					rowbuffer[3*x+1]=s;
					rowbuffer[3*x+2]=M;
				}
				break;
			case IS_TYPE_HSVA:
				for(int x=0;x<width;++x)
				{
					int r=src[4*x];
					int g=src[4*x+1];
					int b=src[4*x+2];
					int a=0;
					int M=r;
					if(M<g) M=g;
					if(M<b) M=b;

					int m=r;
					if(m>g) m=g;
					if(m>b) m=b;

					int c=M-m;
					int h=0;
					if(c)
					{
						if(M==r)
							h=(((g-b)*8192)/c);
						if(M==g)
							h=(((b-r)*8192)/c)+16384;
						if(M==b)
							h=(((r-g)*8192)/c)+32768;
						if(h<0)
							h+=(6*8192);
					}

					int s=0;
					if(M>4)
						s=((IS_SAMPLEMAX/4)*c)/(M/4);
					rowbuffer[4*x]=h;
					rowbuffer[4*x+1]=s;
					rowbuffer[4*x+2]=M;
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

