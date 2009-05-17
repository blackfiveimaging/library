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
	: filename(NULL), searchkey(NULL), header(header), nexttempfile(NULL), prevtempfile(NULL)
{
	if((nexttempfile=header->firsttempfile))
		nexttempfile->prevtempfile=this;
	header->firsttempfile=this;

	if(skey)
		searchkey=strdup(skey);
	if(pfix)
		prefix=strdup(pfix);
}


TempFile::~TempFile()
{
	if(filename)
	{
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


TempFileTracker::TempFileTracker() : firsttempfile(NULL)
{

}


TempFileTracker::~TempFileTracker()
{
	while(firsttempfile)
		delete firsttempfile;
}


TempFile *TempFileTracker::GetTempFile(const char *prefix,const char *searchkey)
{
	if(searchkey)
	{
		TempFile *result=NULL;
		if((result==FindTempFile(searchkey)))
			return(result);
	}
	return(new TempFile(this,prefix,searchkey));
}


TempFile *TempFileTracker::FindTempFile(const char *searchkey)
{
	TempFile *t=FirstTempFile();
	while(t)
	{
		t=t->NextTempFile();
		if(t->MatchTempFile(searchkey))
			return(t);
	}
	return(NULL);
}


TempFile *TempFileTracker::FirstTempFile()
{
	return(firsttempfile);
}

