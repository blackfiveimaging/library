#ifndef NAIVECMYKTRANSFORM_H
#define NAIVECMYKTRANSFORM_H

#include "imagesource.h"
#include "lcmswrapper_lcms2.h"

class NaiveRGBToCMYKCMSTransform : public CMSTransform
{
	public:
	NaiveRGBToCMYKCMSTransform(ImageSource *src) : CMSTransform()
	{
		inputtype=src->type;
		outputtype=IS_TYPE_CMYK;
	}
	virtual void Transform(unsigned short *src,unsigned short *dst,int pixels)
	{
		switch(inputtype)
		{
			case IS_TYPE_RGB:
				for(int x=0;x<pixels;++x)
				{
					unsigned int c=IS_SAMPLEMAX-src[x*3];
					unsigned int m=IS_SAMPLEMAX-src[x*3+1];
					unsigned int y=IS_SAMPLEMAX-src[x*3+2];
					unsigned int k=c;
					if(m<k) k=m;
					if(y<k) k=y;
					dst[x*4+3]=k;
					dst[x*4]=c-k;
					dst[x*4+1]=m-k;
					dst[x*4+2]=y-k;
				}
				break;
			case IS_TYPE_CMYK:
				for(int x=0;x<pixels;++x)
				{
					*dst++=*src++;
					*dst++=*src++;
					*dst++=*src++;
					*dst++=*src++;
				}
				break;
			default:
				throw "NaiveRGBToCMYKTransform: Input type not yet supported";
				break;
		}
	}
};


class NaiveCMYKToRGBCMSTransform : public CMSTransform
{
	public:
	NaiveCMYKToRGBCMSTransform(ImageSource *src) : CMSTransform()
	{
		inputtype=src->type;
		outputtype=IS_TYPE_RGB;
	}
	virtual void Transform(unsigned short *src,unsigned short *dst,int pixels)
	{
		switch(inputtype)
		{
			case IS_TYPE_RGB:
				for(int x=0;x<pixels;++x)
				{
					*dst++=*src++;
					*dst++=*src++;
					*dst++=*src++;
				}
				break;
			case IS_TYPE_CMYK:
				for(int x=0;x<pixels;++x)
				{
					int pc=src[x*4];
					int pm=src[x*4+1];
					int py=src[x*4+2];
					int pk=src[x*4+3];
					int r=(IS_SAMPLEMAX-pc)-(pk);
					int g=(IS_SAMPLEMAX-pm)-(pk);
					int b=(IS_SAMPLEMAX-py)-(pk);
					if(r<0) r=0;
					if(g<0) g=0;
					if(b<0) b=0;
					dst[x*3]=r;
					dst[x*3+1]=g;
					dst[x*3+2]=b;
				}
				break;
			default:
				throw "NaiveCMYKToRGBTransform: Input type not yet supported";
				break;
		}
	}
};


class NullCMSTransform : public CMSTransform
{
	public:
	NullCMSTransform(ImageSource *src) : CMSTransform()
	{
		inputtype=outputtype=src->type;
		samplesperpixel=src->samplesperpixel;
	}
	NullCMSTransform(IS_TYPE type) : CMSTransform()
	{
		inputtype=outputtype=type;
		switch(STRIP_ALPHA(type))
		{
			case IS_TYPE_GREY:
				samplesperpixel=1;
				break;
			case IS_TYPE_RGB:
			case IS_TYPE_LAB:
			case IS_TYPE_HSV:
				samplesperpixel=3;
				break;
			case IS_TYPE_CMYK:
				samplesperpixel=4;
				break;
			default:
				throw "NullCMSTransform: unhandled image type";
				break;
		}
		if(HAS_ALPHA(type))
			++samplesperpixel;
	}
	virtual void Transform(unsigned short *src,unsigned short *dst,int pixels)
	{
		for(int i=0;i<pixels*samplesperpixel;++i)
		{
			*dst++=*src++;
		}
	}
	protected:
	int samplesperpixel;
};

#endif
