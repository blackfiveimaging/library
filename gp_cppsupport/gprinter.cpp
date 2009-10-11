
/*
 * print.cpp - encapsulates printing via GIMP-Print/GutenPrint
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * 2005-12-24: Zeroed out firstrow and firstpixel in GetDimensions,
 * in the hope that it'll fix an intermittent offset bug.
 *
 */

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gutenprint/gutenprint.h>

#include <glib.h>
#include <glib/gstrfuncs.h>
#include <glib/gprintf.h>

#include "../imagesource/imagesource.h"
#include "../support/md5.h"
#include "../miscwidgets/generaldialogs.h"

#include "gprinter.h"

#include "../config.h"
#include "../gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)


#define USE16BITPRINTING

using namespace std;


// We use this to ensure that stp_init() has been called once and only once.
class GPrinter_StaticInitializer
{
	public:
	GPrinter_StaticInitializer()
	{
		if(stp_init())
		{
			cerr << "Couldn't initialize Gutenprint - check STP_DATA_PATH variable" << endl;
//			throw "Couldn't initialize Gutenprint - check STP_DATA_PATH variable";
		}
	}
} gpstaticinitializer;


/*
	Get dimensions from the printer driver, and calculate position on page from image size.
	Image must have been loaded first.
*/

void GPrinter::get_dimensions()
{
	int rndfactor,rndwidth,rndheight;

	GetImageableArea();

	int xres=int(source->xres);
	int yres=int(source->yres);

	int w=pixelwidth=source->width;
	w*=72;
	w/=xres;
	ptwidth=w;
		
	int h=pixelheight=source->height;
	h*=72;
	h/=yres;
	ptheight=h;

	firstpixel=0;
	firstrow=0;

	if(ypos<0)
	{
		if(ptheight<=imageableheight)
		{
			papertop=topmargin+(imageableheight-ptheight)/2;
			firstrow=0;
		}
		else
		{
			int clipped;
			cerr << "Warning: Image is taller than usable page - clipping." << endl;
			papertop=topmargin;
			clipped=(ptheight-imageableheight)/2;
			clipped*=yres;
			clipped/=72;
			firstrow=clipped;
			pixelheight-=2*clipped;
			ptheight=imageableheight;
		}
	}
	else
	{
		int clipped;
		papertop=ypos;
		if(ypos<topmargin)
		{
			cerr << "Warning: Image is clipped by top margin." << endl;
			papertop=topmargin;
			clipped=topmargin-ypos;
			ptheight-=clipped;
			clipped*=yres;
			clipped/=72;
			firstrow=clipped;
			pixelheight-=clipped;
		}
		if((papertop+ptheight)>(pageheight-bottommargin))
		{
			cerr << "Warning: Image is clipped by bottom margin." << endl;
			clipped=(papertop+ptheight)-(pageheight-bottommargin);
			ptheight-=clipped;
			clipped*=yres;
			clipped/=72;
			pixelheight-=clipped;
		}
	}
	
	if(xpos<0)
	{
		if(ptwidth<=imageablewidth)
		{
			paperleft=leftmargin+(imageablewidth-ptwidth)/2;
			firstpixel=0;
		}
		else
		{
			int clipped;
			fprintf(stderr,"Warning: Image is wider than usable page - clipping.\n");
			paperleft=leftmargin;
			clipped=(ptwidth-imageablewidth)/2;
			clipped*=xres;
			clipped/=72;
			firstpixel=clipped;
			pixelwidth-=2*clipped;
			ptwidth=imageablewidth;
		}
	}
	else
	{
		int clipped;
		paperleft=xpos;
		if(xpos<leftmargin)
		{
			cerr << "Warning: Image is clipped by left margin." << endl;
			paperleft=leftmargin;
			clipped=leftmargin-xpos;
			ptwidth-=clipped;
			clipped*=xres;
			clipped/=72;
			firstpixel=clipped;
			pixelwidth-=clipped;
		}
		if((paperleft+ptwidth)>(pagewidth-rightmargin))
		{
			cerr << "Warning: Image is clipped by right margin." << endl;
			clipped=(paperleft+ptwidth)-(pagewidth-rightmargin);
			ptwidth-=clipped;
			clipped*=xres;
			clipped/=72;
			pixelwidth-=clipped;
		}
	}

	/* Avoid moire patterns by adjusting the pixel width and height to be a
	   round number of points */
	if(xres>72)
	{
		rndfactor=xres/72;
		rndwidth=pixelwidth/rndfactor;
		rndwidth*=rndfactor;
		pixelwidth=rndwidth;
	}
	
	if(yres>72)
	{
		rndfactor=yres/72;
		rndheight=pixelheight/rndfactor;
		rndheight*=rndfactor;
		pixelheight=rndheight;
	}
}


static void parameter_help(stp_parameter_t &dparam)
{
	switch(dparam.p_type)
	{
		case STP_PARAMETER_TYPE_STRING_LIST:
			{
				int strcount,j;
				printf("Parameter: %s\n",dparam.name);
				stp_string_list_t *strlist=dparam.bounds.str;
				if(strlist)
				{
					strcount=stp_string_list_count(strlist);
					for(j=0;j<strcount;++j)
					{
						stp_param_string_t *p=stp_string_list_param(strlist,j);
						printf("  %s: %s\n",p->name,p->text);
					}
				}
			}
			break;
		case STP_PARAMETER_TYPE_INT:
			printf("Parameter: %s\n",dparam.name);
			printf("  %d - %d\n",dparam.bounds.integer.lower,dparam.bounds.integer.upper);
			break;
		case STP_PARAMETER_TYPE_DOUBLE:
			printf("Parameter: %s\n",dparam.name);
			printf("  %f - %f\n",dparam.bounds.dbl.lower,dparam.bounds.dbl.upper);
			break;
		case STP_PARAMETER_TYPE_BOOLEAN:
			printf("Parameter: %s\n",dparam.name);
			printf("  0/1\n");
			break;
		case STP_PARAMETER_TYPE_INVALID:
			printf("Parameter: %s\n",dparam.name);
			printf("  Paramter not valid for this printer\n");
			break;
		default:
//			printf("Parameter: %s\n",dparam.name);
//			printf("  FIXME: Can't handle param type %d\n",dparam.p_type);
			break;
	}
}


void GPrinter::Help()
{
	const stp_vars_t *v = stp_printer_get_defaults(the_printer);
	stp_parameter_list_t params = stp_get_parameter_list(v);
	
	int count = stp_parameter_list_count(params);
	
	for (int i = 0; i < count; i++)
	{
		const stp_parameter_t *p = stp_parameter_list_param(params, i);
		if((p->p_level<=STP_PARAMETER_LEVEL_ADVANCED4))
		{
			stp_parameter_t desc;
			stp_describe_parameter(v,p->name,&desc);
			if(desc.is_active)
			{
				parameter_help(desc);
			}
			stp_parameter_description_destroy(&desc);
		}
	}
}


void GPrinter::writefunc(void *obj, const char *buf, size_t bytes)
{
	if(bytes)	// Have seen requests to write zero bytes - catch them
				// otherwise cons->Write() will quite correctly report that
				// it's written zero byes and result in job termination!
	{
		Consumer *cons=(Consumer *)obj;
		bool result=false;
		if(cons)
			result=cons->Write(buf,bytes);
		if(!result)
		{
			writeerror=true;
			cerr << "cons->Write() returned " << result << endl;
		}
	}
}


void GPrinter::Print(ImageSource *src,int xpos,int ypos,Consumer *cons)
{
	cerr << "*** GPrinter: Printing at position: " << xpos << ", " << ypos << endl;
	source=src;
	switch(source->type)
	{
		case IS_TYPE_RGB:
			cerr << "Printing in RGB mode" << endl;
			stp_set_string_parameter(stpvars, "InputImageType", "RGB");
			break;
		case IS_TYPE_CMYK:
			cerr << "Printing in CMYK mode" << endl;
			stp_set_string_parameter(stpvars, "InputImageType", "CMYK");
			break;
		case IS_TYPE_DEVICEN:
			cerr << "Printing in DeviceN mode" << endl;
			stp_set_string_parameter(stpvars, "InputImageType", "Raw");
			{
				char nchan[10];
				sprintf(nchan,"%d",src->samplesperpixel);
				stp_set_string_parameter(stpvars, "RawChannels", nchan);
			}
			break;
		default:
			throw "Only RGB and CMYK images are currently supported!";
			break;
	}

#ifdef USE8BITPRINTING
			stp_set_string_parameter(stpvars, "ChannelBitDepth", "8");
#else
			stp_set_string_parameter(stpvars, "ChannelBitDepth", "16");
#endif

	cerr << "Checking PrintingMode:" << endl;
	const char *pm=stp_get_string_parameter(stpvars,"PrintingMode");
	if(pm)
		cerr << "PrintingMode currently set to:" << pm << endl;
	else
		cerr << "PrintingMode currently not set." << endl;
//	stp_set_string_parameter(stpvars, "PrintingMode", "Color");

	this->xpos=xpos;
	this->ypos=ypos;
	get_dimensions();
	stp_set_width(stpvars, ptwidth);
	stp_set_height(stpvars, ptheight);
	cerr << "Paperleft: " << paperleft << endl;
	cerr << "Papertop: " << papertop << endl;
	stp_set_left(stpvars, paperleft-leftbleed);
	stp_set_top(stpvars, papertop-topbleed);

	if(!(consumer=cons))
		consumer=output.GetConsumer();

	if(!consumer)
		return;

	stp_set_outfunc(stpvars, writefunc);
	stp_set_outdata(stpvars, consumer);

	stp_vars_t *tmpvars=stp_vars_create_copy(stpvars);
	const stp_vars_t *current_vars = stp_printer_get_defaults(stp_get_printer(stpvars));
	stp_merge_printvars(tmpvars, current_vars);

	writeerror=false;

//	Dump();
	bool result=true;
	const char *error=_("Unable to generate print data");
	result&=stp_verify(tmpvars);
	if(result)
	{
		result&=stp_print(tmpvars, &stpImage);
		error=NULL;
	}

	stp_vars_destroy(tmpvars);

	if(!cons)
		delete consumer;
	
	if(result==0 && error)
		throw error;

	if(writeerror)
		throw "Write error: check your print command";
}


// Deal with borderless mode by adding the bleed area to the media size
// and reducing the negative margins to zero.

// Obsolete hack - but shouldn't cause problems now borderless is working
// properly in Gutenprint - so leave it for now:
// The bottom bleed is excessively large on the R300 - presumably because
// the driver just increases the form length to work around the lack of
// a true bleed option.
// Can we shuffle things around so that a saner bottom bleed is presented
// to the user?

void GPrinter::GetImageableArea()
{
//	pagewidth=pageheight=0;
//	stp_get_media_size(stpvars, &pagewidth, &pageheight);

//	cerr << "Media size returned: " << pagewidth << " by " << pageheight << endl;

	const char *papersize=stp_get_string_parameter(stpvars,"PageSize");
	bool gotpapersize=false;
	if(papersize)
	{
		const stp_papersize_t *paper=stp_get_papersize_by_name(papersize);
		if(paper)
		{
			gotpapersize=true;
			if(paper->width)
			{
				pagewidth=minwidth=maxwidth=paper->width;
				stp_set_page_width(stpvars,pagewidth);
				cerr << "Width: " << pagewidth << endl;
			}
			else
			{
				int mw,mh,nw,nh;
				pagewidth=stp_get_page_width(stpvars);
				stp_get_size_limit(stpvars,&mw,&mh,&nw,&nh);
				minwidth=nw;
				maxwidth=mw;
				cerr << "Custom width..." << endl;
			}
			if(paper->height)
			{
				pageheight=minheight=maxheight=paper->height;
				stp_set_page_height(stpvars,pageheight);
				cerr << "Height: " << pageheight << endl;
			}
			else
			{
				int mw,mh,nw,nh;
				pageheight=stp_get_page_height(stpvars);
				stp_get_size_limit(stpvars,&mw,&mh,&nw,&nh);
				minheight=nh;
				maxheight=mh;
				cerr << "Custom height..." << endl;
			}
		}
	}
	if(!gotpapersize)
	{
		pagewidth=pageheight=0;
		stp_get_media_size(stpvars, &pagewidth, &pageheight);
	}

	int l,r,t,b;
	stp_get_imageable_area(stpvars, &l, &r, &b, &t);

	cerr << "Imageable area from GP: L: " << l << ", R: " << r << ", T: " << t << ", B: " << b << endl;

	leftbleed=rightbleed=topbleed=bottombleed=0;

	// *** HACK ***
	// We reduce the bottom bleed to 7pt max, to work around the
	// extremely large bottom bleed reported by the R300.

	if((pageheight-b)<-7 && t<=0)
		b=pageheight+7;

	if(r>pagewidth)
	{
		rightbleed=r-pagewidth; pagewidth=r;
	}
	if(l<0)
	{
		pagewidth-=l; leftbleed=-l; r-=l; l=0;
	}
	if(b>pageheight)
	{
		bottombleed=b-pageheight; pageheight=b;
	}
	if(t<0)
	{
		pageheight-=t; topbleed=-t; b-=t; t=0;
	}

	leftmargin=l;
	topmargin=t;
	rightmargin=pagewidth-r;
	bottommargin=pageheight-b;

	cerr << "Pagewidth: " << pagewidth << endl;
	cerr << "Pageheight: " << pageheight << endl;

	PageExtent::GetImageableArea();

	cerr << "Imageable width: " << imageablewidth << endl;
	cerr << "Imageable height: " << imageableheight << endl;

	cerr << "Left bleed: " << leftbleed << endl;
	cerr << "Right bleed: " << rightbleed << endl;
	cerr << "Top bleed: " << topbleed << endl;
	cerr << "Bottom bleed: " << bottombleed << endl;

	// HACK
	// There seems to be a problem with GutenPrint's setlocale() calls.
	cerr << "After reading papersize and margins: " << setlocale(LC_ALL,"") << endl;
}


void GPrinter::GetSizeLimits(int &minw,int &maxw,int &minh,int &maxh)
{
	minw=minwidth;
	maxw=maxwidth;
	minh=minheight;
	maxh=maxheight;
}


void GPrinter::SetCustomWidth(int w)
{
	cerr << "New width = " << w << endl;
	cerr << "New width without bleed = " << w-(leftbleed+rightbleed) << endl;
	stp_set_page_width(stpvars,w-(leftbleed+rightbleed));
	pagewidth=w-(leftbleed+rightbleed);
}


void GPrinter::SetCustomHeight(int h)
{
	cerr << "New height = " << h << endl;
	cerr << "New height without bleed = " << h-(topbleed+bottombleed) << endl;
	stp_set_page_height(stpvars,h-(topbleed+bottombleed));
	pageheight=h-(topbleed+bottombleed);
}


GPrinter::GPrinter(PrintOutput &output,ConfigFile *ini,const char *section)
	: GPrinterSettings(output,ini,section), source(NULL), firstrow(0), firstpixel(0), progress(NULL)
{
	stpImage.rep=this;
	staticinitializer=&gpstaticinitializer;
}


void GPrinter::SetProgress(Progress *p)
{
	progress=p;
}


GPrinter::~GPrinter()
{
}


int GPrinter::Image_width(struct stp_image *image)
{
	GPrinter *gp=(GPrinter *)image->rep;
	return(gp->pixelwidth);
}
  
  
int GPrinter::Image_height(struct stp_image *image)
{
	GPrinter *gp=(GPrinter *)image->rep;
	return(gp->pixelheight);
}


stp_image_status_t GPrinter::GetRowStub(struct stp_image *image, unsigned char *data,
                                size_t byte_limit, int row)
{
	stp_image_status_t result;
	GPrinter *gp=(GPrinter *)image->rep;
	result=gp->GetRow(row,data);
	return(result);
}


stp_image_status_t GPrinter::GetRow(int row,unsigned char *data)
{
	stp_image_status_t result=STP_IMAGE_STATUS_OK;
	ISDataType *src;
#ifdef USE8BITPRINTING
	unsigned char *dst=(unsigned char *)data;
#else
	unsigned short *dst=(unsigned short *)data;
#endif
	int i;

	src=source->GetRow(row+firstrow);
	src+=(source->samplesperpixel*firstpixel);

	if(progress && !(row&31))
	{
		if(!(progress->DoProgress(row,pixelheight)))
		{
			result=STP_IMAGE_STATUS_ABORT;
			consumer->Cancel();
		}
	}
	
	switch(source->type)
	{
		case IS_TYPE_RGB:
		case IS_TYPE_CMYK:
		case IS_TYPE_DEVICEN:
			for(i=0;i<pixelwidth*source->samplesperpixel;++i)
			{
#ifdef USE8BITPRINTING
				ISDataType t=*src++;
				*dst++=ISTOEIGHT(t);
#else
				*dst++=*src++;
#endif
			}
			break;
		default:
			result=STP_IMAGE_STATUS_ABORT;
			break;
	}

	if(writeerror)
		result=STP_IMAGE_STATUS_ABORT;

	return result;
}

const char *GPrinter::Image_get_appname(struct stp_image *image)
{
	return("Gutenprint output component");
}

bool GPrinter::writeerror;

stp_image_t GPrinter::stpImage =
{
  NULL,
  NULL,
  GPrinter::Image_width,
  GPrinter::Image_height,
  GPrinter::GetRowStub,
  GPrinter::Image_get_appname,
  NULL,
  NULL
};


// ----------- ResponseHash ----------------
// Prints a known test image at a known position on the page, and instead of printing
// the result, generates an MD5 Hash.  This can be used as an indicator of whether
// anything's changed with the printer driver or settings.


// Simple rainbow testpattern.  Fades from full saturation RGB testpattern at the head
// to solid black at the foot.

class ImageSource_RainbowSweep : public ImageSource
{
	public:
	ImageSource_RainbowSweep(int width,int height)
		: ImageSource(width,height,IS_TYPE_RGB)
	{
		randomaccess=true;
		MakeRowBuffer();
	}
	~ImageSource_RainbowSweep()
	{
	}

	ISDataType *GetRow(int row)
	{
		if(currentrow==row)
			return(rowbuffer);
		double a=row;
		a/=height;

		for(int x=0;x<width;++x)
		{
			int c=(x*1536)/width;
			int r=c;
			if(r>768) r-=1536;
			if(r<0) r=-r;
			r=512-r;
			if(r<0) r=0;
			if(r>255) r=255;

			int g=c-512;
			if(g<0) g=-g;
			g=512-g;
			if(g<0) g=0;
			if(g>255) g=255;

			int b=c-1024;
			if(b<0) b=-b;
			b=512-b;
			if(b<0) b=0;
			if(b>255) b=255;

			r=r*(1-a);
			g=g*(1-a);
			b=b*(1-a);

			rowbuffer[x*3]=EIGHTTOIS(r);
			rowbuffer[x*3+1]=EIGHTTOIS(g);
			rowbuffer[x*3+2]=EIGHTTOIS(b);
		}
		
		currentrow=row;
		return(rowbuffer);
	}
};


// A variation on the consumer class which maintains an MD5 hash instead of sending the data anywhere.

class MD5Consumer : public Consumer, public MD5Digest
{
	public:
	MD5Consumer() : Consumer(), MD5Digest()
	{
	}
	virtual ~MD5Consumer()
	{
	}
	virtual bool Write(const char *buffer,int length)
	{
		Update(buffer,length);

		// There's an issue with the PostScript driver in that it includes the creation date
		// so the MD5 hash will be different every time.  To get around this we reset the MD5 if we
		// encounter a CreationDate comment.
		if(length>14 && strncmp(buffer,"%%CreationDate",14)==0)
			InitMD5Context();
		return(true);
	}
	virtual void Cancel()
	{
	}
};


std::string GPrinter::GetResponseHash(Progress *p)
{
	ImageSource *is=new ImageSource_RainbowSweep(360,200);
	is->SetResolution(180,180);

	MD5Consumer cons;

	// Disable borderless mode and set a standard papersize...
	bool prevborderless=false;
	const char *prevpaper=NULL;
	char *paper=NULL;
	stp_parameter_t desc;
	stp_describe_parameter(stpvars,"FullBleed",&desc);
	if(desc.is_active)
	{
		prevborderless=stp_get_boolean_parameter(stpvars,"FullBleed");
		prevpaper=stp_get_string_parameter(stpvars,"PageSize");
		if(prevpaper)
			paper=strdup(prevpaper);
		stp_set_boolean_parameter(stpvars,"FullBleed",false);
		stp_set_string_parameter(stpvars,"PageSize","A4");
	}

	SetProgress(p);
	Print(is,72,72,&cons);
	SetProgress(NULL);

	if(desc.is_active)
	{
		stp_set_boolean_parameter(stpvars,"FullBleed",prevborderless);
		if(paper)
		{
			stp_set_string_parameter(stpvars,"PageSize",paper);
			free(paper);
		}
		GetImageableArea();
	};

	delete is;

	return(std::string(cons.GetPrintableDigest()));
}

