#include <iostream>
#include <string>

#include <cstring>
#include <cstdlib>

#include <dirent.h>
#include <sys/stat.h>

#include "searchpath.h"
#include "util.h"
#include "debug.h"

#include "dirtreewalker.h"

using namespace std;

DirTreeWalker::DirTreeWalker(const char *initialpath,DirTreeWalker *parent)
	: initialpath(initialpath), parent(parent), child(NULL), filename(), files(NULL), dirs(NULL)
{
	Debug[TRACE] << "DTW: " << initialpath << endl;
#ifdef WIN32
	wchar_t *ip=UTF8ToWChar(initialpath);
	dirs=_wopendir(ip);
	files=_wopendir(ip);
	free(ip);
#else
	dirs=opendir(initialpath);
	files=opendir(initialpath);
#endif
}


DirTreeWalker::~DirTreeWalker()
{
	if(child && child!=this)
		delete child;
#ifdef WIN32
	if(files)
		_wclosedir(files);
	if(dirs)
		_wclosedir(dirs);
#else
	if(files)
		closedir(files);
	if(dirs)
		closedir(dirs);
#endif
}


DirTreeWalker *DirTreeWalker::Parent()
{
	return(parent);
}


DirTreeWalker *DirTreeWalker::Child()
{
	return(child);
}


const char *DirTreeWalker::NextFile()
{
#ifdef WIN32
	if(!files)
		return(NULL);
	struct _wdirent *de=NULL;

	while((de=_wreaddir(files)))
	{
		char *dname=NULL;
		if(de)
		{
			dname=WCharToUTF8(de->d_name);
			if(strcmp(".",dname)==0)
				de=NULL;
			else if(strcmp("..",dname)==0)
				de=NULL;
		}
		if(de)
		{
			filename=*this+SEARCHPATH_SEPARATOR+dname;
			free(dname);
			wchar_t *wfn=UTF8ToWChar(filename.c_str());
			struct _stat statbuf;
			_wstat(wfn,&statbuf);
			free(wfn);

			// Do we have a file?
			if(!S_ISDIR(statbuf.st_mode))
			{
				return(filename.c_str());
			}
		}
		if(dname)
			free(dname);
	}
#else
	if(!files)
		return(NULL);
	struct dirent *de=NULL;

	while((de=readdir(files)))
	{
		if(de)
		{
			if(strcmp(".",de->d_name)==0)
				de=NULL;
			else if(strcmp("..",de->d_name)==0)
				de=NULL;
		}
		if(de)
		{
			filename=initialpath+SEARCHPATH_SEPARATOR+de->d_name;

			struct stat statbuf;
			stat(filename.c_str(),&statbuf);

			// Do we have a file?
			if(!S_ISDIR(statbuf.st_mode))
			{
				return(filename.c_str());
			}
		}
	}
#endif
	return(NULL);
}


const char *DirTreeWalker::Directory()
{
	return(initialpath.c_str());
}


DirTreeWalker *DirTreeWalker::NextDirectory()
{
	if(!dirs)
		return(NULL);

#ifdef WIN32
	struct _wdirent *de=NULL;

	while((de=_wreaddir(dirs)))
	{
		char *dname=NULL;
		if(de)
		{
			dname=WCharToUTF8(de->d_name);
			if(strcmp(".",dname)==0)
				de=NULL;
			else if(strcmp("..",dname)==0)
				de=NULL;
		}
		if(de)
		{
			filename=initialpath+SEARCHPATH_SEPARATOR+dname;

			wchar_t *fnw=UTF8ToWChar(filename.c_str());
			struct _stat statbuf;
			_wstat(fnw,&statbuf);
			free(fnw);

			// Do we have a directory?
			if(S_ISDIR(statbuf.st_mode))
			{
				// We leave child deletion until now so we can catch the case of a directory with no subdirs.
				if(child)
					delete child;
				return(child=new DirTreeWalker(filename.c_str(),this));
			}
		}
	}
	return(parent);
#else
	struct dirent *de=NULL;

	while((de=readdir(dirs)))
	{
		if(de)
		{
			if(strcmp(".",de->d_name)==0)
				de=NULL;
			else if(strcmp("..",de->d_name)==0)
				de=NULL;
		}
		if(de)
		{
			filename=initialpath+SEARCHPATH_SEPARATOR+de->d_name;

			struct stat statbuf;
			stat(filename.c_str(),&statbuf);

			// Do we have a directory?
			if(S_ISDIR(statbuf.st_mode))
			{
				// We leave child deletion until now so we can catch the case of a directory with no subdirs.
				if(child)
					delete child;
				return(child=new DirTreeWalker(filename.c_str(),this));
			}
		}
	}
	// Child will only be NULL if the directory has no subdirectories.
	if(!child)
		return(child=this);
	return(parent);
#endif
}


