/*
Classes to simplify the handling and caching of temporary files.
Copyright (c) 2008 by Alastair M. Robinson

TempFileTracker is a header class which maintains a list of TempFiles
and deletes them when it's destroyed, typically when the application
quits.

TempFile is a base class which will be subclassed by classes needing
to generate a specific type of temporary file.
*/

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "tempfile.h"

using namespace std;


TempFile::TempFile(TempFileTracker *header,const char *pfix,const char *skey)
	: filename(NULL), prefix(NULL), searchkey(NULL), header(header), nexttempfile(NULL), prevtempfile(NULL)
{
	header->mutex.ObtainMutex();
	if((prevtempfile=header->firsttempfile))
	{
		while(prevtempfile->nexttempfile)
			prevtempfile=prevtempfile->nexttempfile;
		prevtempfile->nexttempfile=this;
	}
	else
		header->firsttempfile=this;
//	if((nexttempfile=header->firsttempfile))
//		nexttempfile->prevtempfile=this;
//	header->firsttempfile=this;

	if(skey)
		searchkey=strdup(skey);
	if(pfix)
		prefix=strdup(pfix);
	header->mutex.ReleaseMutex();
}


TempFile::~TempFile()
{
//	cerr << "In TempFile destructor" << endl;
	header->mutex.ObtainMutex();
	if(filename)
	{
//		cerr << "Removing " << filename << endl;
		remove(filename);
		free(filename);
	}
	if(searchkey)
		free(searchkey);
	if(prefix)
		free(prefix);
	if(nexttempfile)
		nexttempfile->prevtempfile=prevtempfile;
	if(prevtempfile)
		prevtempfile->nexttempfile=nexttempfile;
	else
		header->firsttempfile=nexttempfile;
	header->mutex.ReleaseMutex();
}


const char *TempFile::Filename()
{
	// If no subclass has provided a filename, generate one here.
	if(!filename)
		filename=tempnam(NULL,prefix);
	return(filename);
}


TempFile *TempFile::NextTempFile()
{
	return(nexttempfile);
}


bool TempFile::MatchTempFile(const char *skey)
{
	return(strcmp(skey,searchkey)==0);
}


// TempFileTracker


TempFileTracker::TempFileTracker() : mutex(), firsttempfile(NULL)
{

}


TempFileTracker::~TempFileTracker()
{
	mutex.ObtainMutex();
	while(firsttempfile)
		delete firsttempfile;
	mutex.ReleaseMutex();
}


TempFile *TempFileTracker::GetTempFile(const char *prefix,const char *searchkey)
{
	mutex.ObtainMutexShared();
	TempFile *result=NULL;
	if(searchkey)
		result=FindTempFile(searchkey);
	if(!result)
		result=new TempFile(this,prefix,searchkey);
	mutex.ReleaseMutexShared();
	return(result);
}


TempFile *TempFileTracker::FindTempFile(const char *searchkey)
{
	mutex.ObtainMutexShared();
	TempFile *result=NULL;
	TempFile *t=FirstTempFile();
	while(t)
	{
		t=t->NextTempFile();
		if(t->MatchTempFile(searchkey))
		{
			result=t;
			t=NULL;
		}
	}
	mutex.ReleaseMutexShared();
	return(result);
}


TempFile *TempFileTracker::FirstTempFile()
{
	mutex.ObtainMutexShared();
	TempFile *result=firsttempfile;
	mutex.ReleaseMutexShared();
	return(result);
}

