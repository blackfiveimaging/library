/*
 * viewingcondselector.c - provides a custom widget for choosing
 * a viewing condition for Argyll utilities.
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include <string.h>

#include <gtk/gtkentry.h>
#include <gtk/gtklist.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtktooltips.h>

#include "viewingcondselector.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

struct argyllviewingcond
{
	const char *value;
	const char *description;
};

struct argyllviewingcond viewingconds[]=
{
	{"",N_("Default")},
	{"pp",N_("Practical Reflection Print")},
	{"pe",N_("Print evaluation environment")},
	{"mt",N_("Monitor in typical work environment")},
	{"mb",N_("Monitor in bright work environment")},
	{"md",N_("Monitor in darkened work environment")},
	{"jm",N_("Projector in dim environment")},
	{"jd",N_("Projector in dark environment")},
	{"pcd",N_("Photo CD - original scene outdoors")},
	{"ob",N_("Original scene - Bright Outdoors")},
	{"cx",N_("Cut Sheet Transparencies on a viewing box")}
};

#define VIEWINGCONDS_COUNT sizeof(viewingconds)/sizeof(struct argyllviewingcond)

static guint viewingcondselector_signals[LAST_SIGNAL] = { 0 };

static void viewingcondselector_class_init (ViewingCondSelectorClass *klass);
static void viewingcondselector_init (ViewingCondSelector *stpuicombo);


static void viewingcondselector_build_options(ViewingCondSelector *c)
{
	if(c->menu)
		gtk_option_menu_remove_menu(GTK_OPTION_MENU(c->optionmenu));
	c->menu=gtk_menu_new();

	for(unsigned int i=0;i<VIEWINGCONDS_COUNT;++i)
	{
		const char *name=gettext(viewingconds[i].description);
		if(name)
		{
			GtkWidget *menu_item = gtk_menu_item_new_with_label (name);
			gtk_menu_shell_append (GTK_MENU_SHELL (c->menu), menu_item);
			gtk_widget_show (menu_item);
		}
	}
	gtk_option_menu_set_menu(GTK_OPTION_MENU(c->optionmenu),c->menu);
}


static void	viewingcondselector_entry_changed(GtkEntry *entry,gpointer user_data)
{
	ViewingCondSelector *c=VIEWINGCONDSELECTOR(user_data);

	int index=gtk_option_menu_get_history(GTK_OPTION_MENU(c->optionmenu));
	if(index==c->previdx)
		return;

	c->previdx=index;
	
	g_signal_emit(G_OBJECT (c),viewingcondselector_signals[CHANGED_SIGNAL], 0);
}


GtkWidget*
viewingcondselector_new ()
{
	ViewingCondSelector *c=VIEWINGCONDSELECTOR(g_object_new (viewingcondselector_get_type (), NULL));

	c->tips=gtk_tooltips_new();
	c->optionmenu=gtk_option_menu_new();
	c->menu=NULL;  // Built on demand...

	viewingcondselector_build_options(c);

	gtk_box_pack_start(GTK_BOX(c),GTK_WIDGET(c->optionmenu),TRUE,TRUE,0);
	gtk_widget_show(c->optionmenu);

	g_signal_connect(c->optionmenu,"changed",G_CALLBACK(viewingcondselector_entry_changed),c);
	
	return(GTK_WIDGET(c));
}


GType
viewingcondselector_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo viewingcondselector_info =
		{
			sizeof (ViewingCondSelectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) viewingcondselector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (ViewingCondSelector),
			0,
			(GInstanceInitFunc) viewingcondselector_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "ViewingCondSelector", &viewingcondselector_info, GTypeFlags(0));
	}
	return stpuic_type;
}


static void
viewingcondselector_class_init (ViewingCondSelectorClass *klass)
{
	viewingcondselector_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (ViewingCondSelectorClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
viewingcondselector_init (ViewingCondSelector *c)
{
	c->optionmenu=NULL;
	c->menu=NULL;
	c->previdx=-1;
}


const char *viewingcondselector_get(ViewingCondSelector *c)
{
	gint index=gtk_option_menu_get_history(GTK_OPTION_MENU(c->optionmenu));
	return(viewingconds[index].value);
}


bool viewingcondselector_set(ViewingCondSelector *c,const char *value)
{
	for(unsigned int i=0;i<VIEWINGCONDS_COUNT;++i)
	{
		if(strcmp(value,viewingconds[i].value)==0)
		{
			gtk_option_menu_set_history(GTK_OPTION_MENU(c->optionmenu),i);
			return(true);
		}		
	}
	return(false);
}
