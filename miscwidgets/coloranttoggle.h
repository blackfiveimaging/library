#ifndef __COLORANTTOGGLE_H__
#define __COLORANTTOGGLE_H__

#include <deque>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreestore.h>
#include <gtk/gtkcellrendererpixbuf.h>

#include "imagesource/devicencolorant.h"


#define COLORANTTOGGLE_TYPE			(coloranttoggle_get_type())
#define COLORANTTOGGLE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), COLORANTTOGGLE_TYPE, ColorantToggle))
#define COLORANTTOGGLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), COLORANTTOGGLE_TYPE, ColorantToggleClass))
#define IS_COLORANTTOGGLE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), COLORANTTOGGLE_TYPE))
#define IS_COLORANTTOGGLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), COLORANTTOGGLE_TYPE))

typedef struct _ColorantToggle ColorantToggle;
typedef struct _ColorantToggleClass ColorantToggleClass;

class ToggleData
{
	public:
	ToggleData(ColorantToggle *toggle,GtkWidget *button,DeviceNColorant &col)
		: toggle(toggle), button(button), col(col)
	{
	}
	~ToggleData()
	{
		gtk_widget_destroy(GTK_WIDGET(toggle));
	}
	static void toggled(GtkWidget *wid,gpointer user_data);
	void refresh();
	protected:
	ColorantToggle *toggle;
	GtkWidget *button;
	DeviceNColorant &col;
};


struct _ColorantToggle
{
	GtkHBox box;
	std::deque<ToggleData *> buttons;
	GtkWidgetClass *parent_class;
};


struct _ColorantToggleClass
{
	GtkHBoxClass parent_class;

	void (*changed)(ColorantToggle *es);
};

GType coloranttoggle_get_type (void);

GtkWidget* coloranttoggle_new(DeviceNColorantList *list);
void coloranttoggle_refresh(ColorantToggle *c);
void coloranttoggle_set_colorants(ColorantToggle *c,DeviceNColorantList *list);


#endif /* __COLORANTTOGGLE_H__ */
