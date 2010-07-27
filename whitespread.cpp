// What's needed is a utility to analyse an image and change the colours surrounding pure white reversed out elements
// to hide the effects of misregistration.

// Two problems:
// Firstly, identify and spread white areas.
// Use a threshold function to take image to pure black/white
// then gaussian blur, and threshold again.
// or possibly leave blurred, scale up by 100%, then and use as an interpolation coefficient.

// Secondly, how to identify colours that can be substituted with pure black.
// Perhaps a heuristic something like this:
// if(k>0.5 && c<0.5 && m<0.5 && y<0.5)
//    k=k+(c+m+y)/3;
// or maybe
// if((c+m+y)<k)
//    k=k+(c+m+y)/3;

#include <iostream>
#include <gtk/gtk.h>

#include "debug.h"
#include "imagesource_util.h"
#include "imagesource_colorantmask.h"
#include "imagesource_cms.h"
#include "pixbuf_from_imagesource.h"
#include "cachedimage.h"
#include "pixbufview.h"
#include "profilemanager.h"
#include "coloranttoggle.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)


#include "imagesource_gaussianblur.h"

class ImageSource_White : public ImageSource
{
	public:
	ImageSource_White(ImageSource *src) : ImageSource(src), source(src)
	{
		if(src->type!=IS_TYPE_CMYK)
			throw "ImageSource_SpreadWhite: Input must currently be CMYK";
		type=IS_TYPE_GREY;
		samplesperpixel=1;
		MakeRowBuffer();
		currentrow=-1;
	}
	~ImageSource_White()
	{
		if(source)
			delete source;
	}
	ISDataType *GetRow(int row)
	{
		if(currentrow==row)
			return(rowbuffer);

		ISDataType *src=source->GetRow(row);
		for(int x=0;x<width;++x)
		{
			int c,m,y,k;
			c=src[x*source->samplesperpixel];
			m=src[x*source->samplesperpixel+1];
			y=src[x*source->samplesperpixel+2];
			k=src[x*source->samplesperpixel+3];
			if((c+m+y+k)==0)
				rowbuffer[x]=IS_SAMPLEMAX;
			else
				rowbuffer[x]=0;
		}

		currentrow=row;
		return(rowbuffer);
	}
	protected:
	ImageSource *source;
};


class ImageSource_WhiteSpread : public ImageSource
{
	public:
	ImageSource_WhiteSpread(ImageSource *src,float radius) : ImageSource(src), source(src), radius(radius)
	{
		source=new ImageSource_White(source);
		source=new ImageSource_GaussianBlur(source,radius);
		type=source->type;
		samplesperpixel=source->samplesperpixel;
		MakeRowBuffer();
		currentrow=-1;
	}
	virtual ~ImageSource_WhiteSpread()
	{
		if(source)
			delete source;
	}
	virtual ISDataType *GetRow(int row)
	{
		if(currentrow==row)
			return(rowbuffer);

		ISDataType *src=source->GetRow(row);

		for(int x=0;x<width*samplesperpixel;++x)
		{
			int t=src[x]*3;
			if(t>IS_SAMPLEMAX)
				t=IS_SAMPLEMAX;
			rowbuffer[x]=t;
		}
		currentrow=row;
		return(rowbuffer);
	}
	protected:
	ImageSource *source;
	float radius;
};


class ImageSource_WhiteTrap : public ImageSource
{
	public:
	ImageSource_WhiteTrap(ImageSource *src,ImageSource *mask) : ImageSource(src), source(src), mask(mask)
	{
		currentrow=-1;
		MakeRowBuffer();
	}
	~ImageSource_WhiteTrap()
	{
		if(source)
			delete source;
		if(mask)
			delete mask;
	}
	ISDataType *GetRow(int row)
	{
		if(row==currentrow)
			return(rowbuffer);

		ISDataType *src=source->GetRow(row);
		ISDataType *msk=mask->GetRow(row);
		for(int x=0;x<width;++x)
		{
			double a=msk[x];
			if(a)
			{
				double c,m,y,k;
				c=src[x*source->samplesperpixel];
				m=src[x*source->samplesperpixel+1];
				y=src[x*source->samplesperpixel+2];
				k=src[x*source->samplesperpixel+3];
				double max=c;
				if(m>max)
					max=m;
				if(y>max)
					max=y;
				double dc=(c-m);
				if(dc<0) dc=-dc;
				double dy=(c-y);
				if(dy<0) dy=-dy;
				if((dc+dy)<(max/3))
				{
					double oc=0, om=0, oy=0, ok=k+(c+m+y)/4.5;
					c=(oc*a+c*(IS_SAMPLEMAX-a))/IS_SAMPLEMAX;
					m=(om*a+m*(IS_SAMPLEMAX-a))/IS_SAMPLEMAX;
					y=(oy*a+y*(IS_SAMPLEMAX-a))/IS_SAMPLEMAX;
					k=(ok*a+k*(IS_SAMPLEMAX-a))/IS_SAMPLEMAX;
					if(k>IS_SAMPLEMAX)
						k=IS_SAMPLEMAX;
					rowbuffer[samplesperpixel*x]=c;
					rowbuffer[samplesperpixel*x+1]=m;
					rowbuffer[samplesperpixel*x+2]=y;
					rowbuffer[samplesperpixel*x+3]=k;
				}
				else
				{
					for(int s=0;s<samplesperpixel;++s)
						rowbuffer[x*samplesperpixel+s]=src[x*samplesperpixel+s];
				}
			}
			else
			{
				for(int s=0;s<samplesperpixel;++s)
					rowbuffer[x*samplesperpixel+s]=src[x*samplesperpixel+s];
			}
		}

		currentrow=row;
		return(rowbuffer);
	}
	protected:
	ImageSource *source;
	ImageSource *mask;
};


class WhiteSpreadTest : public ConfigFile, public ProfileManager
{
	public:
	WhiteSpreadTest() : ConfigFile(), ProfileManager(this,"[Colour Management]"), factory(*this), cached(NULL), colorants(NULL)
	{
		SetInt("DefaultCMYKProfileActive",1);
		tomonitor=factory.GetTransform(CM_COLOURDEVICE_DISPLAY,IS_TYPE_CMYK);

		GtkWidget *win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title (GTK_WINDOW (win), _("PixBufView Test"));
		gtk_signal_connect (GTK_OBJECT (win), "delete_event",
			(GtkSignalFunc) gtk_main_quit, NULL);
		gtk_window_set_default_size(GTK_WINDOW(win),600,400);

		GtkWidget *vbox=gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(win),vbox);
		gtk_widget_show(GTK_WIDGET(vbox));

		pview=pixbufview_new(NULL,false);

		gtk_box_pack_start(GTK_BOX(vbox),pview,TRUE,TRUE,0);
		gtk_widget_show(pview);

		coloranttoggle=coloranttoggle_new(colorants);
		gtk_box_pack_start(GTK_BOX(vbox),coloranttoggle,FALSE,FALSE,0);
		g_signal_connect(G_OBJECT(coloranttoggle),"changed",G_CALLBACK(colorants_changed),this);
		gtk_widget_show(coloranttoggle);

		gtk_widget_show(win);
	}
	~WhiteSpreadTest()
	{
		if(cached)
			delete cached;
	}
	void Process(const char *fn)
	{
		ImageSource *is=ISLoadImage(fn);
		CMSTransform *transform=factory.GetTransform(CM_COLOURDEVICE_DEFAULTCMYK,is);
		is=new ImageSource_CMS(is,transform);
		colorants=new DeviceNColorantList(is->type);
		coloranttoggle_set_colorants(COLORANTTOGGLE(coloranttoggle),colorants);
		if(cached)
			delete cached;
		cached=new CachedImage(is);
		Update();
	}
	void Update()
	{
		if(cached)
		{
			ImageSource *mask=cached->GetImageSource();
			mask=new ImageSource_WhiteSpread(mask,15);

			ImageSource *is=cached->GetImageSource();
			is=new ImageSource_WhiteTrap(is,mask);

			if(colorants)
			{
				is=new ImageSource_ColorantMask(is,colorants);
			}
			is=new ImageSource_CMS(is,tomonitor);
			GdkPixbuf *pb=pixbuf_from_imagesource(is);
			pixbufview_add_page(PIXBUFVIEW(pview),pb);
			g_object_unref(G_OBJECT(pb));
		}
	}
	static void colorants_changed(GtkWidget *wid,gpointer userdata)
	{
		WhiteSpreadTest *t=(WhiteSpreadTest *)userdata;
		t->Update();
	}
	protected:
	GtkWidget *window;
	GtkWidget *pview;
	GtkWidget *coloranttoggle;
	CMTransformFactory factory;
	CMSTransform *tomonitor;
	CachedImage *cached;
	DeviceNColorantList *colorants;
};


int main(int argc,char **argv)
{
	Debug.SetLevel(TRACE);
	gtk_init(&argc,&argv);
	try
	{
		WhiteSpreadTest wst;
		if(argc>1)
		{
			wst.Process(argv[1]);
		}
		gtk_main();
	}
	catch(const char *err)
	{
		Debug[ERROR] << "Error: " << err << std::endl;
	}
}
