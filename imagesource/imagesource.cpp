#include <iostream>

#include "lcmswrapper.h"
#include "imagesource.h"

using namespace std;

ImageSource::ImageSource() : rowbuffer(NULL)
{
	type=IS_TYPE_RGB;
	samplesperpixel=3;
	randomaccess=false;
	xres=72.0;
	yres=72.0;
	currentrow=-1;
}


ImageSource::ImageSource(int width, int height, IS_TYPE type)
	: width(width), height(height), type(type), rowbuffer(NULL)
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


ImageSource::ImageSource(ImageSource *src) : embeddedprofile(src->embeddedprofile), parasites(src->parasites), rowbuffer(NULL)
{
	width=src->width;
	height=src->height;
	type=src->type;
	samplesperpixel=src->samplesperpixel;
	xres=src->xres;
	yres=src->yres;
	randomaccess=src->randomaccess;
	Debug[TRACE] << "Adopted other source's embedded profile... ( " << long(embeddedprofile.GetPtr()) << std::endl;
	currentrow=-1;
}


ImageSource::~ImageSource()
{
	if(rowbuffer)
		free(rowbuffer);
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


void ImageSource::SetEmbeddedProfile(RefCountPtr<CMSProfile> profile)
{
	embeddedprofile=profile;
	Debug[WARN] << "SetEmbeddedProfile - from smart pointer, to " << long(embeddedprofile.GetPtr()) << std::endl;
}


void ImageSource::SetEmbeddedProfile(CMSProfile *profile)
{
	embeddedprofile=RefCountPtr<CMSProfile>(profile);
	Debug[WARN] << "SetEmbeddedProfile - from regular pointer, to " << long(profile) << std::endl;
}


RefCountPtr<ISParasite> ImageSource::GetParasite(ISParasiteType type,ISParasiteApplicability applic)
{
	Debug[TRACE] << "*** Hunting for parasite " << type << ", applic: " << applic << std::endl;
	RefCountPtr<ISParasite> p=parasites[type];
	if(p && p->Applicable(applic))
		return(p);
	Debug[TRACE] << "*** Not found" << std::endl;
	p=NULL;
	return(p);
}

