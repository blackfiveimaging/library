/*
 * coloranttoggle.cpp
 * Provides a list of available colorants,
 * with a sample and a checkmark beside each one.
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <gtk/gtkentry.h>
#include <gtk/gtklist.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkscrolledwindow.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixdata.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "imagesource/imagesource_gdkpixbuf.h"
#include "imagesource/imagesource_greyscale.h"
#include "imagesource/imagesource_solid.h"
#include "imagesource/imagesource_mask.h"
#include "imagesource/imagesource_devicen_preview.h"
#include "imagesource/devicencolorant.h"
#include "imagesource/pixbuf_from_imagesource.h"

#include "miscwidgets/generaldialogs.h"
#include "coloranttoggle.h"

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

#include "squiggledata.cpp"

using namespace std;

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint coloranttoggle_signals[LAST_SIGNAL] = { 0 };

static void coloranttoggle_class_init (ColorantToggleClass *klass);
static void coloranttoggle_init (ColorantToggle *sel);


static bool style_applied=false;
static void apply_style()
{
	if(style_applied)
		return;
	gtk_rc_parse_string("style \"mystyle\"\n"
						"{\n"
						"	GtkWidget::focus-padding = 0\n"
						"	GtkWidget::focus-line-width = 0\n"
						"	xthickness = 0\n"
						"	ythickness = 0\n"
						"}\n"
						"widget \"*.colourtoggle-button\" style \"mystyle\"\n");
	style_applied=true;
}


void ToggleData::toggled(GtkWidget *wid,gpointer user_data)
{
	ToggleData *td=(ToggleData *)user_data;

	int state=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(td->button));
	if(state)
		td->col.Enable();
	else
		td->col.Disable();

	g_signal_emit(G_OBJECT(td->toggle),coloranttoggle_signals[CHANGED_SIGNAL], 0);
}


void ToggleData::refresh()
{
	cerr << "Refreshing" << endl;

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),col.GetEnabled());
}


GtkWidget*
coloranttoggle_new (DeviceNColorantList *list)
{
	ColorantToggle *c=COLORANTTOGGLE(g_object_new (coloranttoggle_get_type (), NULL));

	coloranttoggle_set_colorants(c,list);
	
	return(GTK_WIDGET(c));
}


GType
coloranttoggle_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo coloranttoggle_info =
		{
			sizeof (ColorantToggleClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) coloranttoggle_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (ColorantToggle),
			0,
			(GInstanceInitFunc) coloranttoggle_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "ColorantToggle", &coloranttoggle_info, GTypeFlags(0));
	}
	return stpuic_type;
}


static void *parent_class=NULL;

static void coloranttoggle_destroy(GtkObject *object)
{
	if(object && IS_COLORANTTOGGLE(object))
	{
		ColorantToggle *c=COLORANTTOGGLE(object);

		while(c->buttons.size())
		{
			delete c->buttons[0];
			c->buttons.pop_front();
		}

		if (GTK_OBJECT_CLASS (parent_class)->destroy)
			(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
	}
}


static void
coloranttoggle_class_init (ColorantToggleClass *cls)
{
	GtkObjectClass *object_class=(GtkObjectClass *)cls;

	parent_class = gtk_type_class (gtk_widget_get_type ());

	object_class->destroy = coloranttoggle_destroy;

	coloranttoggle_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (cls),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (ColorantToggleClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
coloranttoggle_init (ColorantToggle *c)
{
	// Because the class is malloced, and no constructor is called, we need to 
	// arrange for the deque to be constructed, via placement new.

	new (&c->buttons) std::deque<ToggleData *>;	// Yeah, I know - yuk
}


// Rebuilds the listview from the ColorantList
void coloranttoggle_refresh(ColorantToggle *c)
{
	if(c)
	{
		for(int i=0;i<c->buttons.size();++i)
		{
			c->buttons[i]->refresh();
		}
	}
}


void coloranttoggle_set_colorants(ColorantToggle *c,DeviceNColorantList *list)
{
	apply_style();

	while(c->buttons.size())
	{
		cerr << "Have " << c->buttons.size() << " entries in button list..." << endl;
		delete c->buttons[0];
		c->buttons.pop_front();
	}

	// Free toggle buttons here, and create a new set...
	if(c && list)
	{
		GdkPixbuf *icon=NULL;
		GdkPixdata pd;
		GError *err;
		if(!gdk_pixdata_deserialize(&pd,sizeof(my_pixbuf),my_pixbuf,&err))
			throw(err->message);

		if(!(icon=gdk_pixbuf_from_pixdata(&pd,false,&err)))
			throw(err->message);

		DeviceNColorant *col=list->FirstColorant();
		int i=0;
		while(col)
		{
			if(col->GetName())
			{
				// Colorize the icon here

				ImageSource *mask=new ImageSource_GdkPixbuf(icon);
				mask=new ImageSource_Greyscale(mask);

				DeviceNColorant *col=(*list)[i];

				ISDataType solid[3];
				solid[0]=EIGHTTOIS(col->red);
				solid[1]=EIGHTTOIS(col->green);
				solid[2]=EIGHTTOIS(col->blue);

				ImageSource *is=new ImageSource_Solid(IS_TYPE_RGB,mask->width,mask->height,solid);

				is=new ImageSource_Mask(is,mask);
				GdkPixbuf *colicon=pixbuf_alpha_from_imagesource(is);

				// We now have an pixbuf with alpha channel

				GtkWidget *img=gtk_image_new_from_pixbuf(colicon);
				GtkWidget *togglebutton=gtk_toggle_button_new();
				gtk_widget_set_name(togglebutton,"colourtoggle-button");
				gtk_button_set_image(GTK_BUTTON(togglebutton),img);
				gtk_button_set_relief(GTK_BUTTON(togglebutton),GTK_RELIEF_NONE);
				gtk_box_pack_start(GTK_BOX(c),togglebutton,FALSE,FALSE,0);
				gtk_widget_set_size_request(togglebutton,is->width+2,is->height+2);
				gtk_widget_show(togglebutton);
				gtk_widget_show(img);

				delete is;

				// Now create a list entry for the colorant...
				ToggleData *td=new ToggleData(c,togglebutton,*col);
				td->refresh();
				c->buttons.push_back(td);

				g_signal_connect(G_OBJECT(togglebutton),"toggled",G_CALLBACK(ToggleData::toggled),td);

			}

			++i;
			col=col->NextColorant();
	    }
		g_object_unref(icon);
	}
}

