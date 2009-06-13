#include <iostream>

#include <gtk/gtk.h>

#include "imagesource/imagesource_util.h"
#include "imagesource/pixbuf_from_imagesource.h"
#include "imagesource/imagesource_histogram.h"
#include "pixbufview.h"

#include "support/progressbar.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)


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

		GtkWidget *pview=pixbufview_new(NULL,false);
		gtk_container_add (GTK_CONTAINER (win), pview);
		gtk_widget_show(pview);
		gtk_widget_show(win);

		ISHistogram *hist=NULL;
		int channels;

		if(argc>1)
		{
			ImageSource *is=ISLoadImage(argv[1]);
			channels=is->samplesperpixel;
			hist=new ISHistogram(channels);
			is=new ImageSource_Histogram(is,*hist);
			GdkPixbuf *pb=pixbuf_from_imagesource(is);
			pixbufview_set_pixbuf(PIXBUFVIEW(pview),pb);
//			g_object_unref(G_OBJECT(pview));
		}

		if(hist)
		{
			for(int chan=0;chan<channels;++chan)
			{
				cerr << "Channel " << chan << endl;
				ISHistogram_Channel &hc=(*hist)[chan];
				for(int i=0;i<IS_HISTOGRAM_BUCKETS;++i)
				{
					cerr << "  " << i << " : " << hc[i] << endl;
				}
			}
		}

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

