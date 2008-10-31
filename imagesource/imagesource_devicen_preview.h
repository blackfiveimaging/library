/*
 * imagesource_devicen.h
 *
 * Renders an RGB preview of a DeviceN Image
 * Supports random access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_DEVICEN_H
#define IMAGESOURCE_DEVICEN_H

#include "imagesource.h"

// A Colorant preview is just an RGB approximate of a colorant's colour.
// This is used by the DeviceN Preview to render an approximate RGB
// impression of the complete DeviceN image.
// TODO:  A colorimetrically-correct approach.

class ImageSource_DeviceN_Preview;
class ISDeviceN_Colorant_Preview
{
	public:
	ISDeviceN_Colorant_Preview(int red, int green, int blue)
		: red(EIGHTTOIS(red)),green(EIGHTTOIS(green)),blue(EIGHTTOIS(blue))
	{
	}
	private:
	ISDataType red, green, blue;
	friend class ImageSource_DeviceN_Preview;
};

// These defines can be used in creating colorant tables to avoid
// specifying RGB values directly.

#define ISDEVICEN_PREVIEW_CYAN ISDeviceN_Colorant_Preview(0,190,255)
#define ISDEVICEN_PREVIEW_MAGENTA ISDeviceN_Colorant_Preview(255,0,190)
#define ISDEVICEN_PREVIEW_YELLOW ISDeviceN_Colorant_Preview(255,255,0)
#define ISDEVICEN_PREVIEW_BLACK ISDeviceN_Colorant_Preview(0,0,0)
#define ISDEVICEN_PREVIEW_LIGHTCYAN ISDeviceN_Colorant_Preview(0,95,127)
#define ISDEVICEN_PREVIEW_LIGHTMAGENTA ISDeviceN_Colorant_Preview(127,0,95)
#define ISDEVICEN_PREVIEW_LIGHTBLACK ISDeviceN_Colorant_Preview(127,127,127)
#define ISDEVICEN_PREVIEW_RED ISDeviceN_Colorant_Preview(255,0,0)
#define ISDEVICEN_PREVIEW_BLUE ISDeviceN_Colorant_Preview(0,0,255)

class ImageSource_DeviceN_Preview : public ImageSource
{
	public:
	ImageSource_DeviceN_Preview(ImageSource *source,ISDeviceN_Colorant_Preview *colorants);
	~ImageSource_DeviceN_Preview();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
	class ISDeviceN_Colorant_Preview *colorants;
};

#endif
