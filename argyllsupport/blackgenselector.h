#ifndef __BLACKGENSELECTOR_H__
#define __BLACKGENSELECTOR_H__

#ifdef HAVE_GTK

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtktooltips.h>

#include "argyllbg.h"

#define BLACKGENSELECTOR_TYPE			(blackgenselector_get_type())
#define BLACKGENSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), BLACKGENSELECTOR_TYPE, BlackGenSelector))
#define BLACKGENSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), BLACKGENSELECTOR_TYPE, BlackGenSelectorClass))
#define IS_BLACKGENSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BLACKGENSELECTOR_TYPE))
#define IS_BLACKGENSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), BLACKGENSELECTOR_TYPE))

typedef struct _BlackGenSelector BlackGenSelector;
typedef struct _BlackGenSelectorClass BlackGenSelectorClass;

struct _BlackGenSelector
{
	GtkHBox box;
	GtkWidget *locus;
	GtkWidget *combo;
	GtkWidget *stle;
	GtkWidget *stpo;
	GtkWidget *enle;
	GtkWidget *enpo;
	GtkWidget *shape;
	GtkWidget *canvas;
	Argyll_BlackGenerationCurve curve;
	bool blockupdate;
};


struct _BlackGenSelectorClass
{
	GtkHBoxClass parent_class;

	void (*changed)(BlackGenSelector *combo);
};

GType blackgenselector_get_type (void);
GtkWidget* blackgenselector_new ();
void blackgenselector_set(BlackGenSelector *c,Argyll_BlackGenerationCurve &curve);
Argyll_BlackGenerationCurve &blackgenselector_get(BlackGenSelector *c);
void blackgenselector_refresh(BlackGenSelector *c);

#endif /* HAVE_GTK */
#endif /* __BLACKGENSELECTOR_H__ */
