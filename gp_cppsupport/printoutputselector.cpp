// FIXME - need a destructor to clean up the printer queue strings.

#include <iostream>

#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkstock.h>

#include "stpui_widgets/stpui_queue.h"
#include "stpui_widgets/stpui_printerselector.h"

#include "../support/debug.h"
#include "../miscwidgets/livedisplaycheck.h"

#include "printoutputselector.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)

using namespace std;

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint printoutputselector_signals[LAST_SIGNAL] = { 0 };

static void printoutputselector_class_init (PrintOutputSelectorClass *klass);
static void printoutputselector_init (PrintOutputSelector *stpuicombo);


static void printersel_changed(GtkWidget *wid,gpointer *ob)
{
	Debug[TRACE] << "In printersel_changed()" << endl;
	PrintOutputSelector *lo=(PrintOutputSelector *)ob;

	const char *driver=stpui_printerselector_get_driver(STPUI_PRINTERSELECTOR(wid));
	if(driver)
	{
		lo->po->SetString("Driver",driver);

		g_signal_emit(G_OBJECT (ob),printoutputselector_signals[CHANGED_SIGNAL], 0);
	}
}


static void printoutputselector_queue_changed(GtkEntry *entry,gpointer *ud)
{
	Debug[TRACE] << "In printoutputselectorqueue_changed()" << endl;

	PrintOutputSelector *ob=PRINTOUTPUTSELECTOR(ud);
	PrintOutput *po=ob->po;

	Debug[TRACE] << "Getting printer queue..." << endl;

	const char *val=po->GetPrinterQueue();
	if(val && strlen(val))
	{
		Debug[TRACE] << "Got printer queue: " << val << endl;
		char *driver=po->GetPrinterDriver();
		if(driver)
		{
			if(driver[0]=='p' && driver[1]=='s')
			{
#ifdef HAVE_LIBCUPS
				gtk_label_set_text(GTK_LABEL(ob->driverhint),_("Can't automatically identify the correct driver for this queue.\nIf in doubt, use Adobe Postscript Level 2."));
#else
				gtk_label_set_text(GTK_LABEL(ob->driverhint),_("This program has been built without CUPS support,\nand can't identify the correct driver for this queue.\nIf in doubt, use Adobe Postscript Level 2."));
#endif
			}
			else
				gtk_label_set_text(GTK_LABEL(ob->driverhint),_("A driver has been automatically detected for this queue."));
			Debug[TRACE] << "Got driver: " << driver << " from Queue" << endl;
			po->SetString("Driver",driver);
			stpui_printerselector_set_driver(STPUI_PRINTERSELECTOR(ob->printersel),driver);
			free(driver);
		}
		else
		{
#ifdef WIN32
			gtk_label_set_text(GTK_LABEL(ob->driverhint),_("Please note: native Windows drivers are not currently\nsupported, so only printers directly supported by Gutenprint\ncan currently be used."));
#else
#ifdef HAVE_LIBCUPS
			gtk_label_set_text(GTK_LABEL(ob->driverhint),_("Can't automatically identify the correct driver for this queue.\nIf in doubt, use Adobe Postscript Level 2."));
#else
			gtk_label_set_text(GTK_LABEL(ob->driverhint),_("This program has been built without CUPS support,\nand can't identify the correct driver for this queue.\nIf in doubt, use Adobe Postscript Level 2."));
#endif
#endif
		}
		g_signal_emit(G_OBJECT (ob),printoutputselector_signals[CHANGED_SIGNAL], 0);
	}
}



void printoutputselector_refresh(PrintOutputSelector *ob)
{
	Debug[TRACE] << "In printoutputselectorrefresh()" << endl;
	PrintOutput *po=ob->po;

	const char *driver=po->FindString("Driver");
	if(driver)
	{
		Debug[TRACE] << "Setting driver to " << driver << endl;
		stpui_printerselector_set_driver(STPUI_PRINTERSELECTOR(ob->printersel),driver);
	}
	const char *command=po->FindString("Command");
	if(command)
	{
		gtk_entry_set_text(GTK_ENTRY(ob->string),command);
	}
}


GtkWidget*
printoutputselector_new (PrintOutput *po)
{
	PrintOutputSelector *ob=PRINTOUTPUTSELECTOR(g_object_new (printoutputselector_get_type (), NULL));
	gtk_box_set_spacing(GTK_BOX(ob),5);

	ob->po=po;

	GtkWidget *label;

	GtkWidget *table=gtk_table_new(2,3,FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(ob),table,FALSE,FALSE,0);
	gtk_widget_show(table);

	label=gtk_label_new(_("Print Queue:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
	gtk_widget_show(label);

	Debug[TRACE] << "Calling DBToQueues()" << endl;

	po->DBToQueues();

	Debug[TRACE] << "Getting PQInfo" << endl;
	struct pqinfo *pq=po->GetPQInfo();
	Debug[TRACE] << "Building stpui_queue" << endl;
	ob->combo=stpui_queue_new(pq);
	Debug[TRACE] << "Done" << endl;
	gtk_table_attach_defaults(GTK_TABLE(table),ob->combo,1,2,0,1);
	gtk_widget_show(ob->combo);

	g_signal_connect(GTK_WIDGET(ob->combo),"changed",G_CALLBACK(printoutputselector_queue_changed),ob);


	label=gtk_label_new(_("Printer Model:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	gtk_widget_show(label);

	ob->printersel=stpui_printerselector_new();
	gtk_table_attach_defaults(GTK_TABLE(table),ob->printersel,1,2,1,2);
	g_signal_connect(G_OBJECT(ob->printersel),"changed",G_CALLBACK(printersel_changed),ob);

	stpui_printerselector_set_driver(STPUI_PRINTERSELECTOR(ob->printersel),po->FindString("Driver"));
	gtk_widget_show(ob->printersel);
	
	ob->driverhint=gtk_label_new("");
	g_object_set (ob->driverhint, "xalign", 0.0, NULL);
	gtk_table_attach_defaults(GTK_TABLE(table),ob->driverhint,1,2,2,3);
	gtk_table_set_row_spacing(GTK_TABLE(table),1,4);
	gtk_widget_show(ob->driverhint);

	return(GTK_WIDGET(ob));
}


GType
printoutputselector_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo printoutputselector_info =
		{
			sizeof (PrintOutputSelectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) printoutputselector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (PrintOutputSelector),
			0,
			(GInstanceInitFunc) printoutputselector_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "PrintOutputSelector", &printoutputselector_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
printoutputselector_class_init (PrintOutputSelectorClass *klass)
{
	printoutputselector_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (PrintOutputSelectorClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
printoutputselector_init (PrintOutputSelector *ob)
{
	ob->po=NULL;
}


void printoutput_queue_dialog(PrintOutput *po)
{
	if(!LiveDisplay.HaveDisplay())
		throw "Running without a display - can't show dialog!";
	const char *oldqueue=po->FindString("Queue");
	char *labeltext=g_strdup_printf(_("The printer queue %s\n is not found - please choose another"),oldqueue);
	GtkWidget *dlg=gtk_dialog_new_with_buttons(_("Printer queue not found"),NULL,
		GtkDialogFlags(0),GTK_STOCK_OK,GTK_RESPONSE_OK,NULL);

	GtkWidget *vbox=gtk_vbox_new(FALSE,5);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox),vbox,TRUE,TRUE,8);
	gtk_widget_show(vbox);

	GtkWidget *label=gtk_label_new(labeltext);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
	g_free(labeltext);
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,0);

	Debug[TRACE] << "Getting PQInfo" << endl;
	struct pqinfo *pq=po->GetPQInfo();
	Debug[TRACE] << "Building stpui_queue" << endl;
	GtkWidget *combo=stpui_queue_new(pq);
	gtk_widget_show(combo);
	gtk_box_pack_start(GTK_BOX(vbox),combo,FALSE,FALSE,8);

	gint result=gtk_dialog_run(GTK_DIALOG(dlg));
	switch(result)
	{
		case GTK_RESPONSE_OK:
			po->QueuesToDB();
			gtk_widget_destroy(dlg);
			break;
	}
}
