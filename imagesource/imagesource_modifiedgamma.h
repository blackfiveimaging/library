/*
 * imagesource_modifiedgamma.h
 * Applies Gamma correction to an image - but uses a modified gamma curve
 * with a linear segment to avoid problems with infinite gradient at 0.
 *
 * The modified gamma curve is:
 * Y = pow((X+offset)/(1.0+offset),gamma) if X > threshold
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


class ImageSource_ModifiedGamma : public ImageSource
{
	public:
	ImageSource_ModifiedGamma(ImageSource *source,double gamma,double offset=-0.02);
	virtual ~ImageSource_ModifiedGamma();
	ISDataType *GetRow(int row);
	static double FindGamma(double x,double y,double offset=-0.02);
	static double ModifiedGamma(double x,double gamma,double offset=-0.02);
	static double InverseModifiedGamma(double x, double gamma, double offset=-0.02);
	private:
	ImageSource *source;
	double gamma;
	double offset;
	double threshold;
	double slope;
};

#endif
