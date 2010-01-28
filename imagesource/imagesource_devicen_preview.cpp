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

#include "../support/util.h"

#include "imagesource_devicen_preview.h"

using namespace std;


ISDeviceN_Colorant_Preview::ISDeviceN_Colorant_Preview()
	: red(0), green(0), blue(0), longname(NULL), alias(0)
{
}


ISDeviceN_Colorant_Preview::ISDeviceN_Colorant_Preview(const char *longname,int red, int green, int blue, char alias)
	: red(EIGHTTOIS(red)),green(EIGHTTOIS(green)),blue(EIGHTTOIS(blue)),longname(NULL),alias(alias)
{
	if(longname)
		this->longname=strdup(longname);
}


ISDeviceN_Colorant_Preview::ISDeviceN_Colorant_Preview(const char *longname)
	: red(EIGHTTOIS(red)),green(EIGHTTOIS(green)),blue(EIGHTTOIS(blue)),longname(NULL),alias(alias)
{
	if(StrcasecmpIgnoreSpaces(longname,"Cyan")==0)
	{
		red=EIGHTTOIS(0); green=EIGHTTOIS(190); blue=EIGHTTOIS(255); alias='C';
	}
	else if(StrcasecmpIgnoreSpaces(longname,"Magenta")==0)
	{
		red=EIGHTTOIS(255); green=EIGHTTOIS(0); blue=EIGHTTOIS(190); alias='M';
	}
	else if(StrcasecmpIgnoreSpaces(longname,"Vivid Magenta")==0)
	{
		red=EIGHTTOIS(255); green=EIGHTTOIS(0); blue=EIGHTTOIS(190); alias='M';
	}
	else if(StrcasecmpIgnoreSpaces(longname,"Yellow")==0)
	{
		red=EIGHTTOIS(255); green=EIGHTTOIS(255); blue=EIGHTTOIS(0); alias='Y';
	}
	else if(StrcasecmpIgnoreSpaces(longname,"Black")==0)
	{
		red=EIGHTTOIS(0); green=EIGHTTOIS(0); blue=EIGHTTOIS(0); alias='K';
	}
	else if(StrcasecmpIgnoreSpaces(longname,"Photo Black")==0)
	{
		red=EIGHTTOIS(0); green=EIGHTTOIS(0); blue=EIGHTTOIS(0); alias='K';
	}
	else if(StrcasecmpIgnoreSpaces(longname,"Matte Black")==0)
	{
		red=EIGHTTOIS(0); green=EIGHTTOIS(0); blue=EIGHTTOIS(0); alias='T';
	}
	else if(StrcasecmpIgnoreSpaces(longname,"Light Cyan")==0)
	{
		red=EIGHTTOIS(127); green=EIGHTTOIS(220); blue=EIGHTTOIS(255); alias='c';
	}
	else if(StrcasecmpIgnoreSpaces(longname,"Light Magenta")==0)
	{
		red=EIGHTTOIS(255); green=EIGHTTOIS(127); blue=EIGHTTOIS(220); alias='m';
	}
	else if(StrcasecmpIgnoreSpaces(longname,"Vivid Light Magenta")==0)
	{
		red=EIGHTTOIS(255); green=EIGHTTOIS(127); blue=EIGHTTOIS(220); alias='m';
	}
	else if(StrcasecmpIgnoreSpaces(longname,"Light Black")==0)
	{
		red=EIGHTTOIS(127); green=EIGHTTOIS(127); blue=EIGHTTOIS(127); alias='k';
	}
	else if(StrcasecmpIgnoreSpaces(longname,"Red")==0)
	{
		red=EIGHTTOIS(255); green=EIGHTTOIS(0); blue=EIGHTTOIS(0); alias='R';
	}
	else if(StrcasecmpIgnoreSpaces(longname,"Blue")==0)
	{
		red=EIGHTTOIS(0); green=EIGHTTOIS(0); blue=EIGHTTOIS(255); alias='B';
	}
	else
		throw "Unknown colorant";
	if(longname)
		this->longname=strdup(longname);
}


ISDeviceN_Colorant_Preview::ISDeviceN_Colorant_Preview(const ISDeviceN_Colorant_Preview &other)
	: red(other.red), green(other.green), blue(other.blue), longname(NULL), alias(other.alias)
{
	if(other.longname)
		longname=strdup(other.longname);
}


ISDeviceN_Colorant_Preview::~ISDeviceN_Colorant_Preview()
{
	if(longname)
		free(longname);
}


ISDeviceN_Colorant_Preview &ISDeviceN_Colorant_Preview::operator=(const ISDeviceN_Colorant_Preview &other)
{
	if(longname)
		free(longname);
	if(other.longname)
		longname=strdup(other.longname);
	red=other.red;
	green=other.green;
	blue=other.blue;
	alias=other.alias;
	return(*this);
}

//////////////////////////////////////////////////////////////////////////////

class ISDeviceNPreview_Colorant
{
	public:
	ISDataType red,green,blue;
};

//////////////////////////////////////////////////////////////////////////////


ImageSource_DeviceN_Preview::~ImageSource_DeviceN_Preview()
{
	if(source)
		delete source;

	if(colorants)
		delete[] colorants;
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


ImageSource_DeviceN_Preview::ImageSource_DeviceN_Preview(struct ImageSource *source,DeviceNColorantList *cols,int firstcolorant)
	: ImageSource(source), source(source), colorants(NULL)
{
	int sourcespp=source->samplesperpixel;
	if(HAS_ALPHA(type))
	{
		type=IS_TYPE_RGBA;
		samplesperpixel=4;
		--sourcespp;
	}
	else
	{
		type=IS_TYPE_RGB;
		samplesperpixel=3;
	}
	int c=cols->GetColorantCount();
	if((c-firstcolorant)<sourcespp)
		throw "ISDeviceN: Not enough colorants provided!";
	colorants=new ISDeviceNPreview_Colorant[c];
	for(int i=0;i<(c-firstcolorant);++i)
	{
		DeviceNColorant *col=(*cols)[i+firstcolorant];
		colorants[i].red=EIGHTTOIS(col->red);
		colorants[i].green=EIGHTTOIS(col->green);
		colorants[i].blue=EIGHTTOIS(col->blue);
	}
	MakeRowBuffer();
}

