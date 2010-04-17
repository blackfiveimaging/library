#ifndef PP_PROGRESS_H
#define PP_PROGRESS_H

#include "progress.h"
#include "thread.h"
#include "refcountui.h"
#include <gtk/gtkwidget.h>

class ProgressBar : public Progress
{
	public:
	ProgressBar(const char *message,bool cancelbutton,GtkWidget *parent=NULL,bool modal=false);
	~ProgressBar();
	bool DoProgress(int i,int maxi);
	void SetMessage(const char *msg);
	void ErrorMessage(const char *msg);
	static void cancel_callback(GtkWidget *wid,gpointer *ob);
	static gboolean update(gpointer ud);
	private:
	char *message;
	GtkWidget *window;
	GtkWidget *progressbar;
	GtkWidget *label;
	bool cancelled;
	int current,max;
	ThreadID threadid;
};


class ProgressBar_RefCount : public ProgressBar, public RefCountUI
{
	public:
	ProgressBar_RefCount(const char *message,bool cancelbutton,GtkWidget *parent=NULL,bool modal=false)
		: ProgressBar(message,cancelbutton,parent,modal), RefCountUI()
	{
	}
};

#endif
