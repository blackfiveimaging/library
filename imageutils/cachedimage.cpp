#include <iostream>

#include "cachedimage.h"

using namespace std;

CachedImage_Deferred::CachedImage_Deferred(ImageSource *source) : source(source)
{
	width=source->width;
	height=source->height;
	samplesperpixel=source->samplesperpixel;

	imagedata=new ISDataType[width*height*samplesperpixel];
}


CachedImage_Deferred::~CachedImage_Deferred()
{
	if(imagedata)
		delete[] imagedata;
	if(source);
		delete source;
}


void CachedImage_Deferred::ReadImage()
{
	for(int row=0;row<height;++row)
		ReadRow(row);
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
		row=height;
	if(row<0)
		row=0;
	return(imagedata+row*width*samplesperpixel);
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
	samplesperpixel=img->samplesperpixel;
}


ImageSource_CachedImage::~ImageSource_CachedImage()
{
}


ISDataType *ImageSource_CachedImage::GetRow(int row)
{
	return(image->GetRow(row));
}

