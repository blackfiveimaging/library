// ImageSink - base class for classes that "consume" image data.
// Savers fall into this category, as does the ImageCache class.
// The advantage of using this interface is that multiple sinks
// can be made to operate on a single ImageSource in sync - such as
// a utility to decompose a CMYK image as four individual greyscale planes.

#ifndef IMAGESINK_H
#define IMAGESINK_H

#include "debug.h"
#include "imagesource.h"
#include "progress.h"
#include "refcountptr.h"

class ImageSink
{
	public:
	ImageSink(RefCountPtr<ImageSource> source) : source(source)
	{
	}
	virtual ~ImageSink()
	{
	}
	virtual bool ProcessImage(Progress *prog=NULL)
	{
		for(int i=0;i<source->height;++i)
		{
			ProcessRow(i);
			if(prog)
			{
				if(!prog->DoProgress(i,source->height-1))
				return(false);
			}
		}
		return(true);
	}
	virtual void ProcessRow(int row)
	{
	}
	protected:
	RefCountPtr<ImageSource> source;
};

#endif
