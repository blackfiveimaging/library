#include <iostream>
#include <string>

#include <gtk/gtk.h>

#include "debug.h"
#include "util.h"

#include "imagesource.h"
#include "pixbuf_from_imagesource.h"
#include "imagesource_util.h"
#include "cachedimage.h"
#include "tiffsave.h"

#include "pixbufview.h"
#include "generaldialogs.h"
#include "progressbar.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)


using namespace std;


struct PixTally
{
	PixTally()
	{
		for(int i=0;i<8;++i)
			tally[i]=0;
	}
	int &operator[](int i)
	{
		return(tally[i]);
	}
	int tally[8];
};


class ImageSource_HTally : public ImageSource
{
	public:
	ImageSource_HTally(ImageSource *src) : ImageSource(src), src(src), tally(NULL)
	{
		randomaccess=false;
		tally=new PixTally[src->height];
		currentrow=-1;
	}
	~ImageSource_HTally()
	{
		if(src)
			delete(src);
		if(tally)
			delete[] tally;
	}
	ISDataType *GetRow(int row)
	{
		ISDataType *srcdata=src->GetRow(row);
		if(row!=currentrow)
		{
			PixTally tallytmp;
			for(int x=0; x<width; ++x)
			{
				for(int s=0; s<samplesperpixel; ++s)
				{
					tallytmp[s]+=ISTOEIGHT(srcdata[x*samplesperpixel+s]);
				}
			}
			tally[row]=tallytmp;
		}
		return(srcdata);
	}
	int Min(int channel)
	{
		int min=src->height*src->width*255;
		for(int y=0;y<src->height;++y)
		{
			if(tally[y][channel]<min)
				min=tally[y][channel];
		}
		return(min);
	}
	int Max(int channel)
	{
		int max=0;
		for(int y=0;y<src->height;++y)
		{
			if(tally[y][channel]>max)
				max=tally[y][channel];
		}
		return(max);
	}
	PixTally &operator[](int i)
	{
		if(i<0) i=0;
		if(i>=src->height)
			i=src->height-1;
		return(tally[i]);
	}
	protected:
	ImageSource *src;
	PixTally *tally;
};


class ImageSource_HMakeWeight : public ImageSource
{
	public:
	ImageSource_HMakeWeight(ImageSource *src,int windowsize=300) : ImageSource(src), source(NULL), img(src), windowsize(windowsize)
	{
		randomaccess=false;

		ImageSource_HTally t(img.GetImageSource());
		for(int y=0;y<src->height;++y)
			t.GetRow(y);

		makeweights=new PixTally[t.height];

		PixTally runningavg;
		for(int y=0;y<windowsize;++y)
		{
			for(int s=0;s<t.samplesperpixel;++s)
				runningavg[s]=windowsize*t[y-windowsize-2][s];
		}

		int maxdiff=0;

		for(int y=0;y<src->height;++y)
		{
//			cout << y << ": ";
			for(int s=0;s<t.samplesperpixel;++s)
			{
				runningavg[s]-=t[y-windowsize/2][s];
				runningavg[s]+=t[y+windowsize/2][s];
				if(runningavg[s]/windowsize<t[y][s])
					runningavg[s]=windowsize*t[y][s];
//				cout << t[y][s] << ", " << runningavg[s]/windowsize;
				int diff=runningavg[s]/windowsize-t[y][s];
				makeweights[y][s]=diff;
				if(diff>maxdiff)
					maxdiff=diff;
			}
//			cout << endl;
		}

		Debug[TRACE] << "  Max diff: " << maxdiff << ", requiring " << maxdiff/255 << " pixels" << endl;
		width+=maxdiff/255;

		source=img.GetImageSource();
		MakeRowBuffer();
		currentrow=-1;
	}
	~ImageSource_HMakeWeight()
	{
		if(source)
			delete source;
	}
	ISDataType *GetRow(int row)
	{
		if(row==currentrow)
			return(rowbuffer);

		ISDataType *srcdata=source->GetRow(row);
		Debug[TRACE] << "Got srcdata " << endl;

		for(int x=0;x<source->width;++x)
		{
			for(int s=0;s<source->samplesperpixel;++s)
			{
				rowbuffer[x*samplesperpixel+s]=srcdata[x*samplesperpixel+s];
			}
		}
		Debug[TRACE] << "Copied source data " << endl;
		int xw=width-source->width;
		for(int x=source->width;x<width;++x)
		{
			for(int s=0;s<source->samplesperpixel;++s)
			{
				rowbuffer[x*samplesperpixel+s]=EIGHTTOIS(makeweights[row][s]/xw);
			}
		}
		currentrow=row;
		return(rowbuffer);
	}

	protected:
	ImageSource *source;
	CachedImage img;
	int windowsize;
	PixTally *makeweights;
};


class DeTrackUI
{
	public:
	DeTrackUI() : img(NULL), smoothingwindowsize(150)
	{
		window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(window),800,600);
		gtk_window_set_title (GTK_WINDOW (window),_("PixBufView Test"));
		gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			(GtkSignalFunc) gtk_main_quit, NULL);

		GtkWidget *vbox=gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(window),vbox);
		gtk_widget_show(GTK_WIDGET(vbox));

		GtkWidget *hbox=gtk_hbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
		gtk_widget_show(hbox);

		GtkWidget *tmp;
		tmp=gtk_label_new(_("Smoothing:"));
		gtk_box_pack_start(GTK_BOX(hbox),tmp,FALSE,FALSE,8);
		gtk_widget_show(tmp);

		smoothing=gtk_spin_button_new_with_range(50,1000,25);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(smoothing),smoothingwindowsize);
		gtk_box_pack_start(GTK_BOX(hbox),smoothing,FALSE,FALSE,8);
		g_signal_connect(G_OBJECT(smoothing),"value-changed",G_CALLBACK(smoothing_changed),this);
		gtk_widget_show(smoothing);

		tmp=gtk_hbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(hbox),tmp,TRUE,TRUE,0);

		tmp=gtk_button_new_with_label(_("Save..."));
		gtk_box_pack_start(GTK_BOX(hbox),tmp,FALSE,FALSE,8);
		g_signal_connect(G_OBJECT(tmp),"clicked",G_CALLBACK(save_clicked),this);
		gtk_widget_show(tmp);

		pview=pixbufview_new(NULL,true);

		gtk_box_pack_start(GTK_BOX(vbox),pview,TRUE,TRUE,0);
		gtk_widget_show(pview);
		gtk_widget_show(window);

	}
	~DeTrackUI()
	{
		if(img)
			delete img;
	}
	static void smoothing_changed(GtkWidget *wid,gpointer ud)
	{
		DeTrackUI *ui=(DeTrackUI *)ud;
		ui->smoothingwindowsize=gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui->smoothing));
		ui->Update();
	}
	static void save_clicked(GtkWidget *wid,gpointer ud)
	{
		DeTrackUI *ui=(DeTrackUI *)ud;
		ui->Save();
	}
	void SetFile(const char *fn)
	{
		char *tmp=BuildFilename(fn,"-detracked","tif");
		filename=tmp;
		free(tmp);
		ImageSource *is=ISLoadImage(fn);
		img=new CachedImage(is);
		Update();
	}
	void Update()
	{
		if(img)
		{
			ImageSource *is=img->GetImageSource();
			is=new ImageSource_HMakeWeight(is,smoothingwindowsize);
			GdkPixbuf *pb=pixbuf_from_imagesource(is);
			delete is;

			pixbufview_set_pixbuf(PIXBUFVIEW(pview),pb);
			g_object_unref(G_OBJECT(pb));
		}
	}
	void Save()
	{
		if(img)
		{
			char *fn=File_Save_Dialog(_("Please choose a filename for saving..."),filename.c_str(),window);
			if(fn)
			{
				filename=fn;

				ImageSource *is=img->GetImageSource();
				is=new ImageSource_HMakeWeight(is,smoothingwindowsize);
				TIFFSaver ts(fn,is);
				ts.Save();
				delete is;
				free(fn);
			}
		}
	}
	protected:
	std::string filename;
	CachedImage *img;
	GtkWidget *window;
	GtkWidget *pview;
	GtkWidget *smoothing;
	int smoothingwindowsize;
};



int main(int argc, char **argv)
{
	Debug.SetLevel(TRACE);

	gtk_init(&argc,&argv);

	try
	{
		if(argc>1)
		{
			DeTrackUI ui;
			ui.SetFile(argv[1]);
			
			gtk_main();
		}
	}
	catch(const char *err)
	{
		Debug[ERROR] << err << endl;
	}
	return(0);
}

