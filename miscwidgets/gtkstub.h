#ifndef GTK_STUB_H

#ifdef HAVE_GTK

#include <gtk/gtk.h>

#else

// A few defines to simplify building without GTK
typedef void * gpointer;
typedef void GtkWidget;
typedef void GdkPixbuf;
typedef unsigned char guint8;
typedef bool gboolean;

#endif // HAVE_GTK

#endif

