#ifndef __PRINTOUTPUTSELECTOR_H__
#define __PRINTOUTPUTSELECTOR_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkspinbutton.h>

#include <gutenprint/gutenprint.h>

#include "printoutput.h"

G_BEGIN_DECLS

#define PRINTOUTPUTSELECTOR_TYPE			(printoutputselector_get_type())
#define PRINTOUTPUTSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PRINTOUTPUTSELECTOR_TYPE, PrintOutputSelector))
#define PRINTOUTPUTSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PRINTOUTPUTSELECTOR_TYPE, PrintOutputSelectorClass))
#define IS_PRINTOUTPUTSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRINTOUTPUTSELECTOR_TYPE))
#define IS_PRINTOUTPUTSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PRINTOUTPUTSELECTOR_TYPE))

typedef struct _PrintOutputSelector PrintOutputSelector;
typedef struct _PrintOutputSelectorClass PrintOutputSelectorClass;

struct _PrintOutputSelector
{
	GtkVBox	box;
	GtkWidget *string;
	GtkWidget *combo;
	GtkWidget *printersel;
	GtkWidget *driverhint;
	GList *queues;
	PrintOutput *po;
};


struct _PrintOutputSelectorClass
{
	GtkVBoxClass parent_class;

	void (*changed)(PrintOutputSelector *book);
};

GType printoutputselector_get_type (void);
GtkWidget* printoutputselector_new (PrintOutput *po);
void printoutputselector_refresh(PrintOutputSelector *ob);

void printoutput_queue_dialog(PrintOutput *po);

G_END_DECLS

#endif /* __PRINTOUTPUTSELECTOR_H__ */
