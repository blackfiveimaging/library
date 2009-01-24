/*
 * imagesource_modifiedgamma.cpp
 * Gamma-corrects an image.
 *
 * Copyright (c) 2009 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <stdlib.h>

#include <math.h>

#include "imagesource_modifiedgamma.h"


using namespace std;

ImageSource_ModifiedGamma::~ImageSource_ModifiedGamma()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_ModifiedGamma::GetRow(int row)
{
	ISDataType *src;

	if(row==currentrow)
		return(rowbuffer);

	src=source->GetRow(row);

	switch(STRIP_ALPHA(type))
	{
		case IS_TYPE_RGB:	// RGB is the only colourspace in which we treat 0 as black.
			if(HAS_ALPHA(type))	// We don't want to adjust the alpha channel!
			{
				for(int i=0;i<width;++i)
				{
					int j;
					for(j=0;j<samplesperpixel-1;++j)
					{
						float x=src[i*samplesperpixel+j];
						x/=IS_SAMPLEMAX;
						float y=0.0;
						if(x<threshold)
							y=x*slope;
						else
							y=pow((x+offset)/(1.0+offset),gamma);
						int t=int(IS_SAMPLEMAX*y);
						if(t<0) t=0;
						if(t>IS_SAMPLEMAX) t=IS_SAMPLEMAX;
						rowbuffer[i*samplesperpixel+j]=t;
					}
				}	
			}
			else
			{
				for(int i=0;i<width;++i)
				{
					for(int j=0;j<samplesperpixel;++j)
					{
						float x=src[i*samplesperpixel+j];
						x/=IS_SAMPLEMAX;
						float y=0.0;
						if(x<threshold)
							y=x*slope;
						else
							y=pow((x+offset)/(1.0+offset),gamma);
						int t=int(IS_SAMPLEMAX*y);
						if(t<0) t=0;
						if(t>IS_SAMPLEMAX) t=IS_SAMPLEMAX;
						rowbuffer[i*samplesperpixel+j]=t;
					}
				}	
			}		
			break;
		default:
			if(HAS_ALPHA(type))	// We don't want to scale the alpha channel!
			{
				for(int i=0;i<width;++i)
				{
					int j;
					for(j=0;j<samplesperpixel-1;++j)
					{
						float x=IS_SAMPLEMAX-src[i*samplesperpixel+j];
						x/=IS_SAMPLEMAX;
						float y=0.0;
						if(x<threshold)
							y=x*slope;
						else
							y=pow((x+offset)/(1.0+offset),gamma);
						int t=IS_SAMPLEMAX-int(IS_SAMPLEMAX*y);
						if(t<0) t=0;
						if(t>IS_SAMPLEMAX) t=IS_SAMPLEMAX;
						rowbuffer[i*samplesperpixel+j]=t;
					}
					rowbuffer[i*samplesperpixel+j]=src[i*samplesperpixel+j];	// Copy alpha unchanged
				}	
			}
			else
			{
				for(int i=0;i<width;++i)
				{
					for(int j=0;j<samplesperpixel;++j)
					{
						float x=IS_SAMPLEMAX-src[i*samplesperpixel+j];
						x/=IS_SAMPLEMAX;
						float y=0.0;
						if(x<threshold)
							y=x*slope;
						else
							y=pow((x+offset)/(1.0+offset),gamma);
						int t=IS_SAMPLEMAX-int(IS_SAMPLEMAX*y);
						if(t<0) t=0;
						if(t>IS_SAMPLEMAX) t=IS_SAMPLEMAX;
						rowbuffer[i*samplesperpixel+j]=t;
					}
				}	
			}		
			break;
	}

	currentrow=row;
	return(rowbuffer);
}


ImageSource_ModifiedGamma::ImageSource_ModifiedGamma(ImageSource *source,float gamma,float offset)
	: ImageSource(source), source(source), gamma(gamma), offset(offset)
{
	cerr << "Modified gamma - using offset of " << offset << endl;
	threshold=offset/(gamma + gamma*offset - 1.0);
	cerr << "Threshold: " << threshold << endl;
	slope=pow((threshold+offset)/(1.0+offset),gamma)/threshold;
	cerr << "Slope of linear section: " << slope << endl;
	MakeRowBuffer();
}


float ImageSource_ModifiedGamma::FindGamma(float x,float y,float offset)
{
	return(log(y)/(log(x+offset)-log(1.0+offset)));
}


float ImageSource_ModifiedGamma::ModifiedGamma(float x,float gamma,float offset)
{
	float threshold=offset/(gamma + gamma*offset - 1.0);
	float slope=pow((threshold+offset)/(1.0+offset),gamma);
	if(x<threshold)
		return(x*slope);
	else
		return(pow((x+offset)/(1.0+offset),gamma));
}


float ImageSource_ModifiedGamma::InverseModifiedGamma(float x, float gamma, float offset)
{
	float threshold=offset/(gamma + gamma*offset - 1.0);
	float slope=pow((threshold+offset)/(1.0+offset),gamma);
	float t2=pow((x+offset)/(1.0+offset),gamma);
	if(x<t2)
		return(x/slope);
	else
		return((1+offset)*pow(x,1/gamma)-offset);
}

