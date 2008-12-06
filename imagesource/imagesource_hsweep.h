/*
 * imagesource_hsweep.h - renders a horizontal sweep
 * Supports Random Access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include "imagesource.h"

class ImageSource_HSweep : public ImageSource
{
	public:
	ImageSource_HSweep(int width,int height,ISDeviceNValue &left,ISDeviceNValue &right,IS_TYPE type=IS_TYPE_RGB);
	~ImageSource_HSweep();
	ISDataType *GetRow(int row);
	protected:
	int size;
	ISDeviceNValue left;
	ISDeviceNValue right;
};
