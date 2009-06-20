#include <iostream>

#include "imagesource_histogram.h"

using namespace std;

ImageSource_Histogram::ImageSource_Histogram(ImageSource *source,ISHistogram &histogram)
	: ImageSource(source), source(source), histogram(histogram)
{
	currentrow=-1;
	histogram.SetHistogramType(source->samplesperpixel,source->type);
}


ImageSource_Histogram::~ImageSource_Histogram()
{
	if(source)
		delete(source);
}


ISDataType *ImageSource_Histogram::GetRow(int row)
{
	ISDataType *srcdata=source->GetRow(row);
	if(row!=currentrow)
	{
		histogram.Record(srcdata,source->width);
		// Build histogram
	}
	currentrow=row;
	return(srcdata);
}


