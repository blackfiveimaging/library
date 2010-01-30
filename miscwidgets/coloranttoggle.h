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
#include <cairo.h>

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
	ToggleData(ColorantToggle *toggle,DeviceNColorant &col)
		: toggle(toggle), button(NULL), canvas(NULL), col(col), level(-1)
	{
		canvas = gtk_drawing_area_new();
		gtk_widget_set_size_request(canvas,20,20);
		g_signal_connect(G_OBJECT (canvas), "expose-event",G_CALLBACK(paint), this);
		gtk_widget_show(canvas);

		button=gtk_check_button_new();
		gtk_button_set_image(GTK_BUTTON(button),canvas);
		gtk_box_pack_start(GTK_BOX(toggle),button,FALSE,FALSE,0);
		gtk_widget_show(button);

//		gtk_box_pack_start(GTK_BOX(toggle),canvas,TRUE,TRUE,0);
	}
	~ToggleData()
	{
		gtk_widget_destroy(GTK_WIDGET(button));
	}
	static void toggled(GtkWidget *wid,gpointer user_data);
	static void paint(GtkWidget *widget,GdkEventExpose *eev,gpointer userdata);
	void refresh(int level=-1);
	protected:
	ColorantToggle *toggle;
	GtkWidget *button;
	GtkWidget *canvas;
	DeviceNColorant &col;
	int level;
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
void coloranttoggle_set_value(ColorantToggle *c,ISDeviceNValue &value);


#endif /* __COLORANTTOGGLE_H__ */
