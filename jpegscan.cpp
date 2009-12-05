#include <iostream>
#include <cmath>
#include <cstdio>

#include <gtk/gtk.h>

#include "imagesource/imagesource_util.h"
#include "imagesource/pixbuf_from_imagesource.h"
#include "imagesource/imagesource_jpeg.h"
#include "pixbufview.h"

#include "progressbar.h"

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

		GtkWidget *notebook=gtk_notebook_new();
		gtk_container_add(GTK_CONTAINER(win),notebook);
		gtk_widget_show(GTK_WIDGET(notebook));


		GtkWidget *label=gtk_label_new("Image");
		gtk_widget_show(label);

		GtkWidget *pview=pixbufview_new(NULL,false);

		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),GTK_WIDGET(pview),GTK_WIDGET(label));
		gtk_widget_show(pview);
		gtk_widget_show(win);

		if(argc>1)
		{
			FILE *f=fopen(argv[1],"rb");
			if(f)
			{
				ProgressBar pb("Scanning file...",win);
				int counter=0;
				bool cont=true;
				while(cont && !feof(f))
				{
					if((fgetc(f)==0xff) && (fgetc(f)==0xd8))
					{
						cerr << "Image found..." << endl;
						fseek(f,-2,SEEK_CUR);
						ImageSource *is=NULL;
						try
						{
							is=new ImageSource_JPEG(f);
							try
							{
								GdkPixbuf *pb=pixbuf_from_imagesource(is);
								pixbufview_set_pixbuf(PIXBUFVIEW(pview),pb);
								g_object_unref(G_OBJECT(pb));
							}
							catch(const char *err)
							{
								cerr << "Error: " << err << endl;
							}
							delete is;
						}
						catch(const char *err)
						{
							cerr << "Error: " << err << endl;
						}
					}
					++counter;
					if((counter&1048575)==0)
						cont=pb.DoProgress(0,0);
				}
			}
		}

		gtk_main();
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}

	return(0);
}

