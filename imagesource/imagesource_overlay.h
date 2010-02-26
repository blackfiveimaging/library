#ifndef IMAGESOURCE_OVERLAY_H
#define IMAGESOURCE_OVERLAY_H

#include <iostream>

class ImageSource_Overlay : public ImageSource
{
	public:
	ImageSource_Overlay(IS_TYPE type) : ImageSource()
	{
		width=height=0;
		this->type=type;
		switch(type)
		{
			case IS_TYPE_RGB:
				samplesperpixel=3;
				break;
			case IS_TYPE_RGBA:
			case IS_TYPE_CMYK:
				samplesperpixel=4;
				break;
			default:
				throw "ImageSource_Overlay: type not yet supported";
				break;
		}
		currentrow=-1;
	}
	~ImageSource_Overlay()
	{
		while(plates.size())
		{
			delete plates[0];
			plates.pop_front();
		}
	}
	void AddPlate(ImageSource *plate)
	{
		Debug[TRACE] << "Width: " << plate->width << std::endl;
		Debug[TRACE] << "Height: " << plate->height << std::endl;
		if(width==0)
		{
			width=plate->width;
			height=plate->height;
			MakeRowBuffer();
		}
		if(width!=plate->width)
			throw "ImageSource_Overlay: component image sizes must match!";
		if(height!=plate->height)
			throw "ImageSource_Overlay: component image sizes must match!";
		plates.push_back(plate);
	}
	ISDataType *GetRow(int row)
	{
		if(row==currentrow)
			return(rowbuffer);

		for(int i=0;i<width*samplesperpixel;++i)
		{
			rowbuffer[i]=0;
		}

		for(unsigned int plate=0;plate<plates.size();++plate)
		{
			ISDataType *srcdata=plates[plate]->GetRow(row);
			for(int i=0;i<width*samplesperpixel;++i)
			{
				unsigned int t=srcdata[i]+rowbuffer[i];
				if(t>IS_SAMPLEMAX)
					t=IS_SAMPLEMAX;
				rowbuffer[i]=t;
			}
		}
		currentrow=row;
		return(rowbuffer);
	}
	protected:
	std::deque<ImageSource *>plates;
};

#endif

