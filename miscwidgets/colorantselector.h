#ifndef __COLORANTSELECTOR_H__
#define __COLORANTSELECTOR_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreestore.h>
#include <gtk/gtkcellrendererpixbuf.h>

#include "imagesource/devicencolorant.h"


#define COLORANTSELECTOR_TYPE			(colorantselector_get_type())
#define COLORANTSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), COLORANTSELECTOR_TYPE, ColorantSelector))
#define COLORANTSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), COLORANTSELECTOR_TYPE, ColorantSelectorClass))
#define IS_COLORANTSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), COLORANTSELECTOR_TYPE))
#define IS_COLORANTSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), COLORANTSELECTOR_TYPE))

typedef struct _ColorantSelector ColorantSelector;
typedef struct _ColorantSelectorClass ColorantSelectorClass;

struct _ColorantSelector
{
	GtkVBox box;
	GtkTreeStore *treestore;
	GtkWidget *treeview;
	DeviceNColorantList *list;
	GtkWidgetClass *parent_class;
};


struct _ColorantSelectorClass
{
	GtkVBoxClass parent_class;

	void (*changed)(ColorantSelector *es);
};

GType colorantselector_get_type (void);

GtkWidget* colorantselector_new(DeviceNColorantList *list);
void colorantselector_refresh(ColorantSelector *c);
void colorantselector_set_colorants(ColorantSelector *c,DeviceNColorantList *list);


#endif /* __COLORANTSELECTOR_H__ */
