/*
 * imagesource_modifiedgamma.h
 * Applies Gamma correction to an image - but uses a modified gamma curve
 * with a linear segment to avoid problems with infinite gradient at 0.
 *
 * The modified gamma curve is:
 * Y = pow(X,(gamma+offset)/(1.0+offset)) if X > threshold
 * and Y = K * X otherwise.
 *
 * offset is -0.02 by default
 *
 * Threshold is automatically calculated as the point at which the line segment
 * has the same gradient as the gamma curve, which turns out to be
 * offset / (gamma + offset*gamma - 1.0)
 * 
 *
 * Copyright (c) 2009 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_MODIFIEDGAMMA_H
#define IMAGESOURCE_MODIFIEDGAMMA_H

#include "imagesource.h"
#include "lcmswrapper.h"

#include <lcms.h>


class ImageSource_ModifiedGamma : public ImageSource
{
	public:
	ImageSource_ModifiedGamma(ImageSource *source,float gamma,float offset=-0.02);
	virtual ~ImageSource_ModifiedGamma();
	ISDataType *GetRow(int row);
	static float FindGamma(float x,float y,float offset=-0.02);
	static float ModifiedGamma(float x,float gamma,float offset=-0.02);
	static float InverseModifiedGamma(float x, float gamma, float offset=-0.02);
	private:
	ImageSource *source;
	float gamma;
	float offset;
	float threshold;
	float slope;
};

#endif
