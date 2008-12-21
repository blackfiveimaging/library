/*
 * imagesource_invert.h
 *
 * Supports all types
 * Supports random access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_INVERT_H
#define IMAGESOURCE_INVERT_H

#include "imagesource.h"

class ImageSource_Invert : public ImageSource
{
	public:
	ImageSource_Invert(ImageSource *source);
	~ImageSource_Invert();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
};

#endif
