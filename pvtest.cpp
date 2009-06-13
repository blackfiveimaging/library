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
			hist=new ISHistogram(is);
			is=new ImageSource_Histogram(is,*hist);
			GdkPixbuf *pb=pixbuf_from_imagesource(is);
			pixbufview_set_pixbuf(PIXBUFVIEW(pview),pb);
//			g_object_unref(G_OBJECT(pview));
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

