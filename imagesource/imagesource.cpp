#include <iostream>

#include "lcmswrapper.h"
#include "imagesource.h"

using namespace std;

ImageSource::ImageSource() : embeddedprofile(NULL), embprofowned(false), rowbuffer(NULL)
{
	type=IS_TYPE_RGB;
	samplesperpixel=3;
	randomaccess=false;
	xres=72.0;
	yres=72.0;
	currentrow=-1;
}


ImageSource::ImageSource(int width, int height, IS_TYPE type)
	: width(width), height(height), type(type), embeddedprofile(NULL), embprofowned(false), rowbuffer(NULL)
{
	switch(type)
	{
		case IS_TYPE_GREY:
			samplesperpixel=1;
			break;
		case IS_TYPE_GREYA:
			samplesperpixel=2;
			break;
		case IS_TYPE_RGB:
		case IS_TYPE_LAB:
			samplesperpixel=3;
			break;
		case IS_TYPE_CMYK:
		case IS_TYPE_RGBA:
		case IS_TYPE_LABA:
			samplesperpixel=4;
			break;
		case IS_TYPE_CMYKA:
			samplesperpixel=5;
			break;
		default:
			samplesperpixel=0;
			break;
	}
	randomaccess=false;
	xres=72.0;
	yres=72.0;
	currentrow=-1;
}


ImageSource::ImageSource(ImageSource *src) : embprofowned(false), rowbuffer(NULL)
{
	width=src->width;
	height=src->height;
	type=src->type;
	samplesperpixel=src->samplesperpixel;
	xres=src->xres;
	yres=src->yres;
	randomaccess=src->randomaccess;
	embeddedprofile=src->embeddedprofile;
	currentrow=-1;
}


ImageSource::~ImageSource()
{
	if(rowbuffer)
		free(rowbuffer);
	if(embeddedprofile && embprofowned)
		delete embeddedprofile;
}


void ImageSource::MakeRowBuffer()
{
	rowbuffer=(ISDataType *)malloc(sizeof(ISDataType)*width*samplesperpixel);
	currentrow=-1;
}


void ImageSource::SetResolution(double xr,double yr)
{
	xres=xr;
	yres=yr;
}


void ImageSource::SetEmbeddedProfile(CMSProfile *profile,bool owned)
{
	if(embeddedprofile && embprofowned)
		delete embeddedprofile;
	embeddedprofile=profile;
	embprofowned=owned;
}

