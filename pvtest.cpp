#include <iostream>
#include <cmath>

#include <gtk/gtk.h>

#include "support/debug.h"

#include "imagesource/imagesource_util.h"
#include "imagesource/pixbuf_from_imagesource.h"
#include "imagesource/imagesource_histogram.h"
#include "simplecombo.h"
#include "pixbufview.h"

#include "progressbar.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)


static void change_page(GtkWidget *widget,gpointer userdata)
{
	int idx=simplecombo_get_index(SIMPLECOMBO(widget));
	pixbufview_set_page(PIXBUFVIEW(userdata),idx);
}


static void mouse_move(GtkWidget *widget,gpointer userdata)
{
	PixbufView *pv=(PixbufView *)userdata;
	int x=pixbufview_get_mousex(pv);
	int y=pixbufview_get_mousey(pv);
	cerr << "Mouse position: " << x << ", " << y << endl;
}


using namespace std;

int main(int argc,char**argv)
{
	Debug.SetLevel(TRACE);

	gtk_init(&argc,&argv);

	try
	{
		GtkWidget *win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title (GTK_WINDOW (win), _("PixBufView Test"));
		gtk_signal_connect (GTK_OBJECT (win), "delete_event",
			(GtkSignalFunc) gtk_main_quit, NULL);

		GtkWidget *vbox=gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(win),vbox);
		gtk_widget_show(GTK_WIDGET(vbox));

		GtkWidget *pview=pixbufview_new(NULL,false);
		g_signal_connect(G_OBJECT(pview),"mousemove",G_CALLBACK(mouse_move),pview);

		gtk_box_pack_start(GTK_BOX(vbox),pview,TRUE,TRUE,0);
		gtk_widget_show(pview);
		gtk_widget_show(win);

		SimpleComboOptions opts;

		for(int i=1;i<argc;++i)
		{
			ImageSource *is=ISLoadImage(argv[i]);
			GdkPixbuf *pb=pixbuf_from_imagesource(is);
			delete is;

			pixbufview_add_page(PIXBUFVIEW(pview),pb);
			g_object_unref(G_OBJECT(pb));

			opts.Add("",argv[i]);
		}

		GtkWidget *combo=simplecombo_new(opts);
		g_signal_connect(G_OBJECT(combo),"changed",G_CALLBACK(change_page),pview);
		gtk_box_pack_start(GTK_BOX(vbox),combo,FALSE,FALSE,0);
		gtk_widget_show(combo);

		gtk_main();

	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}

	return(0);
}

