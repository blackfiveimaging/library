/*
 * colorantselector.cpp
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GTK

#include <gtk/gtk.h>
#include <gtk/gtkentry.h>
#include <gtk/gtklist.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkscrolledwindow.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixdata.h>

#include "imagesource_gdkpixbuf.h"
#include "imagesource_greyscale.h"
#include "imagesource_devicen_preview.h"
#include "devicencolorant.h"
#include "pixbuf_from_imagesource.h"

#include "generaldialogs.h"
#include "pixbuf_from_imagedata.h"
#include "colorantselector.h"

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

#include "squiggledata.cpp"

using namespace std;

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint colorantselector_signals[LAST_SIGNAL] = { 0 };

static void colorantselector_class_init (ColorantSelectorClass *klass);
static void colorantselector_init (ColorantSelector *sel);


static void selection_changed(GtkTreeSelection *select,gpointer user_data)
{
//	ColorantSelector *pe=COLORANTSELECTOR(user_data);
}



// Declare multiple columns for GTK's tree model.  The columnns can contain widgets,
// not just static display, and don't have to be visible, so we can include userdata.
enum
{
  ACTIVE_COLUMN = 0,	// Checkmark
  ICON_COLUMN,			// Pixbuf
  LABEL_COLUMN,			// Colorant name (string)
						// Now non-displayed fields:
  COLORANT_COLUMN,		// gpointer to this colorant.
  NUM_COLUMNS
};


static void populate_list(ColorantSelector *es)
{
	if(es->list)
	{
		GtkTreeIter iter;

		GdkPixbuf *icon=PixbufFromImageData(squiggle_data,sizeof(squiggle_data));

		DeviceNColorant *col=es->list->FirstColorant();
		int i=0;
		while(col)
		{
			if(col->GetName())
			{
				// Colorize the icon here
				ImageSource *is=new ImageSource_GdkPixbuf(icon);
				is=new ImageSource_Greyscale(is);
				is=new ImageSource_DeviceN_Preview(is,es->list,i);
				GdkPixbuf *colicon=pixbuf_from_imagesource(is);

				gtk_tree_store_append (GTK_TREE_STORE(es->treestore), &iter, NULL);
				gtk_tree_store_set (GTK_TREE_STORE(es->treestore), &iter,
					ACTIVE_COLUMN, col->GetEnabled(),
					LABEL_COLUMN, strdup(col->GetDisplayName()),
					ICON_COLUMN, colicon,
					COLORANT_COLUMN,col,
					-1);
			}
			++i;
			col=col->NextColorant();
	    }
		g_object_unref(icon);
	}
}


static void
item_toggled (GtkCellRendererToggle *cell,gchar *path_str,gpointer data)
{
	ColorantSelector *sel=COLORANTSELECTOR(data);
	GtkTreeModel *model = GTK_TREE_MODEL(sel->treestore);
	GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
	GtkTreeIter iter;
	gboolean toggle_item;

	// It seems the checkbox state has to be toggled manually by the callback...

	gint *column = (gint *)g_object_get_data (G_OBJECT (cell), "column");

	/* get toggled iter */
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, column, &toggle_item, -1);

	/* do something with the value */
	toggle_item ^= 1;

	/* set new value */
	gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,
		toggle_item, -1);

	gpointer *ptr=NULL;
	gtk_tree_model_get (model, &iter, COLORANT_COLUMN,&ptr, -1);
	if(ptr)
	{
		DeviceNColorant *col=(DeviceNColorant *)ptr;
		if(toggle_item)
			col->Enable();
		else
			col->Disable();
	}	

	// Emit the Changed signal
	g_signal_emit(G_OBJECT (sel),colorantselector_signals[CHANGED_SIGNAL], 0);

	// Clean up
	gtk_tree_path_free (path);
}


static void setup_renderers(ColorantSelector *sel)
{
	gint col_offset;
	GtkCellRenderer *renderer;

	// Active column
	renderer = gtk_cell_renderer_toggle_new ();

	g_signal_connect (renderer, "toggled", G_CALLBACK (item_toggled), sel);

	col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (sel->treeview),
		-1, _("Active"),
		renderer,
		"active",
		ACTIVE_COLUMN,
		NULL);

	// Pixbuf column
	renderer=gtk_cell_renderer_pixbuf_new();
	col_offset=gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (sel->treeview),
		-1,_("Icon"),
		renderer,
		"pixbuf",
		ICON_COLUMN,
		NULL);

	// column for names
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "xalign", 0.0, NULL);
	g_object_set (renderer, "xpad", 3, NULL);
	
	col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (sel->treeview),
	    -1, _("Name"),
	    renderer, "text",
	    LABEL_COLUMN,
	    NULL);
}


GtkWidget*
colorantselector_new (DeviceNColorantList *list)
{
	ColorantSelector *c=COLORANTSELECTOR(g_object_new (colorantselector_get_type (), NULL));

	c->list=list;

	c->treestore = gtk_tree_store_new (NUM_COLUMNS,
		G_TYPE_BOOLEAN,
		GDK_TYPE_PIXBUF,
		G_TYPE_STRING,
		G_TYPE_POINTER
		);

	GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (c), sw, TRUE, TRUE, 0);
	gtk_widget_show(sw);

	c->treeview=gtk_tree_view_new_with_model(GTK_TREE_MODEL(c->treestore));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(c->treeview),false);
	setup_renderers(c);

	GtkTreeSelection *select;
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (c->treeview));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (selection_changed),c);

	gtk_container_add(GTK_CONTAINER(sw),c->treeview);
	gtk_widget_show(c->treeview);

	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(c),hbox,FALSE,FALSE,0);
	gtk_widget_show(hbox);

	populate_list(c);
	
	return(GTK_WIDGET(c));
}


GType
colorantselector_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo colorantselector_info =
		{
			sizeof (ColorantSelectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) colorantselector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (ColorantSelector),
			0,
			(GInstanceInitFunc) colorantselector_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "ColorantSelector", &colorantselector_info, GTypeFlags(0));
	}
	return stpuic_type;
}


static void *parent_class=NULL;

static void colorantselector_destroy(GtkObject *object)
{
	if(object && IS_COLORANTSELECTOR(object))
	{
//		ColorantSelector *es=COLORANTSELECTOR(object);

		// FIXME - should free the colorant names here...

		if (GTK_OBJECT_CLASS (parent_class)->destroy)
			(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
	}
}


static void
colorantselector_class_init (ColorantSelectorClass *cls)
{
	GtkObjectClass *object_class=(GtkObjectClass *)cls;

	parent_class = gtk_type_class (gtk_widget_get_type ());

	object_class->destroy = colorantselector_destroy;

	colorantselector_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (cls),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (ColorantSelectorClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
colorantselector_init (ColorantSelector *c)
{
}


// Rebuilds the listview from the ColorantList
void colorantselector_refresh(ColorantSelector *c)
{
	if(c)
	{
		// FIXME - should go through the list first, freeing
		// pixbufs and names?
		gtk_tree_store_clear(c->treestore);
		populate_list(c);
	}
}


void colorantselector_set_colorants(ColorantSelector *c,DeviceNColorantList *list)
{
	gtk_tree_store_clear(c->treestore);
	c->list=list;
	populate_list(c);
}

#endif // HAVE_GTK
