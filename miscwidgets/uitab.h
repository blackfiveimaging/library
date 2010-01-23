#ifndef UITAB_H
#define UITAB_H

#include <gtk/gtk.h>
#include "refcountui.h"

class UITab;


class UITab : public RefCountUI
{
	public:
	UITab(GtkWidget *notebook,const char *tabname=NULL);
	virtual ~UITab();
	GtkWidget *GetBox();
	void SetTabText(const char *text);
	void AddTabWidget(GtkWidget *widget);	// Inserts a widget between label and close button
	void AddTabButton(GtkWidget *button);	// As above but removes border relief and sets custom style
	protected:
	static void deleteclicked(GtkWidget *wid,gpointer userdata);
	static void setclosebuttonsize(GtkWidget *wid,GtkStyle *style,gpointer userdata);
	static bool style_applied;
	static void apply_style();
	GtkWidget *hbox;
	GtkWidget *labelbox;
	GtkWidget *label;
	GtkWidget *notebook;
};

#endif

