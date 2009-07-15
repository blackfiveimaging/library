#include <iostream>
#include <cmath>

#include <gtk/gtk.h>

#include "imagesource/imagesource_util.h"
#include "imagesource/pixbuf_from_imagesource.h"
#include "imagesource/imagesource_histogram.h"
#include "pixbufview.h"

#include "support/progressbar.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)


struct Hist_Shades
{
	int r;
	int g;
	int b;
};


static Hist_Shades Hist_RGBShades[]=
{
	{255,255,255},	// None
	{255,64,64},	// Red
	{64,255,64},	// Green
	{96,144,0},		// Red + Green
	{64,64,255},	// Blue
	{96,0,144},		// Red + Blue
	{0,96,144},		// Green + Blue
	{64,64,96},		// Red + Green + Blue
	{128,128,128},	// As above but with Alpha
	{128,32,32},	// ...
	{32,128,32},
	{48,72,0},
	{32,32,128},
	{48,0,72},
	{0,48,72},
	{32,32,48},
};


static Hist_Shades Hist_CMYKShades[]=
{
	{255,255,255},	// None
	{64,255,255},	// Cyan
	{255,64,255},	// Magenta
	{96,96,255},	// Cyan + Magenta
	{255,255,64},	// Yellow
	{96,255,96},	// Cyan + Yellow
	{255,96,96},	// Magenta + Yellow
	{128,128,64},		// Cyan + Magenta + Yellow
	{128,128,128},	// As above but with Black
	{32,128,128},	// ...
	{128,32,128},
	{48,48,128},
	{128,128,32},
	{48,128,48},
	{128,48,48},
	{72,72,48}
};


// DrawHistogram() - renders a GdkPixbuf from a histogram.
GdkPixbuf *DrawHistogram(ISHistogram &histogram)
{
	Hist_Shades *shades;
	switch(histogram.GetType())
	{
		case IS_TYPE_RGB:
		case IS_TYPE_RGBA:
			cerr << "Drawing histogram for RGB Image" << endl;
			shades=Hist_RGBShades;
			break;
		case IS_TYPE_CMYK:
			cerr << "Drawing histogram for CMYK Image" << endl;
			shades=Hist_CMYKShades;
			break;
		default:
			throw "Unknown histogram type";
			break;
	}
	int channels=histogram.GetChannelCount();
	int height=256;
	GdkPixbuf *pb=gdk_pixbuf_new(GDK_COLORSPACE_RGB,FALSE,8,IS_HISTOGRAM_BUCKETS,height);

	if(pb)
	{
		int rowstride=gdk_pixbuf_get_rowstride(pb);
		unsigned char *pixels=gdk_pixbuf_get_pixels(pb);
		double max=histogram.GetMax();

		for(int x=0;x<IS_HISTOGRAM_BUCKETS;++x)
		{
			for(int y=0;y<height;++y)
			{
				int bit=1;
				int ci=0;
				for(int c=0;c<channels;++c)
				{
					double t=histogram[c][x];
					t/=max;
					t=sqrt(t);
					if((height-y)<(255.0*t))
						ci|=bit;
					bit<<=1;
				}
				int pi=3*x+y*rowstride;
				pixels[pi]=shades[ci].r;
				pixels[pi+1]=shades[ci].g;
				pixels[pi+2]=shades[ci].b;
			}
		}
	}
	return(pb);
}



static struct {double r; double g; double b; double a;} RGBShades[]=
{
	{1.0,0,0,1.0},
	{0,1.0,0,0.8},
	{0,0,1.0,0.6},
	{0.5,0.5,0.6}
};


static struct {double r; double g; double b; double a;} CMYKShades[]=
{
	{0,1.0,1.0,1.0},
	{1.0,0,1.0,0.8},
	{1.0,1.0,0,0.6},
	{0,0,0,0.4},
	{0.5,0.5,0.5,0.2}
};


static void paint_histogram(GtkWidget *widget,GdkEventExpose *eev,gpointer userdata)
{
	ISHistogram *hist=(ISHistogram *)userdata;

	int width  = widget->allocation.width;
	int height = widget->allocation.height;

	cairo_t *cr = gdk_cairo_create (widget->window);
 
	cairo_set_source_rgb (cr, 1,1,1);
	cairo_paint (cr);

	cairo_set_source_rgb (cr, .9,.9,.9);
	cairo_set_line_width(cr,1.0);
	cairo_move_to (cr, width/4,0);
	cairo_line_to(cr,width/4,height);

	cairo_move_to (cr, width/2,0);
	cairo_line_to(cr,width/2,height);

	cairo_move_to (cr, 3*width/4,0);
	cairo_line_to(cr,3*width/4,height);

	cairo_move_to (cr, 0, height/2);
	cairo_line_to(cr,width,height/2);

	cairo_move_to (cr,0,height/4);
	cairo_line_to(cr,width,height/4);

	cairo_move_to (cr,0,height/2);
	cairo_line_to(cr,width,height/2);

	cairo_move_to (cr,0,3*height/4);
	cairo_line_to(cr,width,3*height/4);

	cairo_stroke (cr);

	int channels=hist->GetChannelCount();
	for(int chan=0;chan<channels;++chan)
	{
		switch(STRIP_ALPHA(hist->GetType()))
		{
			case IS_TYPE_RGB:
				cairo_set_source_rgba (cr,RGBShades[chan].r,RGBShades[chan].g,RGBShades[chan].b,RGBShades[chan].a);
				break;
			case IS_TYPE_CMYK:
				cairo_set_source_rgba (cr,CMYKShades[chan].r,CMYKShades[chan].g,CMYKShades[chan].b,CMYKShades[chan].a);
				break;
			default:
				cairo_set_source_rgba(cr,0,0,0,0.6);
				break;
		}
		ISHistogram_Channel &hc=(*hist)[chan];
		double max=hist->GetMax();
		for(int x=0;x<width;++x)
		{
			int i=(x*IS_HISTOGRAM_BUCKETS)/width;
			double t=hc[i];
			t/=max;		// Range is now 0 - 1.0
			t=sqrt(t);

			cairo_move_to (cr,x,height);
			cairo_line_to(cr,x,height-height*t);
			cairo_stroke (cr);
		}
	}

	cairo_destroy (cr);
}


using namespace std;

int main(int argc,char**argv)
{
	gtk_init(&argc,&argv);

	try
	{
		GtkWidget *win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title (GTK_WINDOW (win), _("PixBufView Test"));
		gtk_signal_connect (GTK_OBJECT (win), "delete_event",
			(GtkSignalFunc) gtk_main_quit, NULL);

		GtkWidget *notebook=gtk_notebook_new();
		gtk_container_add(GTK_CONTAINER(win),notebook);
		gtk_widget_show(GTK_WIDGET(notebook));


		GtkWidget *label=gtk_label_new("Image");
		gtk_widget_show(label);

		GtkWidget *pview=pixbufview_new(NULL,false);

		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),GTK_WIDGET(pview),GTK_WIDGET(label));
		gtk_widget_show(pview);
		gtk_widget_show(win);


		ISHistogram *hist=NULL;

		if(argc>1)
		{
			ImageSource *is=ISLoadImage(argv[1]);
			hist=new ISHistogram();
			is=new ImageSource_Histogram(is,*hist);
			GdkPixbuf *pb=pixbuf_from_imagesource(is);
			delete is;

			pixbufview_set_pixbuf(PIXBUFVIEW(pview),pb);
			g_object_unref(G_OBJECT(pb));
		}

		// Histogram view widget

		label=gtk_label_new("Histogram");
		gtk_widget_show(label);

		GtkWidget *canvas = gtk_drawing_area_new();
		gtk_widget_set_size_request(canvas, 300, 200);
		g_signal_connect(G_OBJECT (canvas), "expose-event",
		                G_CALLBACK (paint_histogram), hist);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),GTK_WIDGET(canvas),GTK_WIDGET(label));
		gtk_widget_show(canvas);


		label=gtk_label_new("Histogram 2");
		gtk_widget_show(label);
		GtkWidget *pview2=pixbufview_new(NULL,false);

		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),GTK_WIDGET(pview2),GTK_WIDGET(label));
		gtk_widget_show(pview2);

		GdkPixbuf *pb=DrawHistogram(*hist);
		pixbufview_set_pixbuf(PIXBUFVIEW(pview2),pb);

		gtk_main();

		if(hist)
			delete hist;
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}

	return(0);
}

