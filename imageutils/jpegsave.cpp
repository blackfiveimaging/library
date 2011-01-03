/*
 * jpegsave.cpp - class for saving an ImageSource to disk as a JPEG file.
 *
 * Copyright (c) 2007 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 *
 */


#include <iostream>

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <sys/stat.h>

#include "debug.h"
#include "binaryblob.h"
#include "lcmswrapper.h"
#include "imagesource_flatten.h"
#include "util.h"

#include "iccjpeg.h"
#include "jpegsave.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)

using namespace std;


struct JPEGSaver_ErrManager
{
	struct jpeg_error_mgr std;
	FILE *file;
};


static void isjpeg_error_exit (j_common_ptr cinfo)
{
	char buffer[JMSG_LENGTH_MAX];
	JPEGSaver_ErrManager *myerr = (JPEGSaver_ErrManager *) cinfo->err;
	cinfo->err->output_message(cinfo);
	cinfo->err->format_message(cinfo,buffer);
	cerr << buffer << endl;
	if(myerr->file && myerr->file!=stdout)
		fclose(myerr->file);
	exit(1);
}


JPEGSaver::~JPEGSaver()
{
	if(err)
	{
		if(err->file && err->file!=stdout)
			fclose(err->file);
		delete err;
	}
	if(cinfo)
	{
		jpeg_destroy_compress((jpeg_compress_struct *)cinfo);
		delete cinfo;
	}
	
	if(tmpbuffer)
		free(tmpbuffer);
}


void JPEGSaver::Save()
{	
	if(err->file)
	{
		int i;
		unsigned char *dst;
		ISDataType *src;

		for(int row=0;row<height;++row)
		{
			if(progress && !(row&31))
			{
				if(!progress->DoProgress(row,height))
					return;
			}

			dst=tmpbuffer;
				
			src=imagesource->GetRow(row);

			switch(imagesource->type)
			{
				case IS_TYPE_RGB:
					for(i=0;i<bytesperrow;++i)
					{
						unsigned int t;
						t=int(src[i]);
						t=ISTOEIGHT(t);
						if(t>255) t=255;
						if(t<0) t=0;
						dst[i]=t;
					}
					break;
				default:
					for(i=0;i<bytesperrow;++i)
					{
						unsigned int t;
						t=IS_SAMPLEMAX-src[i];
						t=ISTOEIGHT(t);
						if(t>255) t=255;
						if(t<0) t=0;
						dst[i]=t;
					}
					break;
			}
			JSAMPROW rowptr[1]={tmpbuffer};
			jpeg_write_scanlines(cinfo, rowptr, 1);
		}
		if(progress)
			progress->DoProgress(height,height);
		jpeg_finish_compress(cinfo);
		jpeg_destroy_compress(cinfo);
		delete cinfo;
		cinfo=NULL;
	}
}


void JPEGSaver::EmbedProfile(RefCountPtr<CMSProfile> profile)
{
	if(!profile)
		return;
	if(!cinfo)
		throw "JPEGSaver: cinfo already freed!";
	BinaryBlob *blob=profile->GetBlob();
	size_t EmbedLen=blob->GetSize();
	JOCTET *EmbedBuffer=(JOCTET *)blob->GetPointer();
	
	write_icc_profile(cinfo,EmbedBuffer,EmbedLen);
	delete blob;
}


JPEGSaver::JPEGSaver(const char *filename,struct ImageSource *is,int compression)
	: ImageSaver(), imagesource(is), cinfo(NULL), tmpbuffer(NULL)
{
	if(STRIP_ALPHA(is->type)==IS_TYPE_BW)
		throw _("JPEG Saver only supports greyscale and colour images!");

//	if(STRIP_ALPHA(is->type)==IS_TYPE_CMYK)
//		throw _("Saving CMYK JPEGs not (yet) supported");

	if(HAS_ALPHA(is->type))
		is=new ImageSource_Flatten(is);

	this->width=is->width;
	this->height=is->height;

	cinfo=new jpeg_compress_struct;
	err=new JPEGSaver_ErrManager;
	memset(cinfo,0,sizeof(jpeg_compress_struct));
	memset(err,0,sizeof(JPEGSaver_ErrManager));
	cinfo->err = jpeg_std_error(&err->std);
	err->std.error_exit = isjpeg_error_exit;

	if(filename)
	{
		if((err->file = FOpenUTF8(filename,"wb")) == NULL)
			throw _("Can't open file for saving");
		Debug[TRACE] << "File " << filename << " opened" << endl;
	}
	else
		err->file=stdout;

	jpeg_create_compress(cinfo);
	jpeg_stdio_dest(cinfo, err->file);

	cinfo->image_width=is->width;
	cinfo->image_height=is->height;

	switch(is->type)
	{
		case IS_TYPE_RGB:
			cinfo->input_components=3;
			cinfo->in_color_space = JCS_RGB;
			jpeg_set_defaults(cinfo);
			break;
		case IS_TYPE_GREY:
			cinfo->input_components=1;
			cinfo->in_color_space = JCS_GRAYSCALE;
			jpeg_set_defaults(cinfo);
			break;
		case IS_TYPE_CMYK:
			cinfo->input_components=4;
			cinfo->in_color_space = JCS_CMYK;
			cinfo->jpeg_color_space = JCS_YCCK;
			jpeg_set_defaults(cinfo);
		    jpeg_set_colorspace(cinfo, JCS_YCCK);
			cinfo->write_JFIF_header=TRUE;	// HACK to force resolution to be saved.
											// Not strictly correct, since JFIF files can't be CMYK.
											// LCMS's jpegicc does this too (though probably not deliberately!)
			break;
		default:
			throw _("JPEG Saver can currently only save RGB, CMYK or Greyscale images.");
			break;
	}

	jpeg_set_quality(cinfo,compression,TRUE);

	cinfo->density_unit=1;
	cinfo->X_density=is->xres;
	cinfo->Y_density=is->yres;

	Debug[TRACE] << "JPEGSaver: Set xres to " << cinfo->X_density << endl;
	Debug[TRACE] << "JPEGSaver: Set yres to " << cinfo->Y_density << endl;

	jpeg_start_compress(cinfo,TRUE);

	if(is->GetEmbeddedProfile())
		EmbedProfile(is->GetEmbeddedProfile());

	bytesperrow = width*is->samplesperpixel;

	if(!(tmpbuffer=(unsigned char *)malloc(bytesperrow)))
		throw "No memory for tmpbuffer";
}
