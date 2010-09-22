#ifndef IMAGESOURCE_TYPES_H
#define IMAGESOURCE_TYPES_H

#include "../support/debug.h"

typedef unsigned short ISDataType;
#define IS_SAMPLEMAX 65535
#define EIGHTTOIS(x) (((x) << 8) | (x))
#define ISTOEIGHT(x) (((x) >> 8) & 0xff )
//#define ISTOEIGHT(x) ((((x) * 65281 + 8388608) >> 24) & 0xff)

/* Note: 0 is Black for RGB images, but White for Greyscale and CMYK images */

enum IS_TYPE {
	IS_TYPE_NULL=0,
	IS_TYPE_BW,
	IS_TYPE_GREY,
	IS_TYPE_RGB,
	IS_TYPE_CMYK,
	IS_TYPE_LAB,
	IS_TYPE_HSV,
	IS_TYPE_DEVICEN,
	IS_TYPE_NULLA=16,
	IS_TYPE_BWA,
	IS_TYPE_GREYA,
	IS_TYPE_RGBA,
	IS_TYPE_CMYKA,
	IS_TYPE_LABA,
	IS_TYPE_HSVA,
	IS_TYPE_DEVICENA
};

#define IS_TYPE_ALPHA 8
#define IS_MAX_SAMPLESPERPIXEL 5
#define STRIP_ALPHA(x) IS_TYPE(((x)&~IS_TYPE_ALPHA))
#define HAS_ALPHA(x) ((x)&IS_TYPE_ALPHA)


// DeviceNValue - reallly belongs in a sub-library containing all the specialised
// DeviceN stuff, though potentially useful for RGB or CMYK stuff too.

class ISDeviceNValue
{
	public:
	ISDeviceNValue(int channels,ISDataType value=0) : channels(channels), values(NULL)
	{
		values=new ISDataType[channels];
		for(int i=0;i<channels;++i)
			values[i]=value;
	}
	ISDeviceNValue(const ISDeviceNValue &other) : channels(other.channels), values(NULL)
	{
		values=new ISDataType[channels];
		for(int i=0;i<channels;++i)
			values[i]=other[i];
	}
	~ISDeviceNValue()
	{
		if(values)
			delete[] values;
	}
	ISDataType &operator[](int i) const
	{
		if(i<channels && i>=0)
			return(values[i]);
		else
			throw "DeviceNValue - bounds check failed";
	}
	ISDeviceNValue &operator=(const ISDeviceNValue &other)
	{
		if(channels<other.channels)
		{
			if(values)
				delete[] values;
			values=NULL;
			channels=other.channels;
		}
		if(!values)
			values=new ISDataType[channels];
		for(int i=0;i<channels;++i)
			values[i]=other[i];
		return(*this);
	}
	int GetChannels()
	{
		return(channels);
	}
	protected:
	int channels;
	ISDataType *values;
};

#endif
