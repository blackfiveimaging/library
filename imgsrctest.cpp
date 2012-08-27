#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "cachedimage.h"
#include "imagesource_util.h"
#include "imagesource_dither.h"
#include "imagesource_solid.h"
#include "imagesource_cms.h"
#include "imagesource_desaturate.h"
#include "imagesource_rotate.h"
#include "imagesource_bilinear.h"
#include "imagesource_lanczossinc.h"
#include "imagesource_downsample.h"
#include "imagesource_convolution.h"
#include "imagesource_gaussianblur.h"
#include "imagesource_unsharpmask.h"
#include "imagesource_devicen_preview.h"
#include "imagesource_devicen_remap.h"
#include "imagesource_montage.h"
#include "imagesource_scaledensity.h"
#include "imagesource_gamma.h"
#include "imagesource_chequerboard.h"
#include "imagesource/imagesource_dropshadow.h"
#include "imagesource_hsweep.h"
#include "imagesource_hticks.h"
#include "jpegsaver.h"
#include "tiffsaver.h"
#include "progresstext.h"
#include "profilemanager/profilemanager.h"

#if 0
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

			r=128*a+r*(1-a);
			g=128*a+g*(1-a);
			b=128*a+b*(1-a);

			rowbuffer[x*3]=EIGHTTOIS(r);
			rowbuffer[x*3+1]=EIGHTTOIS(g);
			rowbuffer[x*3+2]=EIGHTTOIS(b);
		}
		
		currentrow=row;
		return(rowbuffer);
	}
};


int main(int argc,char **argv)
{
	try
	{
		if(argc<2)
			return(0);
		ImageSource *is=new ImageSource_RainbowSweep(1024,300);
		JPEGSaver js(argv[1],RefCountPtr<ImageSource>(is));
		js.Save();
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}
	return(0);
}
#endif

#if 0

class Test : public ConfigFile, public ProfileManager
{
	public:
	Test() : ConfigFile(), ProfileManager(this,"[ColourManagement]"), factory(*this)
	{
	}
	~Test()
	{
	}
	void ShowParasites(ImageSource *is)
	{
		if(is)
		{
			RefCountPtr<ISParasite> p=is->GetParasite(ISPARATYPE_PSIMAGERESOURCEBLOCK);
			if(p)
				Debug[TRACE] << "Got clipping path parasite" << std::endl;
			else
				Debug[TRACE] << "No clipping path parasite" << std::endl;
		}
	}
	void ProcessImage(const char *fn)
	{
		RefCountPtr<char> tmp=RefCountPtr<char>(BuildFilename(fn,"-test","tif"));
		ImageSource *is=ISLoadImage(fn);
		ShowParasites(is);

		CachedImage cached((ImageSource_rp(is)));
		is=cached.GetImageSource();

		RefCountPtr<CMSTransform> trans;
		RefCountPtr<CMSProfile>emb=is->GetEmbeddedProfile();
		if(emb)
			trans=factory.GetTransform(CM_COLOURDEVICE_DISPLAY,&*emb);
		else
			trans=factory.GetTransform(CM_COLOURDEVICE_DISPLAY,is->type);
		is=new ImageSource_CMS(is,trans);
//		is=new ImageSource_DropShadow(is,20,5);

		ProgressText prog;
		TIFFSaver ts(&*tmp,ImageSource_rp(is));
		ts.SetProgress(&prog);
		ts.Save();
	}
	protected:
	CMTransformFactory factory;
};


int main(int argc,char **argv)
{
	try
	{
		if(argc<2)
			return(0);
		Test test;
		test.ProcessImage(argv[1]);
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}
	return(0);
}
#endif

#if 0

// Render a sweep pattern for approximate gamma determination.
// This doesn't need to be accurate (which is just as well, because it isn't!)
// - just good enough to give us an approximate gamma so the test point when
// printing the linearization charts are reasonably well-distributed.

ImageSource *MakeGammaEvalSweep(DeviceNColorantList &colorants,int width,int height,int res)
{
	int cols=colorants.GetColorantCount();
	int stripeheight=height/3;

	// We search for black, and throw an error if not found...
	int blk=-1;
	blk=colorants.GetColorantIndex("Black");
	if(blk==-1)
		blk=colorants.GetColorantIndex("Matte Black");
	if(blk==-1)
		throw "Error: Can't find black ink!";

	ISDeviceNValue bg(cols,0);	// Background colour
	ISDeviceNValue fg(cols,0);	// Foreground for ticks.
	fg[blk]=IS_SAMPLEMAX;	// Render ticks in black

	ImageSource_Montage *mon=new ImageSource_Montage(IS_TYPE_DEVICEN,res,cols);
	ImageSource_HTicks *ticks=new ImageSource_HTicks(width,stripeheight,IS_TYPE_DEVICEN,cols);
	ticks->SetBG(bg);
	ticks->SetFG(fg);
	mon->Add(ticks,0,0);	// Now owned by the montage

	ImageSource_HSweep *sweep = new ImageSource_HSweep(width,stripeheight,bg,fg,IS_TYPE_DEVICEN);
	mon->Add(sweep,0,stripeheight);	// Now owned by the montage
	
	ImageSource_Chequerboard *cb = new ImageSource_Chequerboard(width,stripeheight,IS_TYPE_DEVICEN,cols,4);
	cb->SetBG(bg);
	cb->SetFG(fg);
	mon->Add(cb,0,stripeheight*2);	// Now owned by the montage
	return(mon);
}


// Render a density evaluation pattern - this pattern should be printed at reduced density in
// Raw mode to determine absolute maximum ink levels for individual ink channels.

ImageSource *MakeDensityEvalSweep(DeviceNColorantList &colorants,int width,int height,int res)
{
	int cols=colorants.GetColorantCount();
	int stripeheight=(2*height)/(cols+1);	// We'll render each pass as stripeheight pixels
											// and the row of "ticks" at half that height

	// We search for black, and throw an error if not found...
	int blk=-1;
	blk=colorants.GetColorantIndex("Black");
	if(blk==-1)
		blk=colorants.GetColorantIndex("Matte Black");
	if(blk==-1)
		throw "Error: Can't find black ink!";

	ISDeviceNValue bg(cols,0);	// Background colour
	ISDeviceNValue tfg(cols,0);	// Foreground for ticks.
	tfg[blk]=IS_SAMPLEMAX;	// Render ticks in black

	ImageSource_Montage *mon=new ImageSource_Montage(IS_TYPE_DEVICEN,res,cols);
	ImageSource_HTicks *ticks=new ImageSource_HTicks(width/2,stripeheight/2,IS_TYPE_DEVICEN,cols);
	ticks->SetBG(bg);
	ticks->SetFG(tfg);
	mon->Add(ticks,0,0);	// Now owned by the montage

	ticks=new ImageSource_HTicks(width/2,stripeheight/2,IS_TYPE_DEVICEN,cols);
	ticks->SetBG(bg);
	ticks->SetFG(tfg);
	mon->Add(ticks,width/2,0);

	for(int i=0;i<cols;++i)
	{
		ISDeviceNValue sfg(cols,0);
		sfg[i]=IS_SAMPLEMAX;
		ImageSource *is=new ImageSource_HSweep(width/2,stripeheight,bg,sfg,IS_TYPE_DEVICEN);
		mon->Add(is,(width/2)*(i&1),stripeheight*(i/2)+stripeheight/2);
	}
	return(mon);
}


int main(int argc,char **argv)
{
	try
	{
		if(argc==2)
		{
			DeviceNColorantList colorants;
			new DeviceNColorant(colorants,"Yellow");
			new DeviceNColorant(colorants,"Black");
			new DeviceNColorant(colorants,"Light Cyan");
			new DeviceNColorant(colorants,"Light Magenta");
			new DeviceNColorant(colorants,"Magenta");
			new DeviceNColorant(colorants,"Cyan");

//			ImageSource *is=MakeDensityEvalSweep(colorants,1140,200,144);
			ImageSource *is=MakeGammaEvalSweep(colorants,1140,90,144);

			ImageSource_DeviceN_Preview dst(is,&colorants);

			TIFFSaver ts(argv[1],&dst);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
		}
		if(argc==4)
		{
			ImageSource *is=ISLoadImage(argv[1]);
			int pc=atoi(argv[2]);
			ImageSource_Downsample dst(is,(is->width*pc)/100,(is->height*pc)/100);
			TIFFSaver ts(argv[3],&dst);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
		}
		if(argc==5)
		{
			ImageSource *is=ISLoadImage(argv[1]);
			int pc=atoi(argv[2]);
			float gamma=atof(argv[3]);
			is=new ImageSource_Gamma(is,gamma);
			is=new ImageSource_Downsample(is,(is->width*pc)/100,(is->height*pc)/100);
			ImageSource_Gamma dst(is,1.0/gamma);
			TIFFSaver ts(argv[4],&dst);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
		}
#endif
#if 0
		if(argc==3)
		{
			int channelmap[]={5,4,0,1,2,3};

			DeviceNColorantList colorants;
			new DeviceNColorant(colorants,"Yellow");
			new DeviceNColorant(colorants,"Black");
			new DeviceNColorant(colorants,"Light Cyan");
			new DeviceNColorant(colorants,"Light Magenta");
			new DeviceNColorant(colorants,"Magenta");
			new DeviceNColorant(colorants,"Cyan");
			new DeviceNColorant(colorants,"Light Black");

			ImageSource *is=ISLoadImage(argv[1]);
			is=new ImageSource_DeviceN_Remap(is,channelmap);
			ImageSource_DeviceN_Preview dst(is,&colorants);
			TIFFSaver ts(argv[2],&dst);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
		}
		if(argc==5)
		{
			ImageSource *is=ISLoadImage(argv[1]);
			int w=atoi(argv[2]);
			int h=atoi(argv[3]);
			ImageSource_Downsample dst(is,w,h);
			TIFFSaver ts(argv[4],&dst);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
		}
#endif
#if 0
		if(argc==4)
		{
			int tempchange=atoi(argv[2]);
			ImageSource *is=ISLoadImage(argv[1]);

			CMSRGBGamma RGBGamma(2.2,2.2,2.2);
			CMSGamma	GreyGamma(2.2);
			CMSWhitePoint srcwp(6500+tempchange);
			CMSWhitePoint dstwp(6500);
			CMSProfile *source;
			switch(STRIP_ALPHA(is->type))
			{
				case IS_TYPE_RGB:
					is=new ImageSource_Desaturate(is);
					source=new CMSProfile(CMSPrimaries_Rec709,RGBGamma,srcwp);
					break;
				case IS_TYPE_GREY:
					source=new CMSProfile(GreyGamma,srcwp);
					break;
				default:
					throw "Only RGB and Greyscale images are currently supported";
			}
			if(!source)
				throw "Can't create source profile";

			CMSProfile dest(CMSPrimaries_Rec709,RGBGamma,dstwp);
			CMSTransform transform(source,&dest,LCMSWRAPPER_INTENT_ABSOLUTE_COLORIMETRIC);
			is=new ImageSource_CMS(is,&transform);
			delete source;
			TIFFSaver ts(argv[3],is);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;
		}
#endif
#if 0
		if(argc==5)
		{
			ImageSource *is=ISLoadImage(argv[1]);
			int w=atoi(argv[2]);
			int h=atoi(argv[3]);
			is=new ImageSource_Downsample(is,w,h);
			TIFFSaver ts(argv[4],is);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;
		}
		if(argc==4)
		{
			double pct=atof(argv[2]);
			ImageSource *is=ISLoadImage(argv[1]);
			cerr << "Original size: " << is->width << " by " << is->height << endl;
			is=new ImageSource_Downsample(is,int((is->width*pct)/100.0),int((is->height*pct)/100.0));
			TIFFSaver ts(argv[3],is);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;
		}

		if(argc==6)
		{
			double radius=atof(argv[2]);
			double amount=atof(argv[3]);
			double threshold=atof(argv[4]);
			ImageSource *is=ISLoadImage(argv[1]);
//			ConvKernel_Gaussian kernel(radius);
//			ConvKernel_UnsharpMask kernel(radius,amount);
//			kernel.Normalize();
//			is=new ImageSource_Convolution(is,&kernel);
			is=new ImageSource_UnsharpMask(is,radius,amount,threshold);
			TIFFSaver ts(argv[5],is);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;
		}
		if(argc==3)
		{
			ImageSource *is=ISLoadImage(argv[1]);
			is=new ImageSource_GaussianBlur(is,5);
			TIFFSaver ts(argv[2],is,false);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;	
		}
		if(argc==4)
		{
			ImageSource *is=ISLoadImage(argv[1]);
			is=new ImageSource_GaussianBlur(is,atof(argv[2]));
			TIFFSaver ts(argv[3],is,false);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;	
		}
		if(argc==5)
		{
			ImageSource *is=ISLoadImage(argv[1]);
//			is=new ImageSource_Downsample(is,(is->width*95)/100,(is->height*95)/100);
//			is=new ImageSource_LanczosSinc(is,atoi(argv[2]),atoi(argv[3]));
			int w=atoi(argv[2]);
			int h=atoi(argv[3]);
			float radius=float(is->width)/w;
//			cerr << "Using Gaussian radius of: " << radius << endl;
//			is=new ImageSource_GaussianBlur(is,radius);
			is=ISScaleImageBySize(is,w,h);
			TIFFSaver ts(argv[4],is,false);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;	
		}
		if(argc==2)
		{
			// Rescue routine for CM screwup!
			ImageSource *is=ISLoadImage(argv[1]);
			
			int prevrow[3]={-1,-1,-1};
			int row=0;
			while(row<is->height)
			{
				ISDataType *rowptr;
				do
				{
					rowptr=is->GetRow(row);
					++row;
				} while((row<is->height)
					&& (rowptr[0]==prevrow[0])
					&& (rowptr[1]==prevrow[1])
					&& (rowptr[2]==prevrow[2]));
				if(row<is->height)
				{
					cerr << "Unique row at " << row << endl;
					prevrow[0]=rowptr[0];
					prevrow[1]=rowptr[1];
					prevrow[2]=rowptr[2];
					int x=0;
					int tmp[3]={-1,-1,-1};
					while(x<is->width)
					{
						int s=x*is->samplesperpixel;
						if(rowptr[s]!=tmp[0] || rowptr[s+1]!=tmp[1] || rowptr[s+2]!=tmp[2])
						{
							tmp[0]=rowptr[s];
							tmp[1]=rowptr[s+1];
							tmp[2]=rowptr[s+2];
							float r=(100.0 * tmp[0])/IS_SAMPLEMAX;
							float g=(100.0 * tmp[1])/IS_SAMPLEMAX;
							float b=(100.0 * tmp[2])/IS_SAMPLEMAX;
							cout << r << ", " << g << ", " << b << endl;
							cerr << r << ", " << g << ", " << b << endl;
						}
						++x;
					}
				}
			}
			delete is;	
		}
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}
	return(0);
}
#endif


int main(int argc,char **argv)
{
	try
	{
		if(argc<2)
			return(0);
		ImageSource *is=ISLoadImage(argv[1]);
		cerr << "File has " << is->samplesperpixel << " spp"<< std::endl;
		for(int y=0;y<is->height;++y)
		{
			ISDataType *row=is->GetRow(y);
			for(int x=0;x<is->width;++x)
			{
				ISDataType r,g,b;
				r=*row++;
				g=*row++;
				b=*row++;
				int out=(r&0xf800)|((g>>5)&0x0Fe0)|((b>>11)&0x1f);
				putc(out>>8,stdout);
				putc(out&255,stdout);
			}
		}
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}
	return(0);
}

