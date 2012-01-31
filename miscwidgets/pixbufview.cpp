#include <iostream>
#include <math.h>
#include <stdio.h>

#include "config.h"

#ifdef HAVE_GTK
#include <gtk/gtk.h>

#include "pixbufview.h"

#include "../support/debug.h"

using namespace std;

enum {
	CHANGED_SIGNAL,
	MOUSEMOVE_SIGNAL,
	LAST_SIGNAL
};

static guint pixbufview_signals[LAST_SIGNAL] = { 0 };

static void pixbufview_class_init               (PixbufViewClass     *klass);
static void pixbufview_init                     (PixbufView          *pageview);
static void pixbufview_realize                  (GtkWidget        *widget);
static void pixbufview_size_request             (GtkWidget        *widget,
                                               GtkRequisition   *requisition);
static void pixbufview_size_allocate            (GtkWidget        *widget,
                                               GtkAllocation    *allocation);
static gboolean pixbufview_expose               (GtkWidget        *widget,
                                               GdkEventExpose   *event);
static gboolean	pixbufview_motion_notify( GtkWidget      *widget,GdkEventMotion *event );
static gboolean pixbufview_button_release( GtkWidget      *widget,GdkEventButton *event );
static gboolean pixbufview_button_press( GtkWidget      *widget,GdkEventButton *event );

static void pixbufview_destroy(GtkObject *object);

GType
pixbufview_get_type ()
{
  static GType pageview_type = 0;

  if (!pageview_type)
    {
      static const GTypeInfo pageview_info =
      {
	sizeof (PixbufViewClass),
	NULL,
	NULL,
	(GClassInitFunc) pixbufview_class_init,
	NULL,
	NULL,
	sizeof (PixbufView),
        0,
	(GInstanceInitFunc) pixbufview_init,
      };

      pageview_type = g_type_register_static (GTK_TYPE_WIDGET, "PixbufView", &pageview_info, GTypeFlags(0));
    }

  return pageview_type;
}


static void
pixbufview_class_init (PixbufViewClass *cl)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	
	object_class = (GtkObjectClass*) cl;
	widget_class = (GtkWidgetClass*) cl;
	
	object_class->destroy = pixbufview_destroy;	

	widget_class->realize = pixbufview_realize;
	widget_class->expose_event = pixbufview_expose;
	widget_class->size_request = pixbufview_size_request;
	widget_class->size_allocate = pixbufview_size_allocate;
	widget_class->button_press_event = pixbufview_button_press;
	widget_class->button_release_event = pixbufview_button_release;
	widget_class->motion_notify_event = pixbufview_motion_notify;

	pixbufview_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (cl),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (PixbufViewClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pixbufview_signals[MOUSEMOVE_SIGNAL] =
	g_signal_new ("mousemove",
		G_TYPE_FROM_CLASS (cl),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (PixbufViewClass, mousemove),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


GtkWidget *pixbufview_new (GdkPixbuf *pb,bool scaletofit)
{
	PixbufView *pv = PIXBUFVIEW(g_object_new (pixbufview_get_type (), NULL));

	pv->currentpage=0;

	pixbufview_set_scale(pv,scaletofit);
	if(pb)
		pixbufview_set_pixbuf(pv,pb);

	return GTK_WIDGET (pv);
}


static void
pixbufview_realize (GtkWidget *widget)
{
	PixbufView *pageview;
	GdkWindowAttr attributes;
	gint attributes_mask;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_PIXBUFVIEW (widget));

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
	pageview = PIXBUFVIEW (widget);

	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.event_mask = gtk_widget_get_events (widget) | 
		GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
		GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
		GDK_POINTER_MOTION_HINT_MASK;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
	widget->window = gdk_window_new (widget->parent->window, &attributes, attributes_mask);

	widget->style = gtk_style_attach (widget->style, widget->window);

	gdk_window_set_user_data (widget->window, widget);

	gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);
}


static void 
pixbufview_size_request (GtkWidget      *widget,
		       GtkRequisition *requisition)
{
	requisition->width = 128;
	requisition->height = 128;
}


static void
pixbufview_size_allocate (GtkWidget     *widget,
			GtkAllocation *allocation)
{
	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_PIXBUFVIEW (widget));
	g_return_if_fail (allocation != NULL);

	widget->allocation = *allocation;

	PixbufView *pv=PIXBUFVIEW(widget);

	if (GTK_WIDGET_REALIZED (widget))
	{
		gdk_window_move_resize (widget->window,
			allocation->x, allocation->y,
			allocation->width, allocation->height);

		pv->resized=true;
	}
}


static void draw_scaled(GtkWidget *widget,GdkEventExpose *event)
{
	PixbufView *pv=PIXBUFVIEW(widget);
	if(pv->resized)
	{
		if(pv->pb_scaled)
			g_object_unref(G_OBJECT(pv->pb_scaled));
		pv->pb_scaled=NULL;

		GdkPixbuf *pb=pixbufview_get_pixbuf(pv,pv->currentpage);
		if(pb)
		{
			int height=widget->allocation.height;
			int width=widget->allocation.width;

			int pw=gdk_pixbuf_get_width(pb);
			int ph=gdk_pixbuf_get_height(pb);

			int nw,nh;
			nh=height;
			nw=(pw*nh)/ph;
			if(nw>width)
			{
				nw=width;
				nh=(ph*nw)/pw;
			}
			pv->pb_scaled=gdk_pixbuf_scale_simple(pb,nw,nh,GDK_INTERP_BILINEAR);
			pv->resized=false;
		}
	}

	int height=widget->allocation.height;
	int width=widget->allocation.width;

	gdk_draw_rectangle (widget->window,
		widget->style->bg_gc[widget->state],TRUE,
		0,0,width,height);

	if(pv->pb_scaled)
	{
		int height=widget->allocation.height;
		int width=widget->allocation.width;

		int pw=gdk_pixbuf_get_width(pv->pb_scaled);
		int ph=gdk_pixbuf_get_height(pv->pb_scaled);

		gdk_draw_pixbuf(widget->window,NULL,pv->pb_scaled,
			0,0,
			(width-pw)/2,(height-ph)/2,
			pw,ph,
			GDK_RGB_DITHER_NONE,0,0);		
	}
}


static void draw_panned(GtkWidget *widget,GdkEventExpose *event)
{
	PixbufView *pv=PIXBUFVIEW(widget);

	int height=widget->allocation.height;
	int width=widget->allocation.width;

	gdk_draw_rectangle (widget->window,
		widget->style->bg_gc[widget->state],TRUE,
		0,0,width,height);

	GdkPixbuf *pb=pixbufview_get_pixbuf(pv,pv->currentpage);
	if(pb)
	{
		int height=widget->allocation.height;
		int width=widget->allocation.width;

		int pw=gdk_pixbuf_get_width(pb);
		int ph=gdk_pixbuf_get_height(pb);

		int xo=0;
		int yo=0;
		int xt=0;
		int yt=0;
		if(pw<width)
			xt=(width-pw)/2;
		else
		{
			if(pv->xoffset<0)
				pv->xoffset=0;
			if((pv->xoffset+width)>pw)
				pv->xoffset=pw-width;
			xo=pv->xoffset;
		}

		if(ph<height)
			yt=(height-ph)/2;
		else
		{
			if(pv->yoffset<0)
				pv->yoffset=0;
			if((pv->yoffset+height)>ph)
				pv->yoffset=ph-height;
			yo=pv->yoffset;
		}

		gdk_draw_pixbuf(widget->window,NULL,pb,
			xo,yo,
			xt,yt,
			pw-xo,ph-yo,
			GDK_RGB_DITHER_NONE,0,0);		
	}
}


static gboolean
pixbufview_expose( GtkWidget      *widget,
		 GdkEventExpose *event )
{
	PixbufView *pv=PIXBUFVIEW(widget);

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (IS_PIXBUFVIEW (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	if (event->count > 0)
		return FALSE;

	if(pv->scaletofit)
		draw_scaled(widget,event);
	else
		draw_panned(widget,event);

	return FALSE;
}


static gboolean
pixbufview_button_press( GtkWidget      *widget,
		       GdkEventButton *event )
{
	PixbufView *pv;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (PIXBUFVIEW(widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	pv = PIXBUFVIEW(widget);

	switch(event->button)
	{
		case 1:
			pv->dragging=true;
			int x,y;
			GdkModifierType mods;
			gdk_window_get_pointer (widget->window, &x, &y, &mods);
			pv->prev_x=x;
			pv->prev_y=y;
			gtk_grab_add(widget);
			break;
		case 3:
			break;
		default:
			break;	
	}
	return FALSE;
}


static gboolean
pixbufview_button_release( GtkWidget      *widget,
                         GdkEventButton *event )
{
	PixbufView *pageview;
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (IS_PIXBUFVIEW (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	pageview = PIXBUFVIEW(widget);

	switch(event->button)
	{
		case 1:
			pageview->dragging=false;
			gtk_grab_remove(widget);
			break;
		case 3:
			pageview->scaletofit=!pageview->scaletofit;

			{
				GdkPixbuf *pb=pixbufview_get_pixbuf(pageview,pageview->currentpage);
				if(pb)
				{
					int height=widget->allocation.height;
					int width=widget->allocation.width;

					int pw=gdk_pixbuf_get_width(pb);
					int ph=gdk_pixbuf_get_height(pb);

					int x,y;
					GdkModifierType mods;
					gdk_window_get_pointer (widget->window, &x, &y, &mods);

					// Work out which pixel of the image was clicked on.

					int nw,nh;
					nh=height;
					nw=(pw*nh)/ph;
					if(nw>width)
					{
						nw=width;
						nh=(ph*nw)/pw;
					}
					if(pw>width)
					{
						int cx=(pw*(x-(width-nw)/2))/nw;
						pageview->xoffset=cx-width/2;
					}
					if(ph>height)
					{
						int cy=(ph*(y-(height-nh)/2))/nh;
						pageview->yoffset=cy-height/2;
					}
					gtk_widget_queue_draw (GTK_WIDGET (pageview));
				}
			}
			break;
		default:
			break;
	}
	g_signal_emit_by_name (GTK_OBJECT (pageview), "changed");
	return FALSE;
}


static gboolean
pixbufview_motion_notify( GtkWidget      *widget,
                        GdkEventMotion *event )
{
	PixbufView *pageview;
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (IS_PIXBUFVIEW(widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	pageview = PIXBUFVIEW(widget);

	if (pageview->dragging && !pageview->scaletofit)
    {
		int x,y;
		GdkModifierType mods;
		gdk_window_get_pointer (widget->window, &x, &y, &mods);
			
		int dx=(x-pageview->prev_x);
		int dy=(y-pageview->prev_y);

		pageview->prev_x=x;
		pageview->prev_y=y;

		pageview->xoffset-=dx;
		if(pageview->xoffset<0)
			pageview->xoffset=0;
		pageview->yoffset-=dy;
		if(pageview->yoffset<0)
			pageview->yoffset=0;

		gtk_widget_queue_draw (GTK_WIDGET (pageview));
		g_signal_emit_by_name (GTK_OBJECT (pageview), "changed");
		g_signal_emit_by_name (GTK_OBJECT (pageview), "mousemove");
	}
	else
	{
		int x,y;
		GdkModifierType mods;
		gdk_window_get_pointer (widget->window, &x, &y, &mods);

		GdkPixbuf *pb=pixbufview_get_pixbuf(pageview,pageview->currentpage);
		if(pb)
		{
			int pw=gdk_pixbuf_get_width(pb);
			int ph=gdk_pixbuf_get_height(pb);

			int height=widget->allocation.height;
			int width=widget->allocation.width;

			if(pageview->scaletofit)
			{
				// Calculate scaled size...
				int nw,nh;
				nh=height;
				nw=(pw*nh)/ph;
				if(nw>width)
				{
					nw=width;
					nh=(ph*nw)/pw;
				}
				x-=(width-nw)/2;
				y-=(height-nh)/2;
				pageview->mousex=(x*pw)/nw;
				pageview->mousey=(y*ph)/nh;
			}
			else
			{
				if(pw<width)
					x-=(width-pw)/2;
				if(ph<height)
					y-=(height-ph)/2;
				pageview->mousex=x+pageview->xoffset;
				pageview->mousey=y+pageview->yoffset;
			}
			if(pageview->mousex<0)
				pageview->mousex=0;
			if(pageview->mousey<0)
				pageview->mousey=0;
			if(pageview->mousex>=pw)
				pageview->mousex=pw-1;
			if(pageview->mousey>=ph)
				pageview->mousey=ph-1;
			g_signal_emit_by_name (GTK_OBJECT (pageview), "mousemove");
		}
	}
	return FALSE;
}

static void *parent_class=NULL;

static void pixbufview_destroy(GtkObject *object)
{
	if(object && IS_PIXBUFVIEW(object))
	{
		if (GTK_OBJECT_CLASS (parent_class)->destroy)
			(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
	// FIXME - cleanup?  Memory leak here?
		pixbufview_clear_pages(PIXBUFVIEW(object));
	}
}


static void
pixbufview_init (PixbufView *pv)
{
	parent_class = gtk_type_class (gtk_widget_get_type ());
	pv->pb_scaled=NULL;
	pv->resized=false;
	pv->xoffset=0;
	pv->yoffset=0;
	new (&pv->pages) std::deque<GdkPixbuf *>;
}


void pixbufview_refresh(PixbufView *pv)
{
	pv->resized=true;
	gtk_widget_queue_draw (GTK_WIDGET (pv));
}


void pixbufview_set_pixbuf(PixbufView *pv,GdkPixbuf *pb,unsigned int page)
{
//	pixbufview_clear_pages(pv);
//	pixbufview_add_page(pv,pb);

	Debug[TRACE] << "Setting image on page " << page << "  -  list has " << pv->pages.size() << " entries" << endl;

	// If we're trying to set the image of a page that doesn't exist
	// we need to create pages until it does!
	int newpages=(page+1); newpages-=pv->pages.size();

	for(int i=0;i<newpages;++i)
		pixbufview_add_page(pv,NULL);

	bool refresh=false;
	if(page==pv->currentpage) // Is this the current page?
	{
		if(pv->pb_scaled)
			g_object_unref(G_OBJECT(pv->pb_scaled));
		pv->pb_scaled=NULL;

		refresh=true;
	}
	if(pv->pages[page])
		g_object_unref(G_OBJECT(pv->pages[page]));

	pv->pages[page]=pb;
	if(pb)
		g_object_ref(G_OBJECT(pb));

	if(refresh)
		pixbufview_refresh(pv);
}


int pixbufview_get_xoffset(PixbufView *pv)
{
	if(pv)
		return(pv->xoffset);
	else
		return(0);
}


int pixbufview_get_yoffset(PixbufView *pv)
{
	if(pv)
		return(pv->yoffset);
	else
		return(0);
}


bool pixbufview_get_scale(PixbufView *pv)
{
	if(pv)
		return(pv->scaletofit);
	else
		return(false);
}


void pixbufview_set_offset(PixbufView *pv,int xoff,int yoff)
{
	if(pv)
	{
		pv->xoffset=xoff;
		pv->yoffset=yoff;
	}
}


void pixbufview_set_scale(PixbufView *pv,bool scaletofit)
{
	if(pv)
	{
		pv->scaletofit=scaletofit;
	}
}


void pixbufview_add_page(PixbufView *pv,GdkPixbuf *pb)
{
	Debug[TRACE] << "Adding page with pixbuf " << (long)pb << endl;
	pv->pages.push_back(pb);
	if(pb)
	{
		g_object_ref(G_OBJECT(pb));
		pixbufview_set_page(pv,pv->pages.size()-1);
	}
}


void pixbufview_set_page(PixbufView *pv,unsigned int page)
{
	Debug[TRACE] << "Setting pview page to " << page << endl;
	if(page<pv->pages.size())
	{
		if(pv->pb_scaled)
			g_object_unref(G_OBJECT(pv->pb_scaled));
		pv->pb_scaled=NULL;

		pv->currentpage=page;

		Debug[TRACE] << "Refreshing..." << endl;
		pixbufview_refresh(pv);
	}
}


void pixbufview_clear_pages(PixbufView *pv)
{
	if(pv->pb_scaled)
		g_object_unref(G_OBJECT(pv->pb_scaled));
	pv->pb_scaled=NULL;
	while(pv->pages.size())
	{
		if(pv->pages[0])
			g_object_unref(G_OBJECT(pv->pages[0]));
		pv->pages.pop_front();
	}
}


GdkPixbuf *pixbufview_get_pixbuf(PixbufView *pv,unsigned int page)
{
	GdkPixbuf *result=NULL;
	if(page<pv->pages.size())
		result=pv->pages[page];
	return(result);
}


int pixbufview_get_mousex(PixbufView *pv)
{
	if(pv)
		return(pv->mousex);
	return(0);
}


int pixbufview_get_mousey(PixbufView *pv)
{
	if(pv)
		return(pv->mousey);
	return(0);
}
#endif

