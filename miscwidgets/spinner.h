#ifndef SPINNER_H
#define SPINNER_H

#ifdef HAVE_GTK

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

#endif // HAVE_GTK
#endif

