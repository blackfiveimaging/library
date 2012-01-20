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
#include <cstdlib>
#include <cstring>

#include "config.h"

#if HAVE_GTK
#include <gtk/gtkentry.h>
#include <gtk/gtklist.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtktooltips.h>

#include "debug.h"

#include "simplecombo.h"

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
static void simplecombo_destroy(GtkObject *object);


static void	simplecombo_entry_changed(GtkEntry *entry,gpointer user_data)
{
	SimpleCombo *c=SIMPLECOMBO(user_data);

	int index=gtk_option_menu_get_history(GTK_OPTION_MENU(c->optionmenu));
	SimpleComboOption *o=(*c->opts)[index];

	if(index==c->previdx)
	{
		if(!(o && o->repeat))
			return;
	}

	c->previdx=index;

	if(o && o->tooltip && (strlen(o->tooltip)>0))
	{
		gtk_tooltips_set_tip(c->tips,c->optionmenu,o->tooltip,o->tooltip);
		gtk_tooltips_enable(c->tips);
	}
	else
	{
//		gtk_tooltips_set_tip(c->tips,c->optionmenu,"","");
		gtk_tooltips_disable(c->tips);
	}

	g_signal_emit(G_OBJECT (c),simplecombo_signals[CHANGED_SIGNAL], 0);
}


void simplecombo_set_opts(SimpleCombo *c,SimpleComboOptions &opts)
{
	string oldkey;
	int oldidx=-1;
	if(c->optionmenu && c->opts)
	{
		oldidx=simplecombo_get_index(c);
		if((*c->opts)[oldidx]->key)
			oldkey=(*c->opts)[oldidx]->key;
		gtk_widget_destroy(c->optionmenu);
	}
	Debug[TRACE] << "Simplecombo - Old index: " << oldidx << ", old key: " << oldkey << std::endl;
	c->optionmenu=c->menu=NULL;
	if(c->opts)
		delete c->opts;

	c->opts=new SimpleComboOptions(opts);

	c->optionmenu=gtk_option_menu_new();
	c->menu=gtk_menu_new();

	SimpleComboOption *o=c->opts->FirstOption();
	while(o)
	{
		GtkWidget *menu_item = gtk_menu_item_new_with_label (o->displayname);
		gtk_menu_shell_append (GTK_MENU_SHELL (c->menu), menu_item);
		gtk_widget_show (menu_item);
		o=o->NextOption();
	}
	gtk_option_menu_set_menu(GTK_OPTION_MENU(c->optionmenu),c->menu);

	gtk_box_pack_start(GTK_BOX(c),GTK_WIDGET(c->optionmenu),TRUE,TRUE,0);
	gtk_widget_show(c->optionmenu);

	try
	{
		if(oldkey.size())
			simplecombo_set(c,oldkey.c_str());
		else if(oldidx>=0)
			simplecombo_set_index(c,oldidx);
	}
	catch(const char *err)
	{
		Debug[WARN] << "Tried to set combo index out of range..." << std::endl;
	}

	g_signal_connect(c->optionmenu,"changed",G_CALLBACK(simplecombo_entry_changed),c);
}


GtkWidget *simplecombo_new (SimpleComboOptions &opts)
{
	SimpleCombo *c=SIMPLECOMBO(g_object_new (simplecombo_get_type (), NULL));

	c->tips=gtk_tooltips_new();

	simplecombo_set_opts(c,opts);

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
simplecombo_class_init (SimpleComboClass *cl)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	
	object_class = (GtkObjectClass*) cl;
	widget_class = (GtkWidgetClass*) cl;
	
	object_class->destroy = simplecombo_destroy;	

	simplecombo_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (cl),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (SimpleComboClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void *parent_class=NULL;

static void simplecombo_destroy(GtkObject *object)
{
	if(object && IS_SIMPLECOMBO(object))
	{
		if (GTK_OBJECT_CLASS (parent_class)->destroy)
			(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
	// FIXME - cleanup?  Memory leak here?
		SimpleCombo *c=SIMPLECOMBO(object);
		if(c->opts)
			delete c->opts;
		c->opts=NULL;
	}
}


static void
simplecombo_init (SimpleCombo *c)
{
	parent_class = gtk_type_class (gtk_widget_get_type ());
	c->optionmenu=NULL;
	c->menu=NULL;
	c->previdx=-1;
}


const char *simplecombo_get(SimpleCombo *c)
{
	gint index=gtk_option_menu_get_history(GTK_OPTION_MENU(c->optionmenu));
	SimpleComboOption *o=(*c->opts)[index];
	if(o)
		return(o->key);
	else
		return(NULL);
}


int simplecombo_get_index(SimpleCombo *c)
{
	return(gtk_option_menu_get_history(GTK_OPTION_MENU(c->optionmenu)));
}


bool simplecombo_set(SimpleCombo *c,const char *key)
{
	int i=0;
	if(key)
	{
		SimpleComboOption *o=c->opts->FirstOption();
		while(o)
		{
			if(o->key && strcmp(key,o->key)==0)
			{
				gtk_option_menu_set_history(GTK_OPTION_MENU(c->optionmenu),i);
				return(true);
			}	
			++i;
			o=o->NextOption();
		}
	}
	return(false);
}


void simplecombo_set_index(SimpleCombo *c,int index)
{
	gtk_option_menu_set_history(GTK_OPTION_MENU(c->optionmenu),index);
}


//  Class definitions for SimpleComboOpt(s)


SimpleComboOption::SimpleComboOption(SimpleComboOptions &header,const char *key,const char *displayname,const char *tooltip, bool repeat)
	:	key(NULL),displayname(NULL),tooltip(NULL),repeat(repeat),header(header),prevopt(NULL),nextopt(NULL)
{
	if((prevopt=header.firstopt))
	{
		while(prevopt->nextopt)
			prevopt=prevopt->nextopt;
		prevopt->nextopt=this;
	}
	else
		header.firstopt=this;

	if(key)
		this->key=strdup(key);
	if(displayname)
		this->displayname=strdup(displayname);
	if(tooltip)
		this->tooltip=strdup(tooltip);
}


SimpleComboOption::SimpleComboOption(SimpleComboOptions &header,SimpleComboOption &other)
	: key(NULL),displayname(NULL),tooltip(NULL),repeat(other.repeat),header(header),prevopt(NULL),nextopt(NULL)
{
	if((prevopt=header.firstopt))
	{
		while(prevopt->nextopt)
			prevopt=prevopt->nextopt;
		prevopt->nextopt=this;
	}
	else
		header.firstopt=this;

	if(other.key)
		key=strdup(other.key);
	if(other.displayname)
		displayname=strdup(other.displayname);
	if(other.tooltip)
		tooltip=strdup(other.tooltip);
}


SimpleComboOption::~SimpleComboOption()
{
	if(prevopt)
		prevopt->nextopt=nextopt;
	else
		header.firstopt=nextopt;
	if(nextopt)
		nextopt->prevopt=prevopt;

	if(tooltip)
		free(tooltip);
	if(displayname)
		free(displayname);
	if(key)
		free(key);
}


SimpleComboOption *SimpleComboOption::NextOption()
{
	return(nextopt);
}


SimpleComboOption *SimpleComboOption::PrevOption()
{
	return(prevopt);
}




SimpleComboOptions::SimpleComboOptions() : firstopt(NULL)
{
}

SimpleComboOptions::SimpleComboOptions(SimpleComboOptions &other) : firstopt(NULL)
{
	SimpleComboOption *opt=other.FirstOption();
	while(opt)
	{
		new SimpleComboOption(*this,*opt);
		opt=opt->NextOption();
	}
}

SimpleComboOptions::~SimpleComboOptions()
{
	Clear();
}

SimpleComboOption *SimpleComboOptions::Add(const char *key,const char *displayname,const char *tooltip,bool repeat)
{
	return(new SimpleComboOption(*this,key,displayname,tooltip,repeat));
}


void SimpleComboOptions::Clear()
{
	while(firstopt)
		delete firstopt;
}


SimpleComboOption *SimpleComboOptions::FirstOption()
{
	return(firstopt);
}

SimpleComboOption *SimpleComboOptions::operator[](int idx)
{
	SimpleComboOption *opt=FirstOption();
	while(opt)
	{
		if(idx==0)
			return(opt);
		opt=opt->NextOption();
		--idx;
	}
	if(!opt)
		throw "SimpleComboOptions::[] Index out of range";
	return(opt);
}
#endif // HAVE_GTK

