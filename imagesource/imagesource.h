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
#include <map>

#include "imagesource_types.h"
#include "imagesource_parasite.h"
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
	RefCountPtr<ISParasite> GetParasite(ISParasiteType type,ISParasiteApplicability applic=ISPARA_UNIVERSAL);
	inline const std::map<ISParasiteType,RefCountPtr<ISParasite> > &GetParasites()
	{
		return(parasites);
	}
	int width,height;
	enum IS_TYPE type;
	int samplesperpixel;
	double xres,yres;
	bool randomaccess;
	protected:
	RefCountPtr<CMSProfile> embeddedprofile;
	std::map<ISParasiteType,RefCountPtr<ISParasite> > parasites;
	int currentrow;
	ISDataType *rowbuffer;
};

typedef ImageSource *ImageSource_p;	// Normal pointer to imagesource type
typedef RefCountPtr<ImageSource> ImageSource_rp; // Refcounted smart-pointer imagesource type

#endif
