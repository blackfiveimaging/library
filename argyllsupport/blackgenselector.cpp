/*
 * blackgenselector.c - provides a custom widget for choosing
 * black generation curve for Argyll utilities.
 *
 * Copyright (c) 2009 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include <string.h>

#include <gtk/gtk.h>
#include <cairo.h>

#include "miscwidgets/simplecombo.h"

#include "blackgenselector.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};


static guint blackgenselector_signals[LAST_SIGNAL] = { 0 };

static void blackgenselector_class_init (BlackGenSelectorClass *klass);
static void blackgenselector_init (BlackGenSelector *stpuicombo);


static void blackgenselector_locus_changed(GtkEntry *entry,gpointer user_data)
{
	BlackGenSelector *c=BLACKGENSELECTOR(user_data);
	c->curve.SetLocusMode(simplecombo_get_index(SIMPLECOMBO(c->locus))==1 ? true : false);
}


static void blackgenselector_mode_changed(GtkEntry *entry,gpointer user_data)
{
	BlackGenSelector *c=BLACKGENSELECTOR(user_data);

	if(c->blockupdate)
		return;
	
	int idx=simplecombo_get_index(SIMPLECOMBO(c->combo));
	c->curve.SetMode(Argyll_BlackGeneration(idx));

	bool sens=(idx==ARGYLLBG_CUSTOM);

	gtk_widget_set_sensitive(c->enle,sens);
	gtk_widget_set_sensitive(c->enpo,sens);
	gtk_widget_set_sensitive(c->stle,sens);
	gtk_widget_set_sensitive(c->stpo,sens);
	gtk_widget_set_sensitive(c->shape,sens);

	blackgenselector_refresh(c);

	g_signal_emit(G_OBJECT (c),blackgenselector_signals[CHANGED_SIGNAL], 0);
}


static void blackgenselector_params_changed(GtkEntry *entry,gpointer user_data)
{
	BlackGenSelector *c=BLACKGENSELECTOR(user_data);
	if(c->blockupdate)
		return;

	c->curve.SetStartLevel(gtk_spin_button_get_value(GTK_SPIN_BUTTON(c->stle)));
	c->curve.SetEndLevel(gtk_spin_button_get_value(GTK_SPIN_BUTTON(c->enle)));
	c->curve.SetStartPos(gtk_spin_button_get_value(GTK_SPIN_BUTTON(c->stpo)));
	c->curve.SetEndPos(gtk_spin_button_get_value(GTK_SPIN_BUTTON(c->enpo)));
	c->curve.SetShape(gtk_spin_button_get_value(GTK_SPIN_BUTTON(c->shape)));

	blackgenselector_refresh(c);

	g_signal_emit(G_OBJECT (c),blackgenselector_signals[CHANGED_SIGNAL], 0);
}


static void paint_graph(GtkWidget *widget,GdkEventExpose *eev,gpointer userdata)
{
	BlackGenSelector *c=(BlackGenSelector *)userdata;

	int xoff=15;
	int yoff=15;

	int width  = widget->allocation.width-xoff;
	int height = widget->allocation.height-yoff;

	cairo_t *cr = gdk_cairo_create (widget->window);
	if(!cr)
		return;

	cairo_set_source_rgb (cr, 1,1,1);
	cairo_paint (cr);

	cairo_translate(cr,xoff,0);

	// Gradient

	for(int i=0;i<10;++i)
	{
		float c=9-i; c/=9.0;
		cairo_new_path(cr);
		cairo_set_source_rgb (cr, c,c,c);
		cairo_rectangle(cr,(width*i)/10,height,width/10,height+yoff-1);
		cairo_close_path(cr);
		cairo_fill(cr);
	}

	// Cyan block
	cairo_new_path(cr);
	cairo_set_source_rgb(cr,0.2,0.8,0.9);
	cairo_rectangle(cr,-xoff,height-yoff,xoff/3,yoff);
	cairo_close_path(cr);
	cairo_fill(cr);

	// Magenta block
	cairo_new_path(cr);
	cairo_set_source_rgb(cr,0.9,0.2,0.8);
	cairo_rectangle(cr,-(2*xoff)/3,height-yoff,xoff/3,yoff);
	cairo_close_path(cr);
	cairo_fill(cr);

	// Yellow block
	cairo_new_path(cr);
	cairo_set_source_rgb(cr,0.9,0.8,0.2);
	cairo_rectangle(cr,-xoff/3,height-yoff,xoff/3,yoff);
	cairo_close_path(cr);
	cairo_fill(cr);


	// Black block
	cairo_new_path(cr);
	cairo_set_source_rgb(cr,0.0,0.0,0.0);
	cairo_rectangle(cr,-xoff,0,xoff,yoff);
	cairo_close_path(cr);
	cairo_fill(cr);


	// Half-way


	// Cyan block
	cairo_new_path(cr);
	cairo_set_source_rgb(cr,0.6,0.9,0.95);
	cairo_rectangle(cr,-xoff,height/2,xoff/3,yoff);
	cairo_close_path(cr);
	cairo_fill(cr);

	// Magenta block
	cairo_new_path(cr);
	cairo_set_source_rgb(cr,0.95,0.6,0.9);
	cairo_rectangle(cr,-(2*xoff)/3,height/2,xoff/3,yoff);
	cairo_close_path(cr);
	cairo_fill(cr);

	// Yellow block
	cairo_new_path(cr);
	cairo_set_source_rgb(cr,0.95,0.9,0.6);
	cairo_rectangle(cr,-xoff/3,height/2,xoff/3,yoff);
	cairo_close_path(cr);
	cairo_fill(cr);


	// Black block
	cairo_new_path(cr);
	cairo_set_source_rgb(cr,0.5,0.5,0.5);
	cairo_rectangle(cr,-xoff,height/2-yoff,xoff,yoff);
	cairo_close_path(cr);
	cairo_fill(cr);



	// Axes

	cairo_set_source_rgb (cr, 0,0,0);
	cairo_set_line_width(cr,1.0);
	cairo_move_to (cr, 1,0);
	cairo_line_to(cr,1,height-1);
	cairo_move_to(cr,0,height-1);
	cairo_line_to (cr, width,height-1);
	cairo_stroke (cr);

	

	cairo_move_to(cr,0,height-height*c->curve.GetLevel(0.0));

	for(int i=1;i<width;++i)
	{
		double in=i;
		in/=width;
		double out=c->curve.GetLevel(in);
		cairo_line_to(cr,i,height-height*out);
	}
	cairo_stroke(cr);

	cairo_destroy (cr);
}


GtkWidget *blackgenselector_new()
{
	BlackGenSelector *c=BLACKGENSELECTOR(g_object_new (blackgenselector_get_type (), NULL));

	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(c),vbox,FALSE,FALSE,8);
	gtk_widget_show(vbox);

	GtkWidget *table=gtk_table_new(2,6,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),4);
	gtk_box_pack_start(GTK_BOX(vbox),table,FALSE,FALSE,8);
	gtk_widget_show(table);

	GtkWidget *tmp=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),tmp,TRUE,TRUE,0);
	gtk_widget_show(tmp);

	GtkWidget *label;

	// Mode combo

//	label=gtk_label_new(_("Black generation:"));
//	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
//	gtk_widget_show(label);

	int row=0;

	SimpleComboOptions opts;

	opts.Add("",_("Absolute black (target, -k)"),_("The black generation will attempt to use the actual level of black specified by the curve"));
	opts.Add("",_("Proportional black (locus, -K)"),_("The black generation will pick a black level between the minimum and maximum possible, based on the curve"));
	c->locus=simplecombo_new(opts);
	gtk_table_attach_defaults(GTK_TABLE(table),c->locus,1,2,row,row+1);
	gtk_widget_show(c->locus);

	label=gtk_label_new(_("Black generation uses:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
	gtk_widget_show(label);

	++row;

	opts.Clear();
	opts.Add("",_("Minimum black"));
	opts.Add("",_("Medium black"));
	opts.Add("",_("Maximum black"));
	opts.Add("",_("Ramp min to max"));
	opts.Add("",_("Custom"));
	opts.Add("",_("Transfer from source profile"));
	opts.Add("",_("Retain from target profile"));

	c->combo=simplecombo_new(opts);
	gtk_table_attach_defaults(GTK_TABLE(table),c->combo,1,2,row,row+1);
	gtk_widget_show(c->combo);

	++row;

	// Spin buttons

	// Start Level

	label=gtk_label_new(_("Start level:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
	gtk_widget_show(label);

	c->stle=gtk_spin_button_new_with_range(0.0,1.0,0.05);
	gtk_table_attach_defaults(GTK_TABLE(table),c->stle,1,2,row,row+1);
	gtk_widget_show(c->stle);

	++row;

	// Start Position

	label=gtk_label_new(_("Start position:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
	gtk_widget_show(label);

	c->stpo=gtk_spin_button_new_with_range(0.0,1.0,0.05);
	gtk_table_attach_defaults(GTK_TABLE(table),c->stpo,1,2,row,row+1);
	gtk_widget_show(c->stpo);

	++row;

	// End Position

	label=gtk_label_new(_("End position:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
	gtk_widget_show(label);

	c->enpo=gtk_spin_button_new_with_range(0.0,1.0,0.05);
	gtk_table_attach_defaults(GTK_TABLE(table),c->enpo,1,2,row,row+1);
	gtk_widget_show(c->enpo);

	++row;

	// End Level

	label=gtk_label_new(_("End level:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
	gtk_widget_show(label);

	c->enle=gtk_spin_button_new_with_range(0.0,1.0,0.05);
	gtk_table_attach_defaults(GTK_TABLE(table),c->enle,1,2,row,row+1);
	gtk_widget_show(c->enle);

	++row;

	// Shape

	label=gtk_label_new(_("Shape:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,row,row+1);
	gtk_widget_show(label);

	c->shape=gtk_spin_button_new_with_range(0.0,2.0,0.05);
	gtk_table_attach_defaults(GTK_TABLE(table),c->shape,1,2,row,row+1);
	gtk_widget_show(c->shape);

	// Graph

	c->canvas = gtk_drawing_area_new();
	g_signal_connect(G_OBJECT (c->canvas), "expose-event",G_CALLBACK(paint_graph), c);
	gtk_box_pack_start(GTK_BOX(c),c->canvas,TRUE,TRUE,0);
	gtk_widget_show(c->canvas);

	// Plumbing

	blackgenselector_refresh(c);

	g_signal_connect(c->locus,"changed",G_CALLBACK(blackgenselector_locus_changed),c);
	g_signal_connect(c->combo,"changed",G_CALLBACK(blackgenselector_mode_changed),c);
	g_signal_connect(c->stle,"value-changed",G_CALLBACK(blackgenselector_params_changed),c);
	g_signal_connect(c->stpo,"value-changed",G_CALLBACK(blackgenselector_params_changed),c);
	g_signal_connect(c->enle,"value-changed",G_CALLBACK(blackgenselector_params_changed),c);
	g_signal_connect(c->enpo,"value-changed",G_CALLBACK(blackgenselector_params_changed),c);
	g_signal_connect(c->shape,"value-changed",G_CALLBACK(blackgenselector_params_changed),c);
	
	return(GTK_WIDGET(c));
}


GType
blackgenselector_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo blackgenselector_info =
		{
			sizeof (BlackGenSelectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) blackgenselector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (BlackGenSelector),
			0,
			(GInstanceInitFunc) blackgenselector_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "BlackGenSelector", &blackgenselector_info, GTypeFlags(0));
	}
	return stpuic_type;
}


static void
blackgenselector_class_init (BlackGenSelectorClass *klass)
{
	blackgenselector_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (BlackGenSelectorClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
blackgenselector_init (BlackGenSelector *c)
{
	c->combo=NULL;
	new (&c->curve) Argyll_BlackGenerationCurve;
	c->blockupdate=false;
}


void blackgenselector_refresh(BlackGenSelector *c)
{
	c->blockupdate=true;

	simplecombo_set_index(SIMPLECOMBO(c->locus),c->curve.GetLocusMode() ? 1 : 0);

	simplecombo_set_index(SIMPLECOMBO(c->combo),c->curve.GetMode());

	bool sens=(c->curve.GetMode()==ARGYLLBG_CUSTOM);
	gtk_widget_set_sensitive(c->enle,sens);
	gtk_widget_set_sensitive(c->enpo,sens);
	gtk_widget_set_sensitive(c->stle,sens);
	gtk_widget_set_sensitive(c->stpo,sens);
	gtk_widget_set_sensitive(c->shape,sens);


	gtk_spin_button_set_value(GTK_SPIN_BUTTON(c->stle),c->curve.GetStartLevel());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(c->enle),c->curve.GetEndLevel());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(c->stpo),c->curve.GetStartPos());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(c->enpo),c->curve.GetEndPos());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(c->shape),c->curve.GetShape());

	gtk_widget_queue_draw_area(c->canvas,0,0,c->canvas->allocation.width,c->canvas->allocation.height);
//	paint_graph(c->canvas,NULL,c);

	c->blockupdate=false;
}


Argyll_BlackGenerationCurve &blackgenselector_get(BlackGenSelector *c)
{
	return(c->curve);
}


void blackgenselector_set(BlackGenSelector *c,Argyll_BlackGenerationCurve &curve)
{
	c->curve=curve;
	blackgenselector_refresh(c);
}

