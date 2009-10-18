#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <fstream>

using namespace std;

// FIXME Win32 namespace clash
#undef ERROR

enum DebugLevel {NONE, ERROR, WARN, COMMENT, TRACE};

class NullStream : public std::streambuf, public std::ostream
{
	public:
	NullStream() : std::streambuf(), std::ostream(this)
	{
	}
	virtual ~NullStream()
	{
	}
	protected:
	virtual int overflow(int c)
	{
		return(c);
	}
};


class DebugStream
{
	public:
	DebugStream(DebugLevel level=ERROR);
	virtual ~DebugStream();
	virtual void SetLogFile(string filename);
	virtual	void SetLevel(enum DebugLevel lvl);
	virtual ostream &operator[](int idx);
	protected:
	enum DebugLevel level;
	NullStream nullstream;
	ofstream logfile;
};

extern DebugStream Debug;

#endif

