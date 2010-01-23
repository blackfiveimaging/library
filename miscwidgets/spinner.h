#ifndef SPINNER_H
#define SPINNER_H

#include <gtk/gtkwidget.h>
#include <gdk/gdkpixbuf.h>

class Spinner
{
	public:
	Spinner();
	virtual ~Spinner();
	void SetFrame(int f);
	GtkWidget *GetWidget();
	protected:
	GdkPixbuf *frames[8];
	GtkWidget *spinner;
};

#endif

