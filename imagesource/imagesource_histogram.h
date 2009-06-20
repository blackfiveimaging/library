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


// An individual channel's histogram.  Access the data using the [] operator.
// A reference is returned, so the result can be modified as well as read.

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
	// Three constructors are provided - one takes the channel count and type directly,
	// one takes an existing imagesource and takes the channel count and type from that,
	// and the last takes no arguments, but requires the caller to call
	// SetHistogramType with channels and type before use.
	ISHistogram(int channelcount,IS_TYPE type=IS_TYPE_RGB) : channels(NULL), channelcount(channelcount),type(type)
	{
		SetHistogramType(channelcount,type);
	}
	ISHistogram(ImageSource *is) : channels(NULL)
	{
		SetHistogramType(is->samplesperpixel,is->type);
	}
	ISHistogram() : channels(NULL), channelcount(0), type(IS_TYPE_NULL)
	{
	}
	~ISHistogram()
	{
		if(channels)
			delete[] channels;
	}

	// Use this to set or change the number of channels and image type.
	// The type will be used by the histogram display widget to choose
	// which colorants to use when drawing the histogram.
	void SetHistogramType(int channelcount,IS_TYPE type=IS_TYPE_RGB)
	{
		this->channelcount=channelcount;
		this->type=type;
		if(channels)
			delete[] channels;
		channels=new ISHistogram_Channel[channelcount];
		Clear();
	}

	// Access the histogram channels through the [] operator.
	// The result is returned as a reference so you can follow it with
	// another [] to access a specific bucket within a channel.
	ISHistogram_Channel &operator[](int chan)
	{
		if(!channels)
			throw "ISHistogram: Must set the histogram type before use!";
		if(chan>=channelcount || chan<0)
			throw "ISHistogram: Channel out of range";
		return(channels[chan]);
	}
	void Clear()
	{
		if(!channels)
			throw "ISHistogram: Must set the histogram type before use!";
		for(int i=0;i<channelcount;++i)
			channels[i].Clear();
		samplecount=0;
	}
	void Record(ISDataType *data,int count=1)
	{
		if(!channels)
			throw "ISHistogram: Must set the histogram type before use!";
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
	IS_TYPE GetType()
	{
		return(type);
	}
	int GetChannelCount()
	{
		return(channelcount);
	}
	int GetSampleCount()
	{
		return(samplecount);
	}
	int GetMax()
	{
		int max=0;
		for(int c=0;c<channelcount;++c)
		{
			for(int b=0;b<IS_HISTOGRAM_BUCKETS;++b)
			{
				int t=channels[c][b];
				if(t>max)
					max=t;
			}
		}
		return(max);
	}
	protected:
	ISHistogram_Channel *channels;
	int channelcount;
	int samplecount;
	IS_TYPE type;
};


// Wedge this into a chain of imagesources - it will tally the pixels as they're processed, but
// pass them though unmodified.

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

