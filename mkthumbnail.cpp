#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "imagesource_util.h"
#include "imagesource_solid.h"
#include "imagesource_montage.h"
#include "imagesource_gaussianblur.h"
#include "imagesource_mask.h"
#include "tiffsaver.h"
#include "jpegsaver.h"
#include "util.h"
#include "progresstext.h"


#if 0
#define TN_SIZE 128
int main(int argc,char **argv)
{
	try
	{
		for(int i=1;i<argc;++i)
		{
			char *tmpfn=BuildFilename(argv[i],"_tn","jpg");
			char *outfn=SafeStrcat(".",tmpfn);
			free(tmpfn);

			ImageSource *is=ISLoadImage(argv[i]);
			int w=TN_SIZE;
			int h=TN_SIZE;
			if((w=((is->width*h)/is->height))>TN_SIZE)
			{
				w=TN_SIZE;
				h=(is->height*w)/is->width;
			}
			is=ISScaleImageBySize(is,w,h);

			// build mask
			ImageSource_Montage *mon=new ImageSource_Montage(IS_TYPE_GREY);
			ISDataType white[]={0};
			ISDataType grey[]={IS_SAMPLEMAX/2};

			ImageSource *tmp;
			tmp=new ImageSource_Solid(IS_TYPE_GREY,w,h,grey);
			mon->Add(tmp,(128-w)/2+15,(128-h)/2+15);
			tmp=new ImageSource_Solid(IS_TYPE_GREY,155,155,white);
			mon->Add(tmp,0,0);
			cerr << "Montage spp: " << mon->samplesperpixel << endl;
			ImageSource *mask=new ImageSource_GaussianBlur(mon,8);

			ISDataType black[]={0,0,0};
			ISDataType bgcol[]={0xf4f4,0xf4f4,0xf4f4};
			ImageSource *background=new ImageSource_Solid(IS_TYPE_RGB,mask->width,mask->height,bgcol);
			ImageSource *shadow=new ImageSource_Solid(IS_TYPE_RGB,mask->width,mask->height,black);
			shadow=new ImageSource_Mask(shadow,mask);

			ImageSource_Montage *mon2=new ImageSource_Montage(IS_TYPE_RGB);
			mon2->Add(is,(128-w)/2+13,(128-h)/2+13);

			mon2->Add(shadow,0,0);
			mon2->Add(background,0,0);
		
			ProgressText p;
			JPEGSaver s(outfn,RefCountPtr<ImageSource>(mon2),90);
			s.SetProgress(&p);
			s.Save();
			free(outfn);
		}
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}
	return(0);
}
#endif

#define TN_SIZE 750

int main(int argc,char **argv)
{
	try
	{
		for(int i=1;i<argc;++i)
		{
			char *outfn=BuildFilename(argv[i],"_tn","jpg");

			ImageSource *is=ISLoadImage(argv[i]);
			int w=TN_SIZE;
			int h=TN_SIZE;
			if((w=((is->width*h)/is->height))>TN_SIZE)
			{
				w=TN_SIZE;
				h=(is->height*w)/is->width;
			}
			is=ISScaleImageBySize(is,w,h);
			ProgressText p;
			JPEGSaver s(outfn,RefCountPtr<ImageSource>(is),90);
			s.SetProgress(&p);
			s.Save();
			free(outfn);
		}
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}
	return(0);
}
