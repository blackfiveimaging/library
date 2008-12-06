/*
 * imagesource_chequerboard.h - renders a chequerboard pattern
 * Supports Random Access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include "imagesource.h"

class ImageSource_Chequerboard : public ImageSource
{
	public:
	ImageSource_Chequerboard(int width,int height,IS_TYPE type=IS_TYPE_RGB,int samplesperpixel=3,int size=8);
	~ImageSource_Chequerboard();
	void SetFG(ISDeviceNValue &col);
	void SetBG(ISDeviceNValue &col);
	ISDataType *GetRow(int row);
	protected:
	int size;
	ISDeviceNValue fg;
	ISDeviceNValue bg;
};
