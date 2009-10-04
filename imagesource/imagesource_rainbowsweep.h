/*
 * imagesource_hsweep.h - renders a rainbow sweep from full to zero saturation.
 * Supports Random Access
 *
 * Copyright (c) 2009 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include "imagesource.h"

class ImageSource_RainbowSweep : public ImageSource
{
	public:
	ImageSource_RainbowSweep(int width,int height);
	~ImageSource_RainbowSweep();
	ISDataType *GetRow(int row);
	protected:
};
