#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#include "pathsupport.h"

#include "searchpath.h"

#include "debug.h"

using namespace std;

class SearchPathInstance
{
	public:
	SearchPathInstance(const char *path);
	~SearchPathInstance();
	char *Simplify(const char *file);
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
		if(strncmp(file,path,strlen(path))==0)
		{
			int i=strlen(path);
			if(file[i]==SEARCHPATH_SEPARATOR)
				++i;
			if(file[i])
				return(strdup(file+i));
		}
	}
	return(strdup(file));
}


char *SearchPathInstance::MakeAbsolute(const char *file)
{
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
	: header(header), searchdirectory(NULL), searchfilename(NULL)
{

}


SearchPathIterator::~SearchPathIterator()
{
	if(searchdirectory)
		closedir(searchdirectory);

	if(searchfilename)
		free(searchfilename);
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
	struct stat statbuf;

	list<SearchPathInstance *>::iterator it=paths.begin();
	while(it!=paths.end())
	{
		Debug[TRACE] << "Searching for " << file << " in " << (*it)->path << endl;
		char *p=(*it)->MakeAbsolute(file);
		Debug[TRACE] << " -> " << p << endl;

		if(stat(p,&statbuf)==0)
			return(p);
		free(p);

		++it;
	}

	if(stat(file,&statbuf)==0)
		return(strdup(file));

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
	if(searchfilename)
		free(searchfilename);
	searchfilename=NULL;

	// If we're provided with a NULL pointer, clean up
	// the remnants of any previous run...

	if(!last)
	{
		if(searchdirectory)
			closedir(searchdirectory);
		searchdirectory=NULL;

		spiterator=header.paths.begin();
		if(spiterator!=header.paths.end())
		{
			while(!searchdirectory)
			{
				if(!(searchdirectory=opendir((*spiterator)->path)))
					++spiterator;
				if(spiterator==header.paths.end())
					return(NULL);
			}
		}
	}

	struct dirent *de=NULL;

	while(searchdirectory && !de)
	{
		de=readdir(searchdirectory);
		if(!de)
		{
			closedir(searchdirectory);
			searchdirectory=NULL;
			while(!searchdirectory && (++spiterator!=header.paths.end()))
			{
				searchdirectory=opendir((*spiterator)->path);
			}
			if(searchdirectory)
				de=readdir(searchdirectory);
		}
		if(de)
		{
			if(strcmp(".",de->d_name)==0)
				de=NULL;
			else if(strcmp("..",de->d_name)==0)
				de=NULL;
		}
	}
	if(de)
	{
		searchfilename=strdup(de->d_name);
	}
	return(searchfilename);
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
		if(homedir && strncmp(homedir,(*spi)->path,homedirlen)==0)
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
		if(homedir && strncmp(homedir,(*spi)->path,homedirlen)==0)
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
