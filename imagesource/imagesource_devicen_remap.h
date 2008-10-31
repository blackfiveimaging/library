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
	ImageSource_DeviceN_Remap(ImageSource *source,const int *maptable);
	~ImageSource_DeviceN_Remap();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
	int *table;
};

#endif
