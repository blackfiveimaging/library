#ifndef MD5CONSUMER_H
#define MD5CONSUMER_H

#include <cstring>

#include "consumer.h"
#include "md5.h"

// A variation on the consumer class which maintains an MD5 hash instead of sending the data anywhere.

class MD5Consumer : public Consumer, public MD5Digest
{
	public:
	MD5Consumer() : Consumer(), MD5Digest()
	{
	}
	virtual ~MD5Consumer()
	{
	}
	virtual bool Write(const char *buffer,int length)
	{
		Update(buffer,length);

		// There's an issue with the PostScript driver in that it includes the creation date
		// so the MD5 hash will be different every time.  To get around this we reset the MD5 if we
		// encounter a CreationDate comment.
		if(length>14 && strncmp(buffer,"%%CreationDate",14)==0)
			InitMD5Context();
		return(true);
	}
	virtual void Cancel()
	{
	}
};

#endif

