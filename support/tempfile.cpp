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

#include "debug.h"

using namespace std;


TempFile::TempFile(TempFileTracker *header,const char *pfix,const char *skey)
	: header(header), filename(NULL), prefix(NULL), searchkey(NULL)
{
	if(skey)
		searchkey=strdup(skey);
	if(pfix)
		prefix=strdup(pfix);
	if(header)
		header->AddTempFile(this);
}


TempFile::~TempFile()
{
	if(filename)
	{
		Debug[TRACE] << "Removing tempfile: " << filename << endl;
		remove(filename);
		free(filename);
	}
	if(searchkey)
		free(searchkey);
	if(prefix)
		free(prefix);
	if(header)
		header->RemoveTempFile(this);
}


const char *TempFile::Filename()
{
	// If no subclass has provided a filename, generate one here.
	if(!filename)
		filename=tempnam(NULL,prefix);
	return(filename);
}


bool TempFile::MatchTempFile(const char *skey)
{
	return(strcmp(skey,searchkey)==0);
}


// TempFileTracker


TempFileTracker::TempFileTracker() : RWMutex()
{

}


TempFileTracker::~TempFileTracker()
{
	ObtainMutex();
	while(size())
	{
		delete (*this)[0];
//		pop_front();	//	Tempfile destructor handles its own removal...
	}	
	ReleaseMutex();
}


TempFile *TempFileTracker::GetTempFile(const char *prefix,const char *searchkey)
{
	TempFile *result=NULL;
	if(searchkey)
		result=FindTempFile(searchkey);
	if(!result)
	{
		result=new TempFile(this,prefix,searchkey);
		AddTempFile(result);
	}
	return(result);
}


TempFile *TempFileTracker::FindTempFile(const char *searchkey)
{
	for(unsigned int idx=0;idx<size();++idx)
	{
		TempFile *t=(*this)[idx];
		if(t->MatchTempFile(searchkey))
		{
			return(t);
		}
	}
	return(NULL);
}


void TempFileTracker::AddTempFile(TempFile *tempfile)
{
	ObtainMutex();
	push_back(tempfile);
	tempfile->header=this;
	ReleaseMutex();
}


void TempFileTracker::RemoveTempFile(TempFile *tempfile)
{
	ObtainMutex();
	for(unsigned int idx=0;idx<size();++idx)
	{
		if((*this)[idx]==tempfile)
		{
			erase(begin()+idx);
		}
	}
	ReleaseMutex();
}

