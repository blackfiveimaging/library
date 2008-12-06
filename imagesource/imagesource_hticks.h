/*
 * imagesource_hticks.h - renders a horizontal scale of "tick marks".
 * Supports Random Access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include "imagesource.h"

class ImageSource_HTicks : public ImageSource
{
	public:
	ImageSource_HTicks(int width,int height,IS_TYPE type=IS_TYPE_RGB,int samplesperpixel=3,int majorticks=10,int minorticks=100);
	~ImageSource_HTicks();
	void SetFG(ISDeviceNValue &col);
	void SetBG(ISDeviceNValue &col);
	ISDataType *GetRow(int row);
	protected:
	int majorticks;
	int minorticks;
	ISDeviceNValue fg;
	ISDeviceNValue bg;
};
