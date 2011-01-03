/*
 * imagesource.h - base class for the efficient scanline-based
 * handling of extremely large images.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_H
#define IMAGESOURCE_H

#include <stdlib.h>

#include "imagesource_types.h"
#include "refcountptr.h"
#include "lcmswrapper.h"

class ImageSource
{
	public:
	ImageSource();
	ImageSource(int width, int height, IS_TYPE type=IS_TYPE_RGB);
	ImageSource(ImageSource *src);
	virtual ~ImageSource();
	virtual ISDataType *GetRow(int row)=0;
	void MakeRowBuffer();
	void SetResolution(double xr,double yr);
	inline RefCountPtr<CMSProfile> GetEmbeddedProfile()	// Inlined to avoid link order problems
	{
		return(embeddedprofile);
	}
	void SetEmbeddedProfile(RefCountPtr<CMSProfile> profile);
	void SetEmbeddedProfile(CMSProfile *profile);	// Use with caution - will assume ownership...
	int width,height;
	enum IS_TYPE type;
	int samplesperpixel;
	double xres,yres;
	bool randomaccess;
	protected:
	RefCountPtr<CMSProfile> embeddedprofile;
	int currentrow;
	ISDataType *rowbuffer;
};


#endif
