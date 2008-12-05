/*
 * imagesource_scaledensity.h
 * Scales values in an imagesource according to a given value.
 * Scales RGB towards IS_SAMPLEMAX, scales Grey and CMYK towards 0.
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_SCALEDENSITY_H
#define IMAGESOURCE_SCALEDENSITY_H

#include "imagesource.h"
#include "lcmswrapper.h"

#include <lcms.h>


class ImageSource_ScaleDensity : public ImageSource
{
	public:
	ImageSource_ScaleDensity(ImageSource *source,float density);
	virtual ~ImageSource_ScaleDensity();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
	float density;
};

#endif
