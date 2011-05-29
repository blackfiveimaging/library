#include <iostream>

#include "imagesource.h"
#include "imagesource_util.h"

#include "debug.h"
#include "imagesink.h"
#include "refcountptr.h"
#include "cachedimage.h"
#include "tiffsaver.h"
#include "jpegsaver.h"


int main(int argc, char **argv)
{
	Debug.SetLevel(TRACE);
	try
	{
		if(argc>1)
		{
			char *outfn(BuildFilename(argv[1],"rcptest","jpg"));
			RefCountPtr<ImageSource> is(ISLoadImage(argv[1]));
			CachedImage_Deferred sink1(is);
			CachedImage_Deferred sink2(is);
			JPEGSaver saver(&*outfn,is);
			CachedImage_Deferred sink4(is);
			for(int i=0;i<is->height;++i)
			{
				sink1.ProcessRow(i);
				sink2.ProcessRow(i);
				saver.ProcessRow(i);
			}
			free(outfn);
		}
	}
	catch (const char *err)
	{
		Debug[ERROR] << err << std::endl;
	}
	return(0);
}

