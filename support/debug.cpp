#include <iostream>
#include <fstream>

#include "debug.h"

DebugStream debug;

using namespace std;

DebugStream::DebugStream(DebugLevel level) : level(level)
{
}

DebugStream::~DebugStream()
{
	if(logfile.is_open())
		logfile.close();
}

void DebugStream::SetLogFile(string filename)
{
	if(logfile.is_open())
		logfile.close();
	logfile.open(filename.c_str());
}


DebugLevel DebugStream::SetLevel(enum DebugLevel lvl)
{
	DebugLevel oldlevel=level;
	level=lvl;
	return(oldlevel);
}


ostream &DebugStream::operator[](int idx)
{
	if(idx>level)
		return(nullstream);
	else if(logfile.is_open())
		return(logfile);
	else
		return(cerr);
}

DebugStream Debug;

