#ifndef DIRTREEWALKER_H
#define DIRTREEWALKER_H

#include <string>

#include "dirent.h"


// Low-level DirTreeWalker class - use like this:
//
// DirTreeWalker toplevel("/top/level/path");
// DirTreeWalker *dir;
// while(dir=toplevel.NextDirectory())
// {
//   const char *file;
//   while(file=dir->NextFile())
//   {
//     std::cout << "Got file: " << file << std::endl;
//   }
// }

class DirTreeWalker
{
	public:
	DirTreeWalker(const char *initialpath,DirTreeWalker *parent=NULL);
	~DirTreeWalker();
	DirTreeWalker *NextDirectory();
	DirTreeWalker *Parent();
	DirTreeWalker *Child();
	const char *Directory();
	const char *NextFile();	// Only valid until next call
	protected:
	std::string initialpath;
	DirTreeWalker *parent;
	DirTreeWalker *child;
	std::string filename;
#ifdef WIN32
	_WDIR *files;
	_WDIR *dirs;
#else
	DIR *files;
	DIR *dirs;
#endif
};


// Helper classes
//
// DirTree_Files - use if all you want is a recursively-scanned list of files, like so:
//
// DirTree_Files dtf("/path/to/be/scanned");
// const char *fn;
// while(fn=dtf.Next())
// {
//   std::cout << "File: " << fn << std::endl;
// }

class DirTree_Files
{
	public:
	DirTree_Files(std::string path) : dtw(path.c_str())
	{
		walker=dtw.NextDirectory();
	}
	const char *Next()	// Only valid until next call
	{
		const char *result=NULL;
		while(!result)
		{
			if(!walker)
				return(NULL);
			result=walker->NextFile();
			if(!result)
				walker=walker->NextDirectory();
		}
		return(result);
	}
	protected:
	DirTreeWalker dtw;
	DirTreeWalker *walker;
};


// DirTree_Dirs - provides a recursively-scanned list of directories.
//
// DirTree_Dirs dtd("/path/to/be/scanned");
// const char *dn;
// while(dn=dtd.Next())
// {
//   std::cout << "Dir: " << dn << std::endl;
// }

class DirTree_Dirs
{
	public:
	DirTree_Dirs(std::string path) : dtw(path.c_str())
	{
		walker=&dtw;
	}
	const char *Next()
	{
		const char *result=NULL;
		while(!result)
		{
			if(!walker)
				return(NULL);
			walker=walker->NextDirectory();
			if(!walker)
				return(dtw.Directory());
			if(!walker->Child())
				result=walker->Directory();
		}
		return(result);
	}
	protected:
	DirTreeWalker dtw;
	DirTreeWalker *walker;
};


#endif

