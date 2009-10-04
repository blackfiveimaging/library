/* 
 * imagesource_Rainbow.cpp - renders a chequerboard pattern.
 * Supports Random Access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


#include <iostream>

#include "imagesource_rainbowsweep.h"

ISDataType *ImageSource_RainbowSweep::GetRow(int row)
{
	if(currentrow==row)
		return(rowbuffer);
	double a=row;
	a/=height;

	for(int x=0;x<width;++x)
	{
		int c=(x*1024)/width;
		int r=c-128;
		if(r>512) r-=1024;
		if(r<0) r=-r;
		r=384-r;
		if(r<0) r=0;
		if(r>255) r=255;

		int g=c-384;
		if(g<0) g=-g;
		g=384-g;
		if(g<0) g=0;
		if(g>255) g=255;

		int b=c-768;
		if(b<0) b=-b;
		b=384-b;
		if(b<0) b=0;
		if(b>255) b=255;

		r=128*a+r*(1-a);
		g=128*a+g*(1-a);
		b=128*a+b*(1-a);

		rowbuffer[x*3]=EIGHTTOIS(r);
		rowbuffer[x*3+1]=EIGHTTOIS(g);
		rowbuffer[x*3+2]=EIGHTTOIS(b);
	}
	
	currentrow=row;
	return(rowbuffer);
}


ImageSource_RainbowSweep::ImageSource_RainbowSweep(int width,int height)
	: ImageSource(width,height,IS_TYPE_RGB)
{
	randomaccess=true;
	MakeRowBuffer();
}

ImageSource_RainbowSweep::~ImageSource_RainbowSweep()
{
}

