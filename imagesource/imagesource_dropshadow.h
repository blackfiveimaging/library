#ifndef IMAGESOURCE_DROPSHADOW_H
#define IMAGESOURCE_DROPSHADOW_H

#include "imagesource_gaussianblur.h"
#include "imagesource_solid.h"
#include "imagesource_montage.h"
#include "imagesource_extractchannel.h"
#include "imagesource_colorize.h"
#include "imagesource_mask.h"
#include "imagesource_rowcache.h"
#include "imagesource_tee.h"

class ImageSource_DropShadow : public ImageSource
{
	public:
	ImageSource_DropShadow(ImageSource *src,int radius,int offset)
		: ImageSource(src), source(src), grey(1,IS_SAMPLEMAX/2), radius(radius), offset(offset)
	{
		ImageSource *cached=new ImageSource_RowCache(src,1,2*(radius+offset));

		// build mask - or take it from alpha channel if there is one...
		ImageSource_Montage *mon=new ImageSource_Montage(IS_TYPE_GREY);
		ISDataType white[]={0};

		ImageSource *tmp;
		if(src->type==IS_TYPE_RGBA)
		{
			tmp=new ImageSource_ExtractChannel(new ImageSource_Tee(cached),3);
			tmp=new ImageSource_Colorize(tmp,IS_TYPE_GREY,grey);
		}
		else
			tmp=new ImageSource_Solid(IS_TYPE_GREY,src->width,src->height,&grey[0]);

		mon->Add(tmp,radius,radius);
		tmp=new ImageSource_Solid(IS_TYPE_GREY,src->width+radius*2+offset,src->height+radius*2+offset,white);
		mon->Add(tmp,0,0);
		ImageSource *mask=new ImageSource_GaussianBlur(mon,radius);

		ISDataType black[]={0,0,0,IS_SAMPLEMAX};
		ImageSource *shadow=new ImageSource_Solid(IS_TYPE_RGBA,mask->width,mask->height,black);
		shadow=new ImageSource_Mask(shadow,mask);

		ImageSource_Montage *mon2=new ImageSource_Montage(IS_TYPE_RGBA);
		mon2->Add(src,radius-offset,radius-offset);

		mon2->Add(shadow,0,0);
		source=mon2;
		width=mon2->width;
		height=mon2->height;
		samplesperpixel=mon2->samplesperpixel;
		type=mon2->type;
	}
	~ImageSource_DropShadow()
	{
		if(source)
			delete source;
	}
	ISDataType *GetRow(int row)
	{
		return(source->GetRow(row));
	}
	protected:
	ImageSource *source;
	ISDeviceNValue grey;
	int radius;
	int offset;
};

#endif

