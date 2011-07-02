#include <iostream>
#include <cmath>

#include <gtk/gtk.h>

#include "support/debug.h"

#include "imagesource/imagesource_util.h"
#include "imagesource/pixbuf_from_imagesource.h"
#include "imagesource/imagesource_histogram.h"
#include "imagesource/imagesource_hreflect.h"
#include "imagesource/imagesource_vreflect.h"
#include "imagesource/imagesource_dropshadow.h"
#include "imagesource/imagesource_cms.h"
#include "imagesource/imagesource_dither.h"
#include "simplecombo.h"
#include "simplelistview.h"
#include "pixbufview.h"

#include "pixbufthumbnail/egg-pixbuf-thumbnail.h"
#include "profilemanager/profilemanager.h"

#include "progressbar.h"

//#include "dtwidgets/slider.h"
//#include "dtwidgets/slider.c"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)


#if 1
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
//			is=new ImageSource_HReflect(is);
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
#endif
#if 0
using namespace std;
// Simple image viewer...
class PVTest : public SimpleListViewOptions
{
	public:
	PVTest() : SimpleListViewOptions(), pview(NULL), listview(NULL)
	{
		GtkWidget *win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title (GTK_WINDOW (win), _("PixBufView Test"));
		gtk_signal_connect (GTK_OBJECT (win), "delete_event",
			(GtkSignalFunc) gtk_main_quit, NULL);

		GtkWidget *hbox=gtk_hbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(win),hbox);
		gtk_widget_show(GTK_WIDGET(hbox));

		pview=pixbufview_new(NULL,false);
//		g_signal_connect(G_OBJECT(pview),"mousemove",G_CALLBACK(mouse_move),pview);

		listview=simplelistview_new(NULL);
		g_signal_connect(G_OBJECT(listview),"double-clicked",G_CALLBACK(doubleclicked),this);
		gtk_box_pack_start(GTK_BOX(hbox),listview,FALSE,FALSE,0);
		gtk_widget_show(listview);

		gtk_box_pack_start(GTK_BOX(hbox),pview,TRUE,TRUE,0);
		gtk_widget_show(pview);
		gtk_widget_show(win);
	}
	~PVTest()
	{
	}
	void Populate()
	{
		simplelistview_set_opts(SIMPLELISTVIEW(listview),this);
	}
	void AddImage(const char *fn)
	{
		ImageSource *is=ISLoadImage(fn);
		if(STRIP_ALPHA(is->type==IS_TYPE_RGB))
		{
			int h=96;
			int w=(is->width*96)/is->height;
			if(w>96)
			{
				w=96;
				h=(is->height*96)/is->width;
			}
			is=ISScaleImageBySize(is,w,h);
			is=new ImageSource_DropShadow(is,5,3);
			GdkPixbuf *pb=pixbuf_from_imagesource(is);
			delete is;

			Add(fn,NULL,fn,pb,this);
			g_object_unref(G_OBJECT(pb));
		}
	}
	void SetImage(const char *fn)
	{
		ImageSource *is=ISLoadImage(fn);
//		is=new ImageSource_DropShadow(is,20,5);
		GdkPixbuf *pb=pixbuf_from_imagesource(is);
		delete is;

		

		pixbufview_add_page(PIXBUFVIEW(pview),pb);
		g_object_unref(G_OBJECT(pb));
	}
	static void doubleclicked(GtkWidget *wid,gpointer userdata)
	{
		PVTest *pv=(PVTest *)userdata;
		Debug[TRACE] << "Fetching index of selected row" << endl;
		int idx=simplelistview_get_index(SIMPLELISTVIEW(pv->listview));
		Debug[TRACE] << "Got index " << idx << endl;
		SimpleListViewOption *opt=(*pv)[idx];
		Debug[TRACE] << "Setting filename to " << opt->displayname << endl;
		pv->SetImage(opt->key);
	}
	protected:
	GtkWidget *window;
	GtkWidget *pview;
	GtkWidget *listview;
};


int main(int argc,char**argv)
{
	Debug.SetLevel(TRACE);

	gtk_init(&argc,&argv);

	try
	{
		PVTest test;
		for(int i=1;i<argc;++i)
			test.AddImage(argv[i]);
		test.Populate();

		gtk_main();

	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}

	return(0);
}

#endif

#if 0
// Testing darktable's slider widgets.
class PVTest : public ConfigFile, public ProfileManager
{
	public:
	PVTest() : ConfigFile(), ProfileManager(this,"[ColourManagement]"), factory(*this), pview(NULL)
	{
		GtkWidget *win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title (GTK_WINDOW (win), _("PixBufView Test"));
		gtk_signal_connect (GTK_OBJECT (win), "delete_event",
			(GtkSignalFunc) gtk_main_quit, NULL);

		GtkWidget *vbox=gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(win),vbox);
		gtk_widget_show(GTK_WIDGET(vbox));

		pview=pixbufview_new(NULL,false);
//		g_signal_connect(G_OBJECT(pview),"mousemove",G_CALLBACK(mouse_move),pview);

		gtk_box_pack_start(GTK_BOX(vbox),pview,TRUE,TRUE,0);
		gtk_widget_show(pview);
		gtk_widget_show(win);
	}
	~PVTest()
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
	void SetImage(const char *fn)
	{
		ImageSource *is=ISLoadImage(fn);
		ShowParasites(is);
		RefCountPtr<CMSTransform> trans;
		RefCountPtr<CMSProfile>emb=is->GetEmbeddedProfile();
		if(emb)
			trans=factory.GetTransform(CM_COLOURDEVICE_DISPLAY,&*emb);
		else
			trans=factory.GetTransform(CM_COLOURDEVICE_DISPLAY,is->type);
		is=new ImageSource_CMS(is,trans);
//		is=new ImageSource_Dither(is,8);
		is=new ImageSource_DropShadow(is,20,5);
		ShowParasites(is);
		GdkPixbuf *pb=pixbuf_from_imagesource(is);
		delete is;

		pixbufview_add_page(PIXBUFVIEW(pview),pb);
		g_object_unref(G_OBJECT(pb));
	}
	protected:
	CMTransformFactory factory;
	GtkWidget *window;
	GtkWidget *pview;
};
#endif

