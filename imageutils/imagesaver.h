#ifndef IMAGESAVER_H
#define IMAGESAVER_H

#include "progress.h"
#include "imagesink.h"

// Base class interface for saving images

class ImageSaver : public ImageSink
{
	public:
	ImageSaver(RefCountPtr<ImageSource> is) : ImageSink(is), progress(NULL)
	{
	}
	virtual ~ImageSaver()
	{
	}
	virtual void SetProgress(Progress *p)
	{
		progress=p;
	}
	virtual void Save()
	{
		// Compatiblity function
		ProcessImage(progress);
	}
	protected:
	Progress *progress;
};


#endif
