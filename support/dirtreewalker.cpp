#include <iostream>
#include <string>

#include <dirent.h>
#include <sys/stat.h>

#include "searchpath.h"

#include "dirtreewalker.h"

using namespace std;

DirTreeWalker::DirTreeWalker(const char *initialpath,DirTreeWalker *parent)
	: parent(parent), child(NULL), path(initialpath), filename(), files(NULL), dirs(NULL)
{
	dirs=opendir(initialpath);
	files=opendir(initialpath);
}


DirTreeWalker::~DirTreeWalker()
{
	if(child)
		delete child;
	if(files)
		closedir(files);
	if(dirs)
		closedir(dirs);
}


const char *DirTreeWalker::NextFile()
{
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
			filename=path+SEARCHPATH_SEPARATOR+de->d_name;

			struct stat statbuf;
			stat(filename.c_str(),&statbuf);

			// Do we have a file?
			if(!S_ISDIR(statbuf.st_mode))
			{
				return(filename.c_str());
			}
		}
	}
	return(NULL);
}


DirTreeWalker *DirTreeWalker::NextDirectory()
{
	if(!dirs)
		return(NULL);

	if(child)
		delete child;
	child=NULL;

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
			filename=path+SEARCHPATH_SEPARATOR+de->d_name;

			struct stat statbuf;
			stat(filename.c_str(),&statbuf);

			// Do we have a directory?
			if(S_ISDIR(statbuf.st_mode))
			{
				return(child=new DirTreeWalker(filename.c_str(),this));
			}
		}
	}
	return(parent);
}


