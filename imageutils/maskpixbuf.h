#ifndef MASKPIXBUF_H
#define MASKPIXBUF_H

#ifdef HAVE_GTK
#include <gdk/gdkpixbuf.h>

void maskpixbuf(GdkPixbuf *img,int xpos,int ypos,int width,int height,const GdkPixbuf *mask,
	int redbg8=255,int greenbg8=255,int bluebg8=255);
#endif // HAVE_GTK
#endif
