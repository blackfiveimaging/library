// An ImageSource that serves no purpose other than to allow multiple references
// to another ImageSource.
// Note that if rows are going to be requested out of order from various other filters, then source must support random access.
// If you know how many rows of overlap you need, you can use ImageSource_RowCache to provide the required support rows.

#ifndef IMAGESOURCE_TEE_H
#define IMAGESOURCE_TEE_H

#include "imagesource.h"
#include "debug.h"

class ImageSource_Tee : public ImageSource
{
	public:
	ImageSource_Tee(ImageSource *source) : ImageSource(source), source(source)
	{
		if(!randomaccess)
			Debug[WARN] << "WARNING: ImageSource_Tee being used with an image that doesn't support random access." << std::endl;
	}
	~ImageSource_Tee()
	{
	}
	ISDataType *GetRow(int row)
	{
		return(source->GetRow(row));
	}
	protected:
	ImageSource *source;
};

#endif

