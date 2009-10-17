#ifndef TEMPFILE_H
#define TEMPFILE_H

#include "rwmutex.h"

class TempFileTracker;
class TempFile
{
	public:
	TempFile(TempFileTracker *header,const char *prefix=NULL,const char *searchkey=NULL);
	virtual ~TempFile();
	virtual const char *Filename();
	virtual TempFile *NextTempFile();
	virtual bool MatchTempFile(const char *searchkey);
	protected:
	char *filename;
	char *prefix;
	char *searchkey;
	TempFileTracker *header;
	TempFile *nexttempfile,*prevtempfile;
};

class TempFileTracker
{
	public:
	TempFileTracker();
	~TempFileTracker();
	TempFile *GetTempFile(const char *prefix,const char *searchkey=NULL);
	TempFile *FindTempFile(const char *searchkey);
	TempFile *FirstTempFile();
	RWMutex mutex;
	protected:
	TempFile *firsttempfile;
	friend class TempFile;
};


#endif

