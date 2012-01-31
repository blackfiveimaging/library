#ifndef __VIEWINGCONDSELECTOR_H__
#define __VIEWINGCONDSELECTOR_H__

#ifdef HAVE_GTK

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtktooltips.h>

G_BEGIN_DECLS

#define VIEWINGCONDSELECTOR_TYPE			(viewingcondselector_get_type())
#define VIEWINGCONDSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), VIEWINGCONDSELECTOR_TYPE, ViewingCondSelector))
#define VIEWINGCONDSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), VIEWINGCONDSELECTOR_TYPE, ViewingCondSelectorClass))
#define IS_VIEWINGCONDSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), VIEWINGCONDSELECTOR_TYPE))
#define IS_VIEWINGCONDSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), VIEWINGCONDSELECTOR_TYPE))

typedef struct _ViewingCondSelector ViewingCondSelector;
typedef struct _ViewingCondSelectorClass ViewingCondSelectorClass;

struct _ViewingCondSelector
{
	GtkHBox box;
	GtkWidget *optionmenu;
	GtkWidget *menu;
	GtkTooltips *tips;
	int previdx;
};


struct _ViewingCondSelectorClass
{
	GtkHBoxClass parent_class;

	void (*changed)(ViewingCondSelector *combo);
};

GType viewingcondselector_get_type (void);
GtkWidget* viewingcondselector_new ();
const char *viewingcondselector_get(ViewingCondSelector *c);
bool viewingcondselector_set(ViewingCondSelector *c,const char *cond);
G_END_DECLS

#endif /* HAVE_GTK */
#endif /* __VIEWINGCONDSELECTOR_H__ */
