#ifndef TEMPFILE_H
#define TEMPFILE_H

class TempFileTracker;
class TempFile
{
	public:
	TempFile(TempFileTracker *header,const char *prefix,const char *searchkey=NULL);
	virtual ~TempFile();
	virtual const char *Filename();
	virtual TempFile *NextTempFile();
	virtual bool MatchTempFile(const char *searchkey);
	protected:
	char *prefix;
	char *filename;
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
	protected:
	TempFile *firsttempfile;
	friend class TempFile;
};


#endif

