#include <iostream>

#include "config.h"
#include "miscwidgets/errordialogqueue.h"


int main(int argc,char **argv)
{
	gtk_init(&argc,&argv);

	GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window),600,450);
	gtk_window_set_title (GTK_WINDOW (window), "ErrorQueue Test");
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		(GtkSignalFunc) gtk_main_quit, NULL);
	gtk_widget_show(window);

	ErrorDialogs.SetParent(window);

	ErrorDialogs.AddMessage("Testing - line 1");
	ErrorDialogs.AddMessage("Testing - line 2");
	ErrorDialogs.AddMessage("Testing - line 3");
//	cerr << "Messages so far: " << queue.GetMessages() << endl;

	ErrorDialogs.AddMessage("Testing - line 4");
	ErrorDialogs.AddMessage("Testing - line 5");
//	cerr << "Messages so far: " << queue.GetMessages() << endl;

	gtk_main();

	return(0);
}

