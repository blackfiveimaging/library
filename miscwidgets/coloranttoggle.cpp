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

#include <cmath>
#include <cstring>
#include <cstdlib>

#include <gtk/gtk.h>
#include <gtk/gtkentry.h>
#include <gtk/gtklist.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkscrolledwindow.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixdata.h>

#include <cairo.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "imagesource_gdkpixbuf.h"
#include "imagesource_greyscale.h"
#include "imagesource_solid.h"
#include "imagesource_mask.h"
#include "imagesource_devicen_preview.h"
#include "devicencolorant.h"
#include "pixbuf_from_imagesource.h"

#include "generaldialogs.h"
#include "pixbuf_from_imagedata.h"
#include "coloranttoggle.h"

#include "debug.h"

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

#include "dabdata.cpp"

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


void ToggleData::paint(GtkWidget *widget,GdkEventExpose *eev,gpointer userdata)
{
	ToggleData *td=(ToggleData *)userdata;

	int width  = widget->allocation.width;
	int height = widget->allocation.height;

	cairo_t *cr = gdk_cairo_create (widget->window);
	if(!cr)
		return;

	// Yuk.

	gdk_draw_rectangle (widget->window,
		widget->style->bg_gc[widget->state],TRUE,
		0,0,width,height);


	// draw ellipse

	cairo_new_path(cr);
	cairo_set_source_rgb(cr,td->col.red/255.0,td->col.green/255.0,td->col.blue/255.0);

	cairo_save (cr);

	cairo_translate (cr, width/2, height/2);
	cairo_rotate(cr,(3*M_PI)/4);
	cairo_scale (cr, width / 2., height / 2.5);
	cairo_arc (cr, 0., 0., 1., 0., 2 * M_PI);
	cairo_close_path(cr);
	cairo_fill(cr);

	cairo_restore (cr);


	if(td->level>=0)
	{
		char buf[10];
		int l=(100 * td->level) / IS_SAMPLEMAX;
		sprintf(buf,"%d",l<100 ? l : 100);

		// We try to figure out whether black or white will contrast better with the blob...
		// Coefficients from http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

		double total=0.2126729*td->col.red + 0.7151522 * td->col.green + 0.0721750*td->col.blue;

		if(total<128)
			cairo_set_source_rgb(cr,1.0,1.0,1.0);
		else
			cairo_set_source_rgb(cr,0.0,0.0,0.0);


	    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                                        CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, 12);

		cairo_text_extents_t extents;
		cairo_text_extents(cr,buf,&extents);

		// Is the graphic too narrow to hold the text?
		if((width-2)<extents.width)
		{
			cairo_scale(cr,double(width-2)/extents.width,1.0);
			width=extents.width+2;
		}

		int xpos=(width-extents.width)/2 - extents.x_bearing;
		int ypos=(height-extents.height)/2 - extents.y_bearing;

		cairo_move_to (cr, xpos, ypos);
		cairo_show_text (cr, buf);
	}

	cairo_destroy (cr);
}


void ToggleData::redraw(int value)
{
	level=value;
	paint(canvas,NULL,this);
}


void ToggleData::refresh()
{
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

	// Free toggle buttons here, and create a new set...
	while(c->buttons.size())
	{
		cerr << "Have " << c->buttons.size() << " entries in button list..." << endl;
		delete c->buttons[0];
		c->buttons.pop_front();
	}

	if(c && list)
	{
//		GdkPixbuf *icon=PixbufFromImageData(dab_data,sizeof(dab_data));

		DeviceNColorant *col=list->FirstColorant();
		int i=0;
		while(col)
		{

			if(col->GetName())
			{
				// Colorize the icon here

#if 0
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
#endif
				// We now have a pixbuf with alpha channel

#if 0
				GtkWidget *img=gtk_image_new_from_pixbuf(colicon);
				GtkWidget *togglebutton=gtk_toggle_button_new();
				gtk_button_set_image(GTK_BUTTON(togglebutton),img);
				gtk_box_pack_start(GTK_BOX(c),togglebutton,FALSE,FALSE,0);
				gtk_widget_show(togglebutton);
				gtk_widget_show(img);
#endif

//				GtkWidget *img=gtk_image_new_from_pixbuf(colicon);
//				GtkWidget *togglebutton=gtk_check_button_new();
//				gtk_button_set_image(GTK_BUTTON(togglebutton),img);
//				gtk_box_pack_start(GTK_BOX(c),togglebutton,FALSE,FALSE,0);
//				gtk_box_pack_start(GTK_BOX(c),img,FALSE,FALSE,0);
//				gtk_widget_show(togglebutton);
//				gtk_widget_show(img);

//				delete is;

				// Now create a list entry for the colorant...
				ToggleData *td=new ToggleData(c,*col);
				td->refresh();
				c->buttons.push_back(td);

//				g_signal_connect(G_OBJECT(togglebutton),"toggled",G_CALLBACK(ToggleData::toggled),td);

			}

			++i;
			col=col->NextColorant();
	    }
//		g_object_unref(icon);
	}
}


void coloranttoggle_set_value(ColorantToggle *c,ISDeviceNValue &value)
{
	for(int i=0;i<c->buttons.size();++i)
	{
		c->buttons[i]->redraw(value[i]);
	}
}


