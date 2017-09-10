#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "cachedimage.h"
#include "imagesource_util.h"
#include "imagesource_greyscale.h"
#include "jpegsaver.h"
#include "tiffsaver.h"
#include "progresstext.h"
#include "profilemanager/profilemanager.h"


class MMMenuSaver : public ImageSaver
{
	public:
	MMMenuSaver(const char *filename,RefCountPtr<ImageSource> is) : ImageSaver(is)
	{
		buffer=new char[is->width*8];
//		buffer2=new char(is->width);
		std::cout << "#ifndef LOGO_H" << std::endl << "#define LOGO_H" << std::endl;
		std::cout << "const unsigned char logodata[" << (is->height+7)/8 << "][" << is->width <<"] = { " << std::endl;
	}
	virtual ~MMMenuSaver()
	{
		std::cout << "};" << std::endl << "#endif" << std::endl;
		if(buffer)
			delete[] buffer;
//		if(buffer2)
//			delete[] buffer2;
	}
	virtual void ProcessRow(int row)
	{
		if((row&7)==0)
		{
			std::cout << "{";
			for(int i=0;i<8;++i)
			{
				if((row+i)>=source->height)
				{
					for(int j=0;j<source->width;++j)
					{
						buffer[i*source->width+j]=0;
					}
				}
				else
				{
					ISDataType *src=source->GetRow(row+i);
					for(int j=0;j<source->width;++j)
					{
						buffer[i*source->width+j]=(src[j] ? 255 : 0);
					}
				}
			}
			for(int i=0;i<source->width;++i)
			{
				char out[]="0x00";
				int a=0;
				for(int j=0;j<8;++j)
				{
					a>>=1;
					if(buffer[j*source->width+i])
						a|=0x80;
				}
				int b=a&15;
				a=(a>>4)&15;

				a+='0';
				if(a>'9') a+=('a'-'9')-1;

				b+='0';
				if(b>'9') b+=('a'-'9')-1;
				
				out[2]=a;
				out[3]=b;

				std::cout << out;
				if(i<source->width-1)
					std::cout << ", ";
				if((i&15)==15)
					std::cout << std::endl;
			}
			std::cout << "}";
			if((source->height-row)>8)
				std::cout << ",";
			std::cout << std::endl;
		}
	}
	private:
	char *buffer;
//	char *buffer2;
};



int main(int argc,char **argv)
{
	Debug.SetLevel(TRACE);
	gtk_init(&argc,&argv);
	try
	{
		if(argc<2)
			return(0);
		ImageSource *is=ISLoadImage(argv[1]);
		if(is->type!=IS_TYPE_GREY)
			is=new ImageSource_Greyscale(is);
		Debug[TRACE] << "Width: " << is->width << std::endl;
		ProgressText prog;
		MMMenuSaver ms(argv[2],ImageSource_rp(is));
		ms.SetProgress(&prog);
		ms.Save();
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}
	return(0);
}

