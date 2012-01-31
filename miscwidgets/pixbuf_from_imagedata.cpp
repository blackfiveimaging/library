#include "config.h"

#ifdef HAVE_GTK
#include <gdk-pixbuf/gdk-pixdata.h>

#include "pixbuf_from_imagedata.h"

GdkPixbuf *PixbufFromImageData(const guint8 *data,size_t len)
{
	GdkPixdata pd;
	GdkPixbuf *result;
	GError *err;

	if(!gdk_pixdata_deserialize(&pd,len,data,&err))
		throw(err->message);

	if(!(result=gdk_pixbuf_from_pixdata(&pd,false,&err)))
		throw(err->message);

	return(result);
}
#endif

