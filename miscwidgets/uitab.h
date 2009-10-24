#ifndef UITAB_H
#define UITAB_H

#include <gtk/gtk.h>
#include "../support/refcount.h"
#include "../support/thread.h"
#include "../support/debug.h"

class UITab;

class RefCountUI : public RefCount
{
	public:
	RefCountUI() : RefCount()
	{
		threadid=Thread::GetThreadID();
	}
	virtual void ObtainRefMutex()
	{
		while(!RefCount::refmutex.AttemptMutex())
		{
			gtk_main_iteration_do(TRUE);
		}
	}
	virtual void UnRef()
	{
		if(threadid==Thread::GetThreadID())
		{
			RefCount::UnRef();
		}
		else
		{
			Debug[TRACE] << "RefCountUI: Unreferencing from a different thread - deferring..." << std::endl;
			g_timeout_add(1,unreffunc,this);
		}
	}
	protected:
	static gboolean unreffunc(gpointer ud)
	{
		RefCountUI *rc=(RefCountUI *)ud;
		rc->RefCount::UnRef();
		return(FALSE);
	}
	
	ThreadID threadid;
};


class UITab : public RefCountUI
{
	public:
	UITab(GtkWidget *notebook,const char *tabname=NULL);
	virtual ~UITab();
	GtkWidget *GetBox();
	void SetText(const char *text);
	protected:
	static void deleteclicked(GtkWidget *wid,gpointer userdata);
	static void setclosebuttonsize(GtkWidget *wid,GtkStyle *style,gpointer userdata);
	static bool style_applied;
	static void apply_style();
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *notebook;
};

#endif

