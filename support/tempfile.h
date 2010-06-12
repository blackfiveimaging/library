#ifndef TEMPFILE_H
#define TEMPFILE_H

#include <deque>

#include "rwmutex.h"

class TempFileTracker;
class TempFile
{
	public:
	TempFile(const char *prefix=NULL,const char *searchkey=NULL);
	virtual ~TempFile();
	virtual const char *Filename();
	virtual bool MatchTempFile(const char *searchkey);
	protected:
	char *filename;
	char *prefix;
	char *searchkey;
};


class TempFileTracker : public std::deque<TempFile *>, public RWMutex
{
	public:
	TempFileTracker();
	~TempFileTracker();
	// GetTempFile returns an existing tempfile if it's able to find one from the searchkey.
	// If not, or if no searchkey is given, a new tempfile is created.
	TempFile *GetTempFile(const char *prefix,const char *searchkey=NULL);

	// Find a tempfile from a given searchkey, returning NULL if not found.
	// In multi-threaded apps, should hold the mutex around this call.
	TempFile *FindTempFile(const char *searchkey);

	// Add a tempfile to the tracker.
	void Add(TempFile *tempfile);
	protected:
	friend class TempFile;
};


#endif

