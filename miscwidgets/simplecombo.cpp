/*
 * simplecombo.c - provides a custom widget for choosing
 * one of a static list of options, where the internal
 * and displayed names are different.
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

#include "simplecombo.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};


static guint simplecombo_signals[LAST_SIGNAL] = { 0 };

static void simplecombo_class_init (SimpleComboClass *klass);
static void simplecombo_init (SimpleCombo *stpuicombo);


static void simplecombo_build_options(SimpleCombo *c)
{
	if(c->menu)
		gtk_option_menu_remove_menu(GTK_OPTION_MENU(c->optionmenu));
	c->menu=gtk_menu_new();

	int i=0;
	while(c->opts[i].option)
	{
		const char *name=gettext(c->opts[i].displayname);
		if(name)
		{
			GtkWidget *menu_item = gtk_menu_item_new_with_label (name);
			gtk_menu_shell_append (GTK_MENU_SHELL (c->menu), menu_item);
			gtk_widget_show (menu_item);
		}
		++i;
	}
	gtk_option_menu_set_menu(GTK_OPTION_MENU(c->optionmenu),c->menu);
}


static void	simplecombo_entry_changed(GtkEntry *entry,gpointer user_data)
{
	SimpleCombo *c=SIMPLECOMBO(user_data);

	int index=gtk_option_menu_get_history(GTK_OPTION_MENU(c->optionmenu));
	if(index==c->previdx)
		return;

	c->previdx=index;
	
	g_signal_emit(G_OBJECT (c),simplecombo_signals[CHANGED_SIGNAL], 0);
}


GtkWidget *simplecombo_new (SimpleComboOption *opts)
{
	SimpleCombo *c=SIMPLECOMBO(g_object_new (simplecombo_get_type (), NULL));

	c->opts=opts;
	c->tips=gtk_tooltips_new();
	c->optionmenu=gtk_option_menu_new();
	c->menu=NULL;  // Built on demand...

	simplecombo_build_options(c);

	gtk_box_pack_start(GTK_BOX(c),GTK_WIDGET(c->optionmenu),TRUE,TRUE,0);
	gtk_widget_show(c->optionmenu);

	g_signal_connect(c->optionmenu,"changed",G_CALLBACK(simplecombo_entry_changed),c);
	
	return(GTK_WIDGET(c));
}


GType
simplecombo_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo simplecombo_info =
		{
			sizeof (SimpleComboClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) simplecombo_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (SimpleCombo),
			0,
			(GInstanceInitFunc) simplecombo_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "SimpleCombo", &simplecombo_info, GTypeFlags(0));
	}
	return stpuic_type;
}


static void
simplecombo_class_init (SimpleComboClass *klass)
{
	simplecombo_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (SimpleComboClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
simplecombo_init (SimpleCombo *c)
{
	c->optionmenu=NULL;
	c->menu=NULL;
	c->previdx=-1;
}


const char *simplecombo_get(SimpleCombo *c)
{
	gint index=gtk_option_menu_get_history(GTK_OPTION_MENU(c->optionmenu));
	int i=0;
	while(i<index)
	{
		if(!c->opts[i].option)
			return(NULL);
		++i;
	}
	return(c->opts[index].option);
}


bool simplecombo_set(SimpleCombo *c,const char *value)
{
	int i=0;
	if(value)
	{
		while(c->opts[i].option)
		{
			if(strcmp(value,c->opts[i].option)==0)
			{
				gtk_option_menu_set_history(GTK_OPTION_MENU(c->optionmenu),i);
				return(true);
			}	
			++i;
		}
	}
	return(false);
}

