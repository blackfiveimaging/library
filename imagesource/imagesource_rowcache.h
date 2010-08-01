#ifndef IMAGESOURCE_ROWCACHE
#define IMAGESOURCE_ROWCACHE

// The row cache is just a simplistic ring-buffer type cache which handles
// the details of tracking several rows of "support" data.

class ImageSource_RowCache : public ImageSource
{
	public:
	ImageSource_RowCache(ImageSource *source,int hextra,int vextra) : ImageSource(source), source(source), hextra(hextra), vextra(vextra)
	{
		cachewidth=source->width+hextra*2+1;
		bufferrows=vextra*2+1;
		rowcache=(ISDataType *)malloc(sizeof(ISDataType)*source->samplesperpixel*source->width*bufferrows);
		rawcurrentrow=currentrow=-1;
	}
	~ImageSource_RowCache()
	{
		if(rowcache)
			free(rowcache);
	}
	ISDataType *GetRow(int row)
	{
		if(row<0)
			row=0;
		if(row>=source->height)
			row=source->height-1;

		int crow=row%(vextra*2+1);
		ISDataType *rowptr=rowcache+crow*source->samplesperpixel*source->width;

		if(row>rawcurrentrow)
		{
			if((row-rawcurrentrow)>vextra)
				rawcurrentrow=row-vextra-1;

			for(rawcurrentrow=rawcurrentrow+1;rawcurrentrow<=row;++rawcurrentrow)
			{
				ISDataType *src=source->GetRow(row);
				int crow=row%(vextra*2+1);
				ISDataType *rowptr=rowcache+crow*source->samplesperpixel*source->width;

				// Store the row to be cached in a temporary buffer...
				for(int s=0;s<source->width*source->samplesperpixel;++s)
				{
					rowptr[s+hextra*samplesperpixel]=src[s];
				}
				for(int x=0;x<hextra;++x)
				{
					for(int s=0;s<samplesperpixel;++s)
					{
						rowptr[x*samplesperpixel+s]=rowptr[hextra*samplesperpixel+s];
						rowptr[(width+x+hextra)*samplesperpixel+s]=rowptr[(width-1+hextra)*samplesperpixel+s];
					}
				}
			}
		}
		rawcurrentrow=row;
		return(rowptr+samplesperpixel*hextra);
	}
	private:
	ImageSource *source;
	ISDataType *rowcache;
	int cachewidth;
	int bufferrows;
	int rawcurrentrow;
	int hextra,vextra;
};


#endif

