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

#include <string.h>

#include "imagesource.h"

#include "devicencolorant.h"

// A Colorant preview is just an RGB approximate of a colorant's colour.
// This is used by the DeviceN Preview to render an approximate RGB
// impression of the complete DeviceN image.
// TODO:  A colorimetrically-correct approach.

class ImageSource_DeviceN_Preview;
class ISDeviceN_Colorant_Preview
{
	public:
	ISDeviceN_Colorant_Preview();
	ISDeviceN_Colorant_Preview(const char *longname,int red, int green, int blue, char alias=0);
	ISDeviceN_Colorant_Preview(const char *longname);
	ISDeviceN_Colorant_Preview(const ISDeviceN_Colorant_Preview &other);
	ISDeviceN_Colorant_Preview &operator=(const ISDeviceN_Colorant_Preview &other);
	~ISDeviceN_Colorant_Preview();
	ISDataType red, green, blue;
	char *longname;
	char alias;
	friend class ImageSource_DeviceN_Preview;
};

// These defines can be used in creating colorant tables to avoid
// specifying RGB values directly.

#define ISDEVICEN_PREVIEW_CYAN ISDeviceN_Colorant_Preview("Cyan",0,190,255,'C')
#define ISDEVICEN_PREVIEW_MAGENTA ISDeviceN_Colorant_Preview("Magenta",255,0,190,'M')
#define ISDEVICEN_PREVIEW_YELLOW ISDeviceN_Colorant_Preview("Yellow",255,255,0,'Y')
#define ISDEVICEN_PREVIEW_BLACK ISDeviceN_Colorant_Preview("Black",0,0,0,'K')
#define ISDEVICEN_PREVIEW_LIGHTCYAN ISDeviceN_Colorant_Preview("Light Cyan",127,220,255,'c')
#define ISDEVICEN_PREVIEW_LIGHTMAGENTA ISDeviceN_Colorant_Preview("Light Magenta",255,127,220,'m')
#define ISDEVICEN_PREVIEW_LIGHTBLACK ISDeviceN_Colorant_Preview("Light Black",127,127,127,'k')
#define ISDEVICEN_PREVIEW_RED ISDeviceN_Colorant_Preview("Red",255,0,0,'R')
#define ISDEVICEN_PREVIEW_BLUE ISDeviceN_Colorant_Preview("Blue",0,0,255,'B')

class ISDeviceNPreview_Colorant;

class ImageSource_DeviceN_Preview : public ImageSource
{
	public:
	ImageSource_DeviceN_Preview(ImageSource *source,DeviceNColorantList *cols,int firstcolorant=0);
	~ImageSource_DeviceN_Preview();
	ISDataType *GetRow(int row);
	protected:
	ImageSource *source;
	ISDeviceNPreview_Colorant *colorants;
};


#endif
