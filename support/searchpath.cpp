#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include "pathsupport.h"

#include "searchpath.h"
#include "util.h"
#include "debug.h"

using namespace std;

class SearchPathInstance
{
	public:
	SearchPathInstance(const char *path);
	~SearchPathInstance();
	char *Simplify(const char *file);
	std::string Search(const char *file);
	char *MakeAbsolute(const char *file);
	SearchPathInstance *Next();
	protected:
	char *path;
	friend class SearchPathHandler;
	friend class SearchPathIterator;
	friend std::ostream& operator<<(std::ostream &s,SearchPathInstance &sp);
};


SearchPathInstance::SearchPathInstance(const char *path)
	: path(NULL)
{
	this->path=substitute_homedir(path);
}


SearchPathInstance::~SearchPathInstance()
{
	if(path)
		free(path);
}


char *SearchPathInstance::Simplify(const char *file)
{
	if(path && file)
	{
		unsigned int bestlen=0;
		DirTree_Dirs dtd(path);
		const char *path;
		while((path=dtd.Next()))
		{
			if(strncmp(file,path,strlen(path))==0)
			{
				if(strlen(path)>bestlen)
				{
					bestlen=strlen(path);
				}
			}
		}
		if(bestlen)
		{
			if(file[bestlen]==SEARCHPATH_SEPARATOR)
				++bestlen;
			if(file[bestlen])
				return(strdup(file+bestlen));
		}
	}
	return(SafeStrdup(file));
}


std::string SearchPathInstance::Search(const char *file)
{
#ifdef WIN32
	struct _stat statbuf;
#else
	struct stat statbuf;
#endif

	DirTree_Dirs dtd(path);
	const char *dir;
	while((dir=dtd.Next()))
	{
//		Debug[TRACE] << "Searching for " << file << " in " << dir << endl;
		std::string fn=dir;
		fn+=SEARCHPATH_SEPARATOR;
		fn+=file;
//		Debug[TRACE] << " -> " << fn << endl;
#ifdef WIN32
		wchar_t *p2=UTF8ToWChar(fn.c_str());	// We check for existence of wchar filename but return UTF8 name
		bool exists=_wstat(p2,&statbuf)==0;
		free(p2);
		if(exists)
			return(fn);
#else
		if(stat(fn.c_str(),&statbuf)==0)
			return(fn);
#endif
	}
#ifdef WIN32
		wchar_t *p2=UTF8ToWChar(file);	// We check for existence of wchar filename but return UTF8 name
		bool exists=_wstat(p2,&statbuf)==0;
		free(p2);
		if(exists)
			return(file);
#else
		if(stat(file,&statbuf)==0)
			return(file);
#endif
	return("");
}

#if 0
char *SearchPathInstance::MakeAbsolute(const char *file)
{
	// FixMe - may not work now we have recursive scanning.
	char *result=NULL;
	int l=strlen(path);
	int m=strlen(file);
	if(l&&m)
	{
		result=(char *)malloc(l+m+2);
		if(path[l-1]!=SEARCHPATH_SEPARATOR)
			sprintf(result,"%s%c%s",path,SEARCHPATH_SEPARATOR,file);
		else
			sprintf(result,"%s%s",path,file);
	}
	return(result);
}
#endif

std::ostream& operator<<(std::ostream &s,SearchPathInstance &spi)
{
	const char *homedir=get_homedir();
	char *path=spi.path;
	if(homedir && strncmp(homedir,path,strlen(homedir))==0)
	{
		s<<"$HOME";
		s<<path+strlen(homedir);
	}
	else
		s<<spi.path;
	return(s);
}

// SearchPathIterator

SearchPathIterator::SearchPathIterator(SearchPathHandler &header)
	: header(header), toplevel(NULL), subdirs(NULL)
{

}


SearchPathIterator::~SearchPathIterator()
{
	if(toplevel)
		delete toplevel;
}

// SearchPathHandler


SearchPathHandler::SearchPathHandler()
	:	searchiterator(NULL)
{
}


SearchPathHandler::SearchPathHandler(SearchPathHandler &other) : searchiterator(NULL)
{
	char *p=other.GetPaths();
	AddPath(p);
	free(p);
}

SearchPathHandler &SearchPathHandler::operator=(SearchPathHandler &other)
{
	ClearPaths();
	char *p=other.GetPaths();
	AddPath(p);
	free(p);
	return(*this);
}

SearchPathHandler::~SearchPathHandler()
{
	list<SearchPathInstance *>::iterator it=paths.begin();
	while(it!=paths.end())
	{
		delete (*it);
		++it;
	}

}


char *SearchPathHandler::SearchPaths(const char *file)
{
#ifdef WIN32
	struct _stat statbuf;
#else
	struct stat statbuf;
#endif
	Debug[WARN] << "Searching for " << file << endl;

	list<SearchPathInstance *>::iterator it=paths.begin();
	while(it!=paths.end())
	{
		Debug[TRACE] << "Searching for " << file << " in " << (*it)->path << endl;
		std::string p=(*it)->Search(file);
		if(p.size())
		{
			Debug[TRACE] << " -> " << p << endl;
			return(strdup(p.c_str()));
		}
		++it;
	}
	return(NULL);
}


void SearchPathHandler::AddPath(const char *path)
{
	if(path && strlen(path))
	{
		try
		{
			char *p=strdup(path);
			char *p2=p;
			char *p3=p;
			while(*p3)
			{
				if(*p3==SEARCHPATH_DELIMITER_C)
				{
					*p3=0;
					paths.push_back(new SearchPathInstance(p2));
					p2=p3+1;
				}
				++p3;
			}
			paths.push_back(new SearchPathInstance(p2));
			free(p);
		}
		catch(const char *err)
		{
			Debug[ERROR] << "Error: " << err << endl;
		}
	}
}


SearchPathInstance *SearchPathHandler::FindPath(const char *path)
{
	SearchPathInstance *result=NULL;
	char *p=NULL;

	if(path)
	{
		if(strncmp(path,"$HOME",5)==0)
		{
			char *homedir=getenv("HOME");
			if(!homedir)
				throw "No home directory";
			p=(char *)malloc(strlen(path)-5+strlen(homedir)+2);
			sprintf(p,"%s%s",homedir,path+5);
		}
		else
			p=strdup(path);

		list<SearchPathInstance *>::iterator it=paths.begin();
		while(it!=paths.end())
		{
			if(strcmp((*it)->path,p)==0)
				result=(*it);
			++it;
		}
		free(p);
	}
	return(result);
}


void SearchPathHandler::RemovePath(const char *path)
{
	SearchPathInstance *spi=FindPath(path);
	if(spi)
		delete spi;
}


void SearchPathHandler::ClearPaths()
{
	list<SearchPathInstance *>::iterator it=paths.begin();
	while(it!=paths.end())
	{
		delete (*it);
		++it;
	}
	paths.clear();
}


char *SearchPathHandler::MakeRelative(const char *path)
{
	char *best=NULL;
	unsigned int bestlen=100000;

	if(!path)
		return(NULL);

	list<SearchPathInstance *>::iterator it=paths.begin();
	while(it!=paths.end())
	{
		char *rel=(*it)->Simplify(path);
		if(strlen(rel)<bestlen)
		{
			if(best)
				free(best);
			best=rel;
			bestlen=strlen(best);
		}
		else
			free(rel);
		++it;
	}
	if(!best)
		best=strdup(path);
	return(best);
}


const char *SearchPathIterator::GetNextFilename(const char *last)
{
//	Debug.PushLevel(TRACE);
	// If we're provided with a NULL pointer, clean up
	// the remnants of any previous run...

	if(!last || !subdirs)
	{
//		Debug[TRACE] << "No previous filename provided - starting over..." << endl;
		if(toplevel)
			delete toplevel;
		toplevel=NULL;

		spiterator=header.paths.begin();
		if(spiterator==header.paths.end())
			return(NULL);

//		Debug[TRACE] << "Setting toplevel to " << (*spiterator)->path << endl;
		toplevel=new DirTreeWalker((*spiterator)->path);
		subdirs=toplevel->NextDirectory();
		++spiterator;
	}

	const char *searchfilename=NULL;
	while(!searchfilename)
	{
		if(subdirs)
			searchfilename=subdirs->NextFile();
		if(!searchfilename)
		{
//			Debug[TRACE] << "No more filenames, going onto next directory..." << endl;

			if(subdirs)
				subdirs=subdirs->NextDirectory();

			// If we've reached the end of the files, we load the next path...
			if(!subdirs)
			{
//				Debug[TRACE] << "No more directories, going onto next path..." << endl;
				if(toplevel)
					delete toplevel;
				toplevel=NULL;

				if(spiterator==header.paths.end())
					return(NULL);

//				Debug[TRACE] << "Setting toplevel to " << (*spiterator)->path << endl;
				toplevel=new DirTreeWalker((*spiterator)->path);
				subdirs=toplevel->NextDirectory();
				++spiterator;
			}
		}
	}
//	Debug[TRACE] << "Returning filename " << (searchfilename ? searchfilename : "NULL") << endl;
//	Debug.PopLevel();
	searchfn="";
	if(searchfilename)
	{
		char *tmp=strdup(searchfilename);
		searchfn=basename(tmp);
		free(tmp);
	}
	return(searchfn.c_str());
}


const char *SearchPathIterator::GetNextPath(const char *last)
{
	const char *result=NULL;

	if(!last)
		spiterator=header.paths.begin();

	if(spiterator!=header.paths.end())
	{
		result=(*spiterator)->path;
		++spiterator;
	}
	return(result);
}


std::ostream& operator<<(std::ostream &s,SearchPathHandler &sp)
{
	list<SearchPathInstance *>::iterator it=sp.paths.begin();
	while(it!=sp.paths.end())
	{
		s << (*it);
		++it;
		if(it!=sp.paths.end())
			s << SEARCHPATH_DELIMITER_S;
	}
	return(s);
}


char *SearchPathHandler::GetPaths()
{
	const char *homedir=get_homedir();
	int homedirlen=0;
	int sl=0;

	if(homedir)
		homedirlen=strlen(homedir);

	list<SearchPathInstance *>::iterator spi=paths.begin();
	while(spi!=paths.end())
	{
		if(homedir && homedirlen && strncmp(homedir,(*spi)->path,homedirlen)==0)
			sl+=strlen((*spi)->path)+strlen("$HOME/")+1-homedirlen;
		else
			sl+=strlen((*spi)->path)+1;
		++spi;
	}

	char *result=(char *)malloc(sl+1);
	result[0]=0;

	spi=paths.begin();
	while(spi!=paths.end())
	{
		if(homedir && homedirlen && strncmp(homedir,(*spi)->path,homedirlen)==0)
		{
			strcat(result,"$HOME");
			strcat(result,(*spi)->path+homedirlen);
			++spi;
			if(spi!=paths.end())
				strcat(result,SEARCHPATH_DELIMITER_S);
		}
		else
		{
			strcat(result,(*spi)->path);
			++spi;
			if(spi!=paths.end())
				strcat(result,SEARCHPATH_DELIMITER_S);
		}
	}
	return(result);
}
