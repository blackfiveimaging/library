#ifndef IMAGESOURCE_COLORIZE_H
#define IMAGESOURCE_COLORIZE_H

class ImageSource_Colorize : public ImageSource
{
	public:
	ImageSource_Colorize(ImageSource *src,IS_TYPE type,ISDeviceNValue &colour) : ImageSource(src), src(src), colour(colour)
	{
		this->type=type;
		samplesperpixel=colour.GetChannels();
		MakeRowBuffer();
		if(STRIP_ALPHA(src->type)!=IS_TYPE_GREY)
			throw "ImageSource_Colorize currently only supports greyscale input";
		currentrow=-1;
	}
	~ImageSource_Colorize()
	{
		if(src)
			delete src;
	}
	ISDataType *GetRow(int row)
	{
		if(row==currentrow)
			return(rowbuffer);
		ISDataType *srcdata=src->GetRow(row);

		if(HAS_ALPHA(src->type))
		{
			for(int x=0;x<width;++x)
			{
				unsigned int t=srcdata[x*2];
				for(int s=0;s<samplesperpixel;++s)
				{
					rowbuffer[x*samplesperpixel+s]=(t*colour[s])/IS_SAMPLEMAX;
				}
			}
		}
		else
		{
			for(int x=0;x<width;++x)
			{
				unsigned int t=srcdata[x];
				for(int s=0;s<samplesperpixel;++s)
				{
					rowbuffer[x*samplesperpixel+s]=(t*colour[s])/IS_SAMPLEMAX;
				}
			}
		}
		currentrow=row;
		return(rowbuffer);
	}
	protected:
	ImageSource *src;
	ISDeviceNValue &colour;
};

#endif

