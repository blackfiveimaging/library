/*
 * imagesource_jpeg.cpp
 * 24-bit RGB and 8-bit Greyscale JPEG scanline-based Loader
 * Doesn't support Random Access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


#include <iostream>

#include <stdio.h>
#include <string.h>
extern "C"
{
#ifdef WIN32
// Ugly hack to be compatible with the libjpeg62 shipped with GIMP.
typedef unsigned char boolean;
#define HAVE_BOOLEAN
#endif
#include <jpeglib.h>
#include <jerror.h>
}

#include "../support/debug.h"

#include "lcmswrapper.h"
#include "util.h"

#include "iccjpeg.h"

#include "imagesource.h"
#include "imagesource_jpeg.h"

using namespace std;

struct ImageSource_JPEG_ErrManager
{
	struct jpeg_error_mgr std;
	FILE *File;
	bool FileOwned;
};

static void isjpeg_error_exit (j_common_ptr cinfo)
{
	char buffer[JMSG_LENGTH_MAX];
	ImageSource_JPEG_ErrManager *myerr = (ImageSource_JPEG_ErrManager *) cinfo->err;
	cinfo->err->output_message(cinfo);
	cinfo->err->format_message(cinfo,buffer);
	Debug[TRACE] << buffer << endl;
	jpeg_destroy_compress((jpeg_compress_struct *)cinfo);
	if(myerr->FileOwned)
		fclose(myerr->File);
	throw "Error reading JPEG file";
}


ImageSource_JPEG::ImageSource_JPEG(const char *filename)
	: ImageSource(), cinfo(NULL), tmprow(NULL), err(NULL), iccprofbuffer(NULL), started(false)
{
	err=new ImageSource_JPEG_ErrManager;
	if ((err->File = FOpenUTF8(filename,"rb")) == NULL)
    	throw "Unable to open file";
	err->FileOwned=true;
	Init();
}


ImageSource_JPEG::ImageSource_JPEG(FILE *file)
	: ImageSource(), cinfo(NULL), tmprow(NULL), err(NULL), iccprofbuffer(NULL), started(false)
{
	err=new ImageSource_JPEG_ErrManager;
	err->File = file;
	err->FileOwned=false;
	Init();
}


void ImageSource_JPEG::Init()
{
	cinfo=new jpeg_decompress_struct;

	/* Initialize the JPEG compression object with default error handling. */
	memset(cinfo,0,sizeof(jpeg_decompress_struct));
	memset(cinfo,0,sizeof(ImageSource_JPEG_ErrManager));

	cinfo->err = jpeg_std_error(&err->std);
	err->std.error_exit = isjpeg_error_exit;

	jpeg_create_decompress(cinfo);
	jpeg_stdio_src(cinfo, err->File);

	setup_read_icc_profile(cinfo);

	jpeg_read_header(cinfo,TRUE);

	width=cinfo->image_width;
	height=cinfo->image_height;

	Debug[TRACE] << "JPEG Loader: Have " << cinfo->num_components << " components" << endl;

	switch(cinfo->num_components)
	{
		case 1:
			type=IS_TYPE_GREY;
			samplesperpixel=1;
			break;
		case 3:
			type=IS_TYPE_RGB;
			samplesperpixel=3;
			break;
		case 4:
			type=IS_TYPE_CMYK;
			samplesperpixel=4;
			break;
		default:
			throw "Only greyscale, RGB and CMYK JPEGs are currently supported";
			break;
	}

	randomaccess=false;
	currentrow=-1;

	switch(cinfo->density_unit)
	{
		case 1:
			xres=cinfo->X_density;
			yres=cinfo->Y_density;
			break;
		case 2:
			xres=int(2.54*cinfo->X_density);
			yres=int(2.54*cinfo->Y_density);
			break;
		default:
			xres=yres=72;
			break;
	}
	
	JOCTET *iccprofile;
	unsigned int profilelen;
	if(read_icc_profile(cinfo,&iccprofile,&profilelen))
	{
		iccprofbuffer=(char *)iccprofile;
		SetEmbeddedProfile(new CMSProfile(iccprofbuffer,profilelen));
	}

	if(!(tmprow=(unsigned char *)malloc(sizeof(char)*(width*samplesperpixel))))
		throw "Can't allocate temp buffer...";

	MakeRowBuffer();
}


ISDataType *ImageSource_JPEG::GetRow(int row)
{
	int x;
	JSAMPROW rowptr[1]={0};

	if(!started)
	{
		jpeg_start_decompress(cinfo);
		started=true;
	}

	if(row==currentrow)
		return(rowbuffer);
		
	if(row<currentrow)
	{
		Debug[TRACE] << "JPEG error - can't support random access.  Row " << row << " requested after row " << currentrow << endl;
		throw "Random access not supported for JPEG files";
	}
	
	rowptr[0]=(JSAMPROW)tmprow;
	for(;currentrow<row;++currentrow)
	{
		jpeg_read_scanlines(cinfo, rowptr, 1);
	}
	
	switch(samplesperpixel)
	{
		case 1:
			for(x=0;x<width;++x)
			{
				int t=tmprow[x];
				rowbuffer[x]=IS_SAMPLEMAX-EIGHTTOIS(t);
			}
			break;
		case 3:
			for(x=0;x<width;++x)
			{
				int t=tmprow[x*3];
				rowbuffer[x*3]=EIGHTTOIS(t);
				t=tmprow[x*3+1];
				rowbuffer[x*3+1]=EIGHTTOIS(t);
				t=tmprow[x*3+2];
				rowbuffer[x*3+2]=EIGHTTOIS(t);
			}
			break;
		case 4:
			for(x=0;x<width*samplesperpixel;++x)
			{
				int t=tmprow[x];
				rowbuffer[x]=IS_SAMPLEMAX-EIGHTTOIS(t);
			}
			break;
	}

	return(rowbuffer);
}


ImageSource_JPEG::~ImageSource_JPEG()
{
	while(currentrow<(height-1))
		GetRow(currentrow+1);

	if(iccprofbuffer)
		free(iccprofbuffer);

	if(tmprow)
		free(tmprow);

	if(cinfo)
	{
		jpeg_finish_decompress(cinfo);
		jpeg_destroy_decompress(cinfo);
		delete cinfo;
	}

	if(err)
	{
		if(err->File && err->FileOwned)
			fclose(err->File);
		delete err;
	}
}
