#ifndef CACHEDIMAGE_H
#define CACHEDIMAGE_H

#include <iostream>

#include "progress.h"
#include "imagesource/imagesource.h"

// CachedImage_Deferred - the base class for cached images.  Sets up the width, height, type, etc.
// and allocated storage, but doesn't actually read the data from the ImageSource until asked.
// ReadImage() reads and caches the entire image, disposing of the source ImageSource when done.
// ReadRow() reads and caches a single row.
// Generally you won't use this except with ImageSource_Tee.

class CachedImage_Deferred
{
	public:
	CachedImage_Deferred(ImageSource *source);
	virtual ~CachedImage_Deferred();
	virtual void ReadImage(Progress *prog=NULL);
	virtual void ReadRow(int row);
	virtual ISDataType *GetRow(int row);
	virtual ImageSource *GetImageSource();
	virtual ISDeviceNValue GetPixel(int x, int y);
	protected:
	ImageSource *source;
	int width, height;
	int samplesperpixel;
	IS_TYPE type;
	ISDataType *imagedata;
	RefCountPtr<CMSProfile> embeddedprofile;
	double xres,yres;
	friend class ImageSource_CachedImage;
	friend class ImageSource_Tee;
};


// Convenience subclass, this variant completely builds and processes the Cached Image immediately.

class CachedImage : public CachedImage_Deferred
{
	public:
	CachedImage(ImageSource *src, Progress *prog=NULL) : CachedImage_Deferred(src)
	{
		ReadImage(prog);
	}
	virtual ~CachedImage()
	{
	}
	protected:
};



// The GetImageSource member function of the CachedImage classes will return one of these.
// Unlike most imagesources, this one claims no ownership of the source data, so you can 
// have as many operating concurrently and as many times on the Cached image as you like.

class ImageSource_CachedImage : public ImageSource
{
	public:
	ImageSource_CachedImage(CachedImage_Deferred *img);
	~ImageSource_CachedImage();
	ISDataType *GetRow(int row);
	protected:
	CachedImage_Deferred *image;
};

#endif

