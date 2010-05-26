#include <iostream>

#include "profilemanager/lcmswrapper.h"
#include "support/debug.h"

#include "cachedimage.h"

using namespace std;

CachedImage_Deferred::CachedImage_Deferred(ImageSource *source)
	: source(source), width(source->width), height(source->height),
	samplesperpixel(source->samplesperpixel), type(source->type), embeddedprofile(NULL),
	xres(source->xres), yres(source->yres)
{
	Debug[TRACE] << "In CachedImage_Deferred constructor" << endl;
	Debug[TRACE] << "Image type: " << type << ", width: " << width << ", height: " << height << endl;
	Debug[TRACE] << "(" << source->type << ")" << endl;
	imagedata=new ISDataType[width*height*samplesperpixel];
	CMSProfile *prof=source->GetEmbeddedProfile();
	if(prof)
		embeddedprofile=new CMSProfile(*prof);
}


CachedImage_Deferred::~CachedImage_Deferred()
{
	if(imagedata)
		delete[] imagedata;
	if(source);
		delete source;
	if(embeddedprofile)
		delete embeddedprofile;
}


void CachedImage_Deferred::ReadImage(Progress *prog)
{
	Debug[TRACE] << "CachedImage: ReadImage()" << endl;
	if(!source)
		throw "CachedImage_Deferred::ReadImage - source is NULL";

	// Displaying the progress meter can be expensive,
	// so we only update it often enough to reflect single
	// percentage steps.
	int progressmodulo=height/100;
	if(progressmodulo==0) progressmodulo=1;

	bool cont=true;

	for(int row=0;row<height && cont==true;++row)
	{
		ReadRow(row);
		if((row % progressmodulo)==0 && prog)
			cont=prog->DoProgress(row,height);
	}
	if(source)
		delete source;
	source=NULL;
}


void CachedImage_Deferred::ReadRow(int row)
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
	cerr << "Image type: " << type << endl;
	randomaccess=true;
	if(img->embeddedprofile)
	{
		CMSProfile *prof=new CMSProfile(*img->embeddedprofile);
		SetEmbeddedProfile(prof,true);
	}
}


ImageSource_CachedImage::~ImageSource_CachedImage()
{
}


ISDataType *ImageSource_CachedImage::GetRow(int row)
{
	return(image->GetRow(row));
}

