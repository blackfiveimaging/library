#include <iostream>

#include "profilemanager/lcmswrapper.h"
#include "support/debug.h"

#include "cachedimage.h"

using namespace std;

CachedImage_Deferred::CachedImage_Deferred(RefCountPtr<ImageSource> source)
	: ImageSink(source), width(source->width), height(source->height),
	samplesperpixel(source->samplesperpixel), type(source->type), embeddedprofile(NULL), parasites(source->GetParasites()),
	xres(source->xres), yres(source->yres)
{
	Debug[TRACE] << "In CachedImage_Deferred constructor" << endl;
	Debug[TRACE] << "Image type: " << type << ", width: " << width << ", height: " << height << endl;
	Debug[TRACE] << "(" << source->type << ")" << endl;
	try
	{
		imagedata=new ISDataType[width*height*samplesperpixel];
	}
	catch (bad_alloc&)
	{
		throw "Can't allocate pixel buffer";
	}
	embeddedprofile=source->GetEmbeddedProfile();
	Debug[TRACE] << "CachedImage: embeddedprofile set to " << long(&*embeddedprofile) << std::endl;
}


CachedImage_Deferred::~CachedImage_Deferred()
{
	if(imagedata)
		delete[] imagedata;
}


void CachedImage_Deferred::ProcessRow(int row)
{
	ISDataType *srcdata=source->GetRow(row);
	ISDataType *dstdata=GetRow(row);
	int spr=width*samplesperpixel;
	for(int s=0;s<spr;++s)
		dstdata[s]=srcdata[s];
}


ISDataType *CachedImage_Deferred::GetRow(int row)
{
	if(row>=height)
		row=height-1;
	if(row<0)
		row=0;
	return(imagedata+row*width*samplesperpixel);
}


ISDeviceNValue CachedImage_Deferred::GetPixel(int x, int y)
{
	ISDeviceNValue result(samplesperpixel);
	ISDataType *row=GetRow(y);
	if(x<0)
		x=0;
	if(x>=width)
		x=width-1;
	row+=x*samplesperpixel;
	for(int s=0;s<samplesperpixel;++s)
		result[s]=row[s];
	return(result);
}


ImageSource *CachedImage_Deferred::GetImageSource()
{
	return(new ImageSource_CachedImage(this));
}


// ImageSource_CachedImage


ImageSource_CachedImage::ImageSource_CachedImage(CachedImage_Deferred *img) : ImageSource(), image(img)
{
	width=img->width;
	height=img->height;
	xres=img->xres;
	yres=img->yres;
	samplesperpixel=img->samplesperpixel;
	type=img->type;
	randomaccess=true;
	if(img->embeddedprofile)
	{
		SetEmbeddedProfile(img->embeddedprofile);
	}
	parasites=img->parasites;
}


ImageSource_CachedImage::~ImageSource_CachedImage()
{
}


ISDataType *ImageSource_CachedImage::GetRow(int row)
{
	return(image->GetRow(row));
}

