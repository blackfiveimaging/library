/*
 * imagesource_greyscale.h
 *
 * Supports RGB and CMYK data
 * Supports random access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_GREYSCALE_H
#define IMAGESOURCE_GREYSCALE_H

#include "imagesource.h"

class ImageSource_Greyscale : public ImageSource
{
	public:
	ImageSource_Greyscale(ImageSource *source);
	~ImageSource_Greyscale();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
};

#endif
