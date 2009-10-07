#include <iostream>
#include <cstring>

#include "imagesource/imagesource.h"

#include <gutenprint/gutenprint.h>
#include <gtk/gtk.h>

#include "stpui_widgets/stpui_optionbook.h"

#include "gp_cppsupport/gprinter.h"
#include "support/md5.h"

using namespace std;

int main(int argc, char **argv)
{
	gtk_init(&argc,&argv);
	stp_init();
	ConfigFile config;
	PrintOutput printoutput(&config,"[Output]");
	GPrinter printer(printoutput,&config,"[Print]");

	if(argc>1)
		config.ParseConfigFile(argv[1]);

	GtkWidget *dialog=gtk_dialog_new_with_buttons("Printer Setup",
		NULL,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);
	gtk_window_set_default_size(GTK_WINDOW(dialog),500,350);

	GtkWidget *optionbook=stpui_optionbook_new(printer.stpvars,NULL,0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),optionbook,TRUE,TRUE,0);
	gtk_widget_show(optionbook);

	gtk_widget_show(dialog);
	gint result=gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_OK:
			break;
	}
	gtk_widget_destroy(dialog);

	string hash=printer.GetResponseHash();

	cerr << "Hash is: " << hash << endl;

	return(0);
}

