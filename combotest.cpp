#include <iostream>
#include <cstring>
#include <cstdlib>

#include <gtk/gtk.h>

#include "config.h"
#include "miscwidgets/simplecombo.h"
#include "breakhandler.h"
#include "gettext.h"
#define _(x) gettext(x)

using namespace std;

gboolean updatefunc(gpointer ud)
{
	SimpleCombo *c=SIMPLECOMBO(ud);

	if(BreakHandler.TestBreak())
		std::cerr << "Break signal received..." << std::endl;
	else
		std::cerr << "No break signal received..." << std::endl;

	int idx=simplecombo_get_index(c);
	if(idx==2)
	{
		SimpleComboOptions opts;
		opts.Add("Opt2_1","New opts: Option 1");
		opts.Add("Opt2_2","New opts: Option 2","Option 2_2 has a tooltip...");
		opts.Add("Opt2_3","New opts: Option 3","So does option 2_3!");
		simplecombo_set_opts(c,opts);
	}
	return(FALSE);
}


void combo_changed(GtkWidget *wid,gpointer userdata)
{
	SimpleCombo *c=SIMPLECOMBO(wid);
	g_timeout_add(1,updatefunc,c);
}


int main(int argc, char **argv)
{
	Debug.SetLevel(TRACE);
	gtk_init(&argc,&argv);

	SimpleComboOptions opts;
	opts.Add("Opt1","Option 1");
	opts.Add("Opt2","Option 2","Option 2 has a tooltip...");
	opts.Add("Opt3","Option 3","So does option 3!");

	try
	{
		GtkWidget *win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title (GTK_WINDOW (win), _("PixBufView Test"));
		gtk_signal_connect (GTK_OBJECT (win), "delete_event",
			(GtkSignalFunc) gtk_main_quit, NULL);

		GtkWidget *combo=simplecombo_new(opts);
		g_signal_connect(GTK_OBJECT(combo),"changed",G_CALLBACK(combo_changed),NULL);
		gtk_container_add(GTK_CONTAINER(win),combo);
		gtk_widget_show(combo);
		gtk_widget_show(win);

		gtk_main();
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}

	return(0);
}

