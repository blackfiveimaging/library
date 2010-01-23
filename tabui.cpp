#include <iostream>

#include <gtk/gtk.h>

#include "miscwidgets/uitab.h"

#include "miscwidgets/pixbuf_from_imagedata.h"
#include "miscwidgets/spinner1.cpp"
#include "miscwidgets/spinner2.cpp"
#include "miscwidgets/spinner3.cpp"
#include "miscwidgets/spinner4.cpp"
#include "miscwidgets/spinner5.cpp"
#include "miscwidgets/spinner6.cpp"
#include "miscwidgets/spinner7.cpp"
#include "miscwidgets/spinner8.cpp"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)

using namespace std;


class Spinner
{
	public:
	Spinner()
	{
		frames[0]=PixbufFromImageData(spinner1_data,sizeof(spinner1_data));
		frames[1]=PixbufFromImageData(spinner2_data,sizeof(spinner2_data));
		frames[2]=PixbufFromImageData(spinner3_data,sizeof(spinner3_data));
		frames[3]=PixbufFromImageData(spinner4_data,sizeof(spinner4_data));
		frames[4]=PixbufFromImageData(spinner5_data,sizeof(spinner5_data));
		frames[5]=PixbufFromImageData(spinner6_data,sizeof(spinner6_data));
		frames[6]=PixbufFromImageData(spinner7_data,sizeof(spinner7_data));
		frames[7]=PixbufFromImageData(spinner8_data,sizeof(spinner8_data));
		spinner=gtk_image_new();
		g_object_ref(G_OBJECT(spinner));
		gtk_widget_show(spinner);
		SetFrame(0);
	}
	~Spinner()
	{
		for(int i=0;i<8;++i)
		{
			if(frames[i])
				g_object_unref(frames[i]);
		}
		if(spinner)
			gtk_widget_destroy(spinner);
		g_object_unref(G_OBJECT(spinner));
	}
	void SetFrame(int f)
	{
		f=f&7;	// clamp to range 0-7
		gtk_image_set_from_pixbuf(GTK_IMAGE(spinner),frames[f]);
	}
	GtkWidget *GetWidget()
	{
		return(spinner);
	}
	protected:
	GdkPixbuf *frames[8];
	GtkWidget *spinner;
};


class UITab;

class TestUI
{
	public:
	TestUI();
	~TestUI();
	GtkWidget *window;
	GtkWidget *notebook;
	protected:
	friend class UITab;
};


TestUI::TestUI() : notebook(NULL)
{
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window),600,450);
	gtk_window_set_title (GTK_WINDOW (window), _("TestUI"));
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		(GtkSignalFunc) gtk_main_quit, NULL);
	gtk_widget_show(window);


	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),hbox);
	gtk_widget_show(hbox);

	notebook=gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook),true);
	gtk_box_pack_start(GTK_BOX(hbox),notebook,TRUE,TRUE,0);
	gtk_widget_show(GTK_WIDGET(notebook));
}

TestUI::~TestUI()
{
}


gboolean timeoutfunc(gpointer userdata)
{
	Spinner *s=(Spinner *)userdata;
	static int frame=0;
	s->SetFrame(frame);
	++frame;
	return(TRUE);
}


int main(int argc,char **argv)
{
	gtk_init(&argc,&argv);
	try
	{
		TestUI ui;
		Spinner spinner;

		new UITab(ui.notebook,"Tab 1");
		(new UITab(ui.notebook))->AddTabWidget(spinner.GetWidget());
		(new UITab(ui.notebook))->SetTabText("Tab 2");
		(new UITab(ui.notebook))->AddTabButton(gtk_button_new_with_label("Hi!"));
		new UITab(ui.notebook);

		g_timeout_add(100,timeoutfunc,&spinner);

		gtk_main();
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}
	return(0);
}

