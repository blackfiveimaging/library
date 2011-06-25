#include <gtk/gtk.h>

#include "spinner.h"

#include "pixbuf_from_imagedata.h"
#include "spinner1.cpp"
#include "spinner2.cpp"
#include "spinner3.cpp"
#include "spinner4.cpp"
#include "spinner5.cpp"
#include "spinner6.cpp"
#include "spinner7.cpp"
#include "spinner8.cpp"

Spinner::Spinner()
{
	frames[0]=PixbufFromImageData(spinner1_data,sizeof(spinner1_data));
	frames[1]=PixbufFromImageData(spinner2_data,sizeof(spinner2_data));
	frames[2]=PixbufFromImageData(spinner3_data,sizeof(spinner3_data));
	frames[3]=PixbufFromImageData(spinner4_data,sizeof(spinner4_data));
	frames[4]=PixbufFromImageData(spinner5_data,sizeof(spinner5_data));
	frames[5]=PixbufFromImageData(spinner6_data,sizeof(spinner6_data));
	frames[6]=PixbufFromImageData(spinner7_data,sizeof(spinner7_data));
	frames[7]=PixbufFromImageData(spinner8_data,sizeof(spinner8_data));
	spinner=gtk_image_new();
	g_object_ref(G_OBJECT(spinner));
	gtk_widget_show(spinner);
	SetFrame(0);
}


Spinner::~Spinner()
{
	for(int i=0;i<8;++i)
	{
		if(frames[i])
			g_object_unref(frames[i]);
	}
	if(spinner)
		gtk_widget_destroy(spinner);
	g_object_unref(G_OBJECT(spinner));
}


void Spinner::SetFrame(int f)
{
	f=f&7;	// clamp to range 0-7
	gtk_image_set_from_pixbuf(GTK_IMAGE(spinner),frames[f]);
}


GtkWidget *Spinner::GetWidget()
{
	return(spinner);
}

