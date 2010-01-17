#ifndef IMAGESAVER_H
#define IMAGESAVER_H

#include "progress.h"

// Base class interface for saving images

class ImageSaver
{
	public:
	ImageSaver() : progress(NULL)
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
		throw "Save() should be overridden by subclass!";
	}
	protected:
	Progress *progress;
};


#endif
