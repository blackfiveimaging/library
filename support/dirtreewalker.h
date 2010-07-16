#ifndef DIRTREEWALKER_H
#define DIRTREEWALKER_H

#include <string>

#include "dirent.h"

class DirTreeWalker : public std::string
{
	public:
	DirTreeWalker(const char *initialpath,DirTreeWalker *parent=NULL);
	~DirTreeWalker();
	DirTreeWalker *NextDirectory();
	DirTreeWalker *Parent();
	const char *NextFile();
	protected:
	DirTreeWalker *parent;
	DirTreeWalker *child;
//	std::string path;
	std::string filename;
#ifdef WIN32
	_WDIR *files;
	_WDIR *dirs;
#else
	DIR *files;
	DIR *dirs;
#endif
};

#endif

