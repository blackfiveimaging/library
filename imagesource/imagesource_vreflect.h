/*
 * imagesource_vreflect.h - filter to reflect an image vertically.
 * supports random access, even if source image doesn't.
 *
 * Copyright (c) 2010 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_VREFLECT_H
#define IMAGESOURCE_VREFLECT_H

#include "imagesource.h"
#include "imagesource_interruptible.h"

struct ImageSource *ImageSource_Rotate_New(struct ImageSource *source,int rotation,int spanrows);

class ImageSource_VReflect : public ImageSource_Interruptible
{
	public:
	ImageSource_VReflect(ImageSource *source);
	~ImageSource_VReflect();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
	int rotation;
	int spanfirstrow;
	int spanrows;
	int samplesperrow;
	ISDataType *spanbuffer;
};

#endif
