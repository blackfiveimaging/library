#include <iostream>
#include <cstring>

#include "imagesource/imagesource.h"

#include <gutenprint/gutenprint.h>
#include <gtk/gtk.h>

#include "stpui_widgets/stpui_optionbook.h"

#include "gp_cppsupport/gprinter.h"
#include "support/md5.h"

using namespace std;

class ImageSource_RainbowSweep : public ImageSource
{
	public:
	ImageSource_RainbowSweep(int width,int height)
		: ImageSource(width,height,IS_TYPE_RGB)
	{
		randomaccess=true;
		MakeRowBuffer();
	}
	~ImageSource_RainbowSweep()
	{
	}

	ISDataType *GetRow(int row)
	{
		if(currentrow==row)
			return(rowbuffer);
		double a=row;
		a/=height;

		for(int x=0;x<width;++x)
		{
			int c=(x*1536)/width;
			int r=c;
			if(r>768) r-=1536;
			if(r<0) r=-r;
			r=512-r;
			if(r<0) r=0;
			if(r>255) r=255;

			int g=c-512;
			if(g<0) g=-g;
			g=512-g;
			if(g<0) g=0;
			if(g>255) g=255;

			int b=c-1024;
			if(b<0) b=-b;
			b=512-b;
			if(b<0) b=0;
			if(b>255) b=255;

			r=128*a+r*(1-a);
			g=128*a+g*(1-a);
			b=128*a+b*(1-a);

			rowbuffer[x*3]=EIGHTTOIS(r);
			rowbuffer[x*3+1]=EIGHTTOIS(g);
			rowbuffer[x*3+2]=EIGHTTOIS(b);
		}
		
		currentrow=row;
		return(rowbuffer);
	}
};


class MD5Consumer : public Consumer, public MD5Digest
{
	public:
	MD5Consumer() : Consumer(), MD5Digest()
	{
	}
	virtual ~MD5Consumer()
	{
	}
	virtual bool Write(const char *buffer,int length)
	{
		Update(buffer,length);

		// There's an issue with the PostScript drive in that it includes the creation date
		// so the MD5 hash will be different every time.  To get around this we reset the MD5 if we
		// encounter a CreationDate comment.
		if(length>14 && strncmp(buffer,"%%CreationDate",14)==0)
			InitMD5Context();
		return(true);
	}
	virtual void Cancel()
	{
	}
};


int main(int argc, char **argv)
{
	gtk_init(&argc,&argv);
	stp_init();
	ConfigFile config;
	PrintOutput printoutput(&config,"[Output]");
	GPrinter printer(printoutput,&config,"[Print]");

	if(argc>1)
		config.ParseConfigFile(argv[1]);

	GtkWidget *dialog=gtk_dialog_new_with_buttons("Printer Setup",
		NULL,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);
	gtk_window_set_default_size(GTK_WINDOW(dialog),500,350);

	GtkWidget *optionbook=stpui_optionbook_new(printer.stpvars,NULL,0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),optionbook,TRUE,TRUE,0);
	gtk_widget_show(optionbook);

	gtk_widget_show(dialog);
	gint result=gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_OK:
			break;
	}
	gtk_widget_destroy(dialog);

	ImageSource *is=new ImageSource_RainbowSweep(360,200);
	is->SetResolution(180,180);

	MD5Consumer cons;

	printer.Print(is,72,72,&cons);

	cerr << "MD5 hash is: " << cons << endl;
 
	delete is;

	return(0);
}

