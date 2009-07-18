#include <iostream>
#include <cstring>
#include <cstdlib>

#include <gtk/gtk.h>

#include "miscwidgets/simplecombo.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)

using namespace std;

#if 0
class SimpleComboOption;
class SimpleComboOptions
{
	public:
	SimpleComboOptions();
	SimpleComboOptions(SimpleComboOptions &other);
	~SimpleComboOptions();
	SimpleComboOption *Add(const char *key,const char *displayname,const char *tooltip=NULL);
	SimpleComboOption *FirstOption();
	SimpleComboOption *operator[](int idx);
	protected:
	SimpleComboOption *firstopt;
	friend class SimpleComboOption;
};


class SimpleComboOption
{
	public:
	SimpleComboOption(SimpleComboOptions &header,const char *key,const char *displayname,const char *tooltip=NULL);
	SimpleComboOption(SimpleComboOptions &header,SimpleComboOption &other);
	~SimpleComboOption();
	SimpleComboOption *NextOption();
	SimpleComboOption *PrevOption();
	char *key;
	char *displayname;
	char *tooltip;
	protected:
	SimpleComboOptions &header;
	SimpleComboOption *prevopt,*nextopt;
};


SimpleComboOption::SimpleComboOption(SimpleComboOptions &header,const char *key,const char *displayname,const char *tooltip)
	:	key(NULL),displayname(NULL),tooltip(NULL),header(header),prevopt(NULL),nextopt(NULL)
{
	if((prevopt=header.firstopt))
	{
		while(prevopt->nextopt)
			prevopt=prevopt->nextopt;
		prevopt->nextopt=this;
	}
	else
		header.firstopt=this;

	this->key=strdup(key);
	this->displayname=strdup(displayname);
	if(tooltip)
		this->tooltip=strdup(tooltip);
}


SimpleComboOption::SimpleComboOption(SimpleComboOptions &header,SimpleComboOption &other)
	: key(NULL),displayname(NULL),tooltip(NULL),header(header),prevopt(NULL),nextopt(NULL)
{
	if((prevopt=header.firstopt))
	{
		while(prevopt->nextopt)
			prevopt=prevopt->nextopt;
		prevopt->nextopt=this;
	}
	else
		header.firstopt=this;

	key=strdup(other.key);
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
	while(firstopt)
		delete firstopt;
}

SimpleComboOption *SimpleComboOptions::Add(const char *key,const char *displayname,const char *tooltip)
{
	return(new SimpleComboOption(*this,key,displayname,tooltip));
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
	return(opt);
}
#endif

int main(int argc, char **argv)
{
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

