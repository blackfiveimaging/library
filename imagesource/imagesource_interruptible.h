/*
 * imagesource_interruptible.h - subclass of ImageSource which provides
 * for the interruption of lengthy operations - rotation in particular
 * benefits from this.
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_INTERRUPTIBLE_H
#define IMAGESOURCE_INTERRUPTIBLE_H

#include <stdlib.h>
#include "imagesource.h"
#include "imagesource_types.h"

class CMSProfile;

class ImageSource_Interruptible : public ImageSource
{
	public:
	ImageSource_Interruptible(ImageSource *src) : ImageSource(src), testbreak(NULL), userdata(NULL)
	{
	}
	virtual ~ImageSource_Interruptible()
	{
	}
	void SetTestBreak(bool (*testbreak)(void *userdata),void *userdata)
	{
		this->testbreak=testbreak;
		this->userdata=userdata;
	}
	bool TestBreak()
	{
		if(testbreak)
			return(testbreak(userdata));
		return(false);
	}
	protected:
	bool (*testbreak)(void *userdata);
	void *userdata;
};


#endif
