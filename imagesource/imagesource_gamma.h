/*
 * imagesource_Gamma.h
 * Scales values in an imagesource according to a given value.
 * Scales RGB towards IS_SAMPLEMAX, scales Grey and CMYK towards 0.
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_GAMMA_H
#define IMAGESOURCE_GAMMA_H

#include "imagesource.h"
#include "lcmswrapper.h"

#include <lcms.h>


#define IS_GAMMA(x,g) ISDataType(IS_SAMPLEMAX*pow((double(x)/IS_SAMPLEMAX),(g)))
#define IS_INVGAMMA(x,g) ISDataType(IS_SAMPLEMAX*pow((double(x)/IS_SAMPLEMAX),1.0/(g)))


class ImageSource_Gamma : public ImageSource
{
	public:
	ImageSource_Gamma(ImageSource *source,float gamma);
	virtual ~ImageSource_Gamma();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
	float gamma;
};

#endif
