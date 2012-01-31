#ifndef __PIXBUFVIEW_H__
#define __PIXBUFVIEW_H__

#include <deque>

#ifdef HAVE_GTK

#include <gdk/gdk.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PIXBUFVIEW(obj)          GTK_CHECK_CAST (obj, pixbufview_get_type (), PixbufView)
#define PIXBUFVIEW_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, pixbufview_get_type (), PixbufViewClass)
#define IS_PIXBUFVIEW(obj)       GTK_CHECK_TYPE (obj, pixbufview_get_type ())


typedef struct _PixbufView        PixbufView;
typedef struct _PixbufViewClass   PixbufViewClass;


// Signals:
//
// "changed" - emitted when the view changes
// "mousemove" - emitted when the mouse position changes but the widget is not being manipulated

struct _PixbufView
{
	GtkWidget wid;
	GdkPixbuf *pb_scaled;
	bool resized;
	bool scaletofit;
	int xoffset,yoffset;
	bool dragging;
	int prev_x,prev_y;
	unsigned int currentpage;
	int mousex,mousey;
	std::deque<GdkPixbuf *> pages;
};


struct _PixbufViewClass
{
	GtkWidgetClass parent_class;
	
	void (*changed)(PixbufView *pv);
	void (*mousemove)(PixbufView *pv);
	void (*popupmenu)(PixbufView *pv);
};


GtkWidget* pixbufview_new(GdkPixbuf *pb=NULL,bool scaletofit=true);
GtkType pixbufview_get_type(void);

void pixbufview_set_pixbuf(PixbufView *pv,GdkPixbuf *pb,unsigned int page=0);
GdkPixbuf *pixbufview_get_pixbuf(PixbufView *pv,unsigned int page=0);

void pixbufview_refresh(PixbufView *pv);

// Get mouse position, in image coordinates.
int pixbufview_get_mousex(PixbufView *pv);
int pixbufview_get_mousey(PixbufView *pv);

// Get view parameters.
int pixbufview_get_xoffset(PixbufView *pv);
int pixbufview_get_yoffset(PixbufView *pv);
bool pixbufview_get_scale(PixbufView *pv);

// Set view parameters
void pixbufview_set_offset(PixbufView *pv,int xoff,int yoff);
void pixbufview_set_scale(PixbufView *pv,bool scaletofit);

// Add and remove pages
void pixbufview_add_page(PixbufView *pv,GdkPixbuf *pb);
void pixbufview_set_page(PixbufView *pb,unsigned int page);
void pixbufview_clear_pages(PixbufView *pb);

G_END_DECLS
#endif // HAVE_GTK
#endif /* __PIXBUFVIEW_H__ */
