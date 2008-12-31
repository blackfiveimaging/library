/*
 * imagesource_devicen_remap.h
 *
 * Supports DeviceN
 * Supports random access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_DEVICEN_REMAP_H
#define IMAGESOURCE_DEVICEN_REMAP_H

#include "imagesource.h"

class ImageSource_DeviceN_Remap : public ImageSource
{
	public:
	// If outchannels==0, the output will have the same number of channels as the input
	ImageSource_DeviceN_Remap(ImageSource *source,const int *maptable,int outchannels=0);
	~ImageSource_DeviceN_Remap();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
	int *table;
};


// Used to force an ImageSource to DeviceN type - which can't be done automatically
// since 4spp Separated space is the only way TIFFs mark CMYK, for instance.
// Also inverts RGB data (excluding alpha channel).

class ImageSource_ToDeviceN : public ImageSource
{
	public:
	ImageSource_ToDeviceN(ImageSource *source);
	~ImageSource_ToDeviceN();
	ISDataType *GetRow(int row);
	protected:
	ImageSource *source;
};


// Forces an ImageSource from DeviceN to RGB, inverting the data as it goes.
// NOTE this *doesn't* remap the channels or downrender to RGB - it merely
// re-interprets the data as RGB.
// Alpha channel, if present, is not inverted.

class ImageSource_DeviceNToRGB : public ImageSource
{
	public:
	ImageSource_DeviceNToRGB(ImageSource *source);
	~ImageSource_DeviceNToRGB();
	ISDataType *GetRow(int row);
	protected:
	ImageSource *source;
};


// Forces an ImageSource from DeviceN to CMYK
// NOTE this *doesn't* remap the channels or downrender to CMYK - it merely
// re-interprets the data as CMYK.

class ImageSource_DeviceNToCMYK : public ImageSource
{
	public:
	ImageSource_DeviceNToCMYK(ImageSource *source);
	~ImageSource_DeviceNToCMYK();
	ISDataType *GetRow(int row);
	protected:
	ImageSource *source;
};


#endif
