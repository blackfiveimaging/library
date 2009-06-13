// imagesource_histogram.h
// Copyright (c) 2009 by Alastair M. Robinson
//
// This imagesource performs a straight pass-through of the image data
// but counts pixel values as it goes, creating a histogram.
// The histogram itself is owned by the client application and is supplied
// by reference, so much remain valid for the lifetime of the
// ImageSource_Histogram.
//

#ifndef IMAGESOURCE_HISTOGRAM_H
#define IMAGESOURCE_HISTOGRAM_H

#define IS_HISTOGRAM_BUCKETS 256

#include "imagesource.h"

class ISHistogram_Channel
{
	public:
	ISHistogram_Channel() : channeldata(NULL)
	{
		channeldata=new int[IS_HISTOGRAM_BUCKETS];
	}
	~ISHistogram_Channel()
	{
		if(channeldata)
			delete[] channeldata;
	}
	int &operator[](int samp)
	{
		if(samp>=IS_HISTOGRAM_BUCKETS || samp<0)
			throw "ISHistogram_Channel: Sample out of range";
//		std::cerr << "bucket " << samp << " - current value " << channeldata[samp] << std::endl;
		return(channeldata[samp]);
	}
	void Clear()
	{
		for(int i=0;i<IS_HISTOGRAM_BUCKETS;++i)
			channeldata[i]=0;
	}
	protected:
	int *channeldata;
};


class ISHistogram
{
	public:
	ISHistogram(int channelcount) : channels(NULL), channelcount(channelcount)
	{
		channels=new ISHistogram_Channel[channelcount];
		Clear();
	}
	~ISHistogram()
	{
		if(channels)
			delete[] channels;
	}
	ISHistogram_Channel &operator[](int chan)
	{
		if(chan>=channelcount || chan<0)
			throw "ISHistogram: Channel out of range";
		return(channels[chan]);
	}
	void Clear()
	{
		for(int i=0;i<channelcount;++i)
			channels[i].Clear();
		samplecount=0;
	}
	void Record(ISDataType *data,int count=1)
	{
		for(int x=0;x<count;++x)
		{
			for(int s=0;s<channelcount;++s)
			{
				int d=*data++;
				d=(d*IS_HISTOGRAM_BUCKETS)/(IS_SAMPLEMAX+1);
				channels[s][d]+=1;
			}
		}
		samplecount+=count;
	}
	protected:
	ISHistogram_Channel *channels;
	int channelcount;
	int samplecount;
};


class ImageSource_Histogram : public ImageSource
{
	public:
	ImageSource_Histogram(ImageSource *source,ISHistogram &histogram);
	~ImageSource_Histogram();
	ISDataType *GetRow(int row);
	protected:
	ImageSource *source;
	ISHistogram &histogram;
};

#endif

