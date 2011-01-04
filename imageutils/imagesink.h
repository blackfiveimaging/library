// ImageSink - base class for classes that "consume" image data.
// Savers fall into this category, as does the ImageCache class.
// The advantage of using this interface is that multiple sinks
// can be made to operate on a single ImageSource in sync - such as
// a utility to decompose a CMYK image as four individual greyscale planes.
//
// When subclassing, override ProcessRow() at a minimum.
//
// When using a subclass, call ProcessRow() once per row, and call Finish() at the end.
// (By default Finish() releases the reference count pointer.)

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
		if(!source)
			throw "ImageSink::ProcessImage.source is NULL";
		// Displaying the progress meter can be expensive,
		// so we only update it often enough to reflect single
		// percentage steps.
		int progressmodulo=source->height/100;
		if(progressmodulo==0) progressmodulo=1;

		bool cont=true;

		for(int row=0;row<source->height && cont==true;++row)
		{
			ProcessRow(row);
			if((row % progressmodulo)==0 && prog)
				cont=prog->DoProgress(row,source->height);
		}
		Finish();
		return(cont);
	}
	virtual void ProcessRow(int row)
	{
	}
	virtual void Finish()
	{
		source=NULL;	// Release reference when done.
	}
	protected:
	RefCountPtr<ImageSource> source;
};

#endif
