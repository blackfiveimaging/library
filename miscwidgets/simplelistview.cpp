/*
 * simplelistview.c - provides a custom widget for choosing
 * one of a static list of options, where the internal
 * and displayed names are different.
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <cstdlib>
#include <cstring>

#include <gtk/gtkentry.h>
#include <gtk/gtklist.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkcellrendererpixbuf.h>
#include <gtk/gtkcellrenderertext.h>

#include "simplelistview.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

enum {
	CHANGED_SIGNAL,
	DOUBLECLICKED_SIGNAL,
	LAST_SIGNAL
};


// Declare multiple columns for GTK's tree model.  The columnns can contain widgets,
// not just static display, and don't have to be visible, so we can include userdata.
enum
{
	ICON_COLUMN=0,		// Pixbuf
	LABEL_COLUMN,		// Entry name (string)
						// Now non-displayed fields:
	INDEX_COLUMN,		// Index which can be fetched from signal handlers
	POINTER_COLUMN,		// Pointer to Option
	NUM_COLUMNS
};



static guint simplelistview_signals[LAST_SIGNAL] = { 0 };

static void simplelistview_class_init (SimpleListViewClass *klass);
static void simplelistview_init (SimpleListView *stpuilistview);
static void simplelistview_destroy(GtkObject *object);


static void	simplelistview_entry_changed(GtkEntry *entry,gpointer user_data)
{
	SimpleListView *c=SIMPLELISTVIEW(user_data);

	int index=simplelistview_get_index(c);

	SimpleListViewOption *o=(*c->opts)[index];

	if(o && o->tooltip && (strlen(o->tooltip)>0))
	{
//		gtk_tooltips_set_tip(c->tips,c->optionmenu,o->tooltip,o->tooltip);
		gtk_tooltips_enable(c->tips);
	}
	else
	{
//		gtk_tooltips_set_tip(c->tips,c->optionmenu,"","");
		gtk_tooltips_disable(c->tips);
	}

	g_signal_emit(G_OBJECT (c),simplelistview_signals[CHANGED_SIGNAL], 0);
}


void simplelistview_set_opts(SimpleListView *c,SimpleListViewOptions *opts)
{
	gtk_list_store_clear(GTK_LIST_STORE(c->liststore));

	if(c->opts)
		delete c->opts;
	if(opts)
		c->opts=new SimpleListViewOptions(*opts);
	else
		c->opts=new SimpleListViewOptions();

	GtkTreeIter iter;
	for(unsigned int idx=0;idx<c->opts->size();++idx)
	{
		SimpleListViewOption *o=(*c->opts)[idx];

		gtk_list_store_append (GTK_LIST_STORE(c->liststore), &iter);
		gtk_list_store_set (GTK_LIST_STORE(c->liststore), &iter,
			ICON_COLUMN, o->icon,
			LABEL_COLUMN, o->displayname,
			INDEX_COLUMN, idx,
			POINTER_COLUMN,o,
			-1);
	}
}


static void row_activated(GtkTreeView *tree_view,GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
	SimpleListView *c=SIMPLELISTVIEW(user_data);
	cerr << "Got row-activated signal" << endl;
	g_signal_emit(G_OBJECT (c),simplelistview_signals[DOUBLECLICKED_SIGNAL], 0);
}


static void selection_changed(GtkTreeSelection *select,gpointer user_data)
{
	cerr << "Got selection changed signal" << endl;
	SimpleListView *c=SIMPLELISTVIEW(user_data);
	g_signal_emit(G_OBJECT(c),simplelistview_signals[CHANGED_SIGNAL], 0);
};


GtkWidget *simplelistview_new (SimpleListViewOptions *opts)
{
	SimpleListView *c=SIMPLELISTVIEW(g_object_new (simplelistview_get_type (), NULL));

	c->tips=gtk_tooltips_new();
	c->liststore=gtk_list_store_new(NUM_COLUMNS,GDK_TYPE_PIXBUF,G_TYPE_STRING,G_TYPE_INT,G_TYPE_POINTER);

	GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (c), sw, TRUE, TRUE, 0);
	gtk_widget_show(sw);

	c->treeview=gtk_tree_view_new_with_model(GTK_TREE_MODEL(c->liststore));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(c->treeview),FALSE);
	gtk_container_add(GTK_CONTAINER(sw),c->treeview);
	gtk_widget_show(c->treeview);

	GtkCellRenderer *renderer;
	renderer=gtk_cell_renderer_pixbuf_new();
	g_object_set(G_OBJECT(renderer),"xpad",4,NULL);
	g_object_set(G_OBJECT(renderer),"ypad",4,NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(c->treeview),-1,_("Image"),renderer,"pixbuf",ICON_COLUMN,NULL);

	// column for names
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "xalign", 0.0, NULL);
	g_object_set (renderer, "xpad", 10, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(c->treeview),-1, _("Name"),renderer, "text",LABEL_COLUMN,NULL);

	g_signal_connect (G_OBJECT (c->treeview), "row-activated",
		G_CALLBACK (row_activated),c);

	simplelistview_set_opts(c,opts);


	GtkTreeSelection *select;
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (c->treeview));

	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (selection_changed),c);

	return(GTK_WIDGET(c));
}


GType
simplelistview_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo simplelistview_info =
		{
			sizeof (SimpleListViewClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) simplelistview_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (SimpleListView),
			0,
			(GInstanceInitFunc) simplelistview_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "SimpleListView", &simplelistview_info, GTypeFlags(0));
	}
	return stpuic_type;
}


static void
simplelistview_class_init (SimpleListViewClass *cl)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	
	object_class = (GtkObjectClass*) cl;
	widget_class = (GtkWidgetClass*) cl;
	
	object_class->destroy = simplelistview_destroy;	

	simplelistview_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (cl),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (SimpleListViewClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	simplelistview_signals[DOUBLECLICKED_SIGNAL] =
	g_signal_new ("double-clicked",
		G_TYPE_FROM_CLASS (cl),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (SimpleListViewClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void *parent_class=NULL;

static void simplelistview_destroy(GtkObject *object)
{
	if(object && IS_SIMPLELISTVIEW(object))
	{
		if (GTK_OBJECT_CLASS (parent_class)->destroy)
			(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
	// FIXME - cleanup?  Memory leak here?
		SimpleListView *c=SIMPLELISTVIEW(object);
		if(c->opts)
			delete c->opts;
		c->opts=NULL;
	}
}


static void
simplelistview_init (SimpleListView *c)
{
	parent_class = gtk_type_class (gtk_widget_get_type ());
}


SimpleListViewOption *simplelistview_get(SimpleListView *c)
{
	SimpleListViewOption *result=0;
	cerr << "Getting selection" << endl;
	GtkTreeSelection *select = gtk_tree_view_get_selection (GTK_TREE_VIEW (c->treeview));

	GtkTreeIter iter;
	GtkTreeModel *model;
	if(gtk_tree_selection_get_selected (select, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, POINTER_COLUMN, &result, -1);
	}
	return(result);
}


int simplelistview_get_index(SimpleListView *c)
{
	int result=0;
	cerr << "Getting selection" << endl;
	GtkTreeSelection *select = gtk_tree_view_get_selection (GTK_TREE_VIEW (c->treeview));

	GtkTreeIter iter;
	GtkTreeModel *model;
	if(gtk_tree_selection_get_selected (select, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, INDEX_COLUMN, &result, -1);
	}
	return(result);
}


#if 0
bool simplelistview_set(SimpleListView *c,const char *key)
{
	int i=0;
	if(key)
	{
		for(unsigned int idx=0;idx<c->opts->size();++idx)
		{
			SimpleListViewOption *o=(*c->opts)[idx];
			if(strcmp(key,o->key)==0)
			{
//				gtk_option_menu_set_history(GTK_OPTION_MENU(c->optionmenu),i);
				// FIXME - set selection option here...
				return(true);
			}	
			++i;
		}
	}
	return(false);
}
#endif


void simplelistview_set_index(SimpleListView *c,int idx)
{
	// Use -1 to indicate no selection
	if(idx<0)
	{
		GtkTreeSelection *s=gtk_tree_view_get_selection(GTK_TREE_VIEW(c->treeview));
		gtk_tree_selection_unselect_all(s);
	}
	else
	{
		// FIXME - only supports up to 100 list entries!
		char path[]={'0','0'};
		if(idx>9)
		{
			path[0]+=idx/10;
			path[1]+=idx%10;
		}
		else
		{
			path[0]+=idx%10;
			path[1]=0;
		}
		GtkTreePath *tmppath=gtk_tree_path_new_from_string(path);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(c->treeview),tmppath,NULL,FALSE);
		gtk_tree_view_row_activated(GTK_TREE_VIEW(c->treeview),tmppath,NULL);
		gtk_tree_path_free(tmppath);
	}
}


//  Class definitions for SimpleListViewOpt(s)


SimpleListViewOption::SimpleListViewOption(const char *key,const char *displayname,const char *tooltip,
	GdkPixbuf *icon,void *userdata)
	:	key(NULL),displayname(NULL),tooltip(NULL),icon(icon),userdata(userdata)
{
	if(key)
		this->key=strdup(key);
	if(displayname)
		this->displayname=strdup(displayname);
	if(tooltip)
		this->tooltip=strdup(tooltip);
	if(icon)
		g_object_ref(icon);
}


SimpleListViewOption::SimpleListViewOption(SimpleListViewOption &other)
	: key(NULL),displayname(NULL),tooltip(NULL),icon(other.icon),userdata(other.userdata)
{
	if(other.key)
		key=strdup(other.key);
	if(other.displayname)
		displayname=strdup(other.displayname);
	if(other.tooltip)
		tooltip=strdup(other.tooltip);
	if(icon)
		g_object_ref(icon);
}


SimpleListViewOption::~SimpleListViewOption()
{
	if(tooltip)
		free(tooltip);
	if(displayname)
		free(displayname);
	if(key)
		free(key);
	if(icon)
		g_object_unref(icon);
}


SimpleListViewOptions::SimpleListViewOptions()
{
}

SimpleListViewOptions::SimpleListViewOptions(SimpleListViewOptions &other) : std::deque<SimpleListViewOption *>()
{
	for(unsigned int i=0;i<other.size();++i)
		push_back(new SimpleListViewOption(*other[i]));
}


SimpleListViewOptions::~SimpleListViewOptions()
{
	Clear();
}


SimpleListViewOption *SimpleListViewOptions::Add(const char *key,const char *displayname,const char *tooltip,GdkPixbuf *icon,void *userdata)
{
	SimpleListViewOption *opt;
	push_back(opt=new SimpleListViewOption(key,displayname,tooltip,icon,userdata));
	return(opt);
}


void SimpleListViewOptions::Clear()
{
	while(size())
	{
		delete (*this)[0];
		pop_front();
	}
}


