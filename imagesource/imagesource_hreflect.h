/*
 * imagesource_hreflect.h - filter to reflect an image horizontally.
 *
 * Copyright (c) 2010 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_GAMMA_H
#define IMAGESOURCE_GAMMA_H

#include "imagesource.h"
#include "lcmswrapper.h"


#define IS_GAMMA(x,g) ISDataType(IS_SAMPLEMAX*pow((double(x)/IS_SAMPLEMAX),(g)))
#define IS_INVGAMMA(x,g) ISDataType(IS_SAMPLEMAX*pow((double(x)/IS_SAMPLEMAX),1.0/(g)))


class ImageSource_HReflect : public ImageSource
{
	public:
	ImageSource_HReflect(ImageSource *source);
	virtual ~ImageSource_HReflect();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
};

#endif
