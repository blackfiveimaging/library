/*
 * util.cpp - miscellaneous support functions.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "debug.h"
#include "searchpath.h"
#include "pathsupport.h"

using namespace std;

bool CreateDirIfNeeded(const char *path)
{
	struct stat s;
	if(stat(path,&s)==0)
		return(true);

	if(errno==ENOENT)
	{
#ifdef WIN32
		return(mkdir(path)==0);
#else
		return(mkdir(path,0755)==0);
#endif
	}
	return(false);
}


bool CheckFileExists(const char *file)
{
	struct stat s;
	return(stat(file,&s)==0);
}


bool CheckSettingsDir(const char *dirname)
{
	const char *homedir=get_homedir();
	if(homedir)
	{
		char *path=(char *)malloc(strlen(homedir)+strlen(dirname)+2);
		sprintf(path,"%s%c%s",homedir,SEARCHPATH_SEPARATOR,dirname);

		Debug[TRACE] << "Settings directory: " << path << endl;
		CreateDirIfNeeded(path);

		free(path);
		return(true);
	}
	else
		return(false);
}


char *BuildAbsoluteFilename(const char *fname)
{
	char *result=NULL;
	char cwdbuf[1024];
	int l;

	if(!(getcwd(cwdbuf,1023)))
		throw "Can't get curent working directory";

	l=strlen(fname)+strlen(cwdbuf)+3;	
	result=(char *)malloc(l);
	
	sprintf(result,"%s%c%s",cwdbuf,SEARCHPATH_SEPARATOR,fname);	
	return(result);
}


char *BuildFilename(const char *root,const char *suffix,const char *fileext)
{
	// Build a filename like <imagename><channel>.<extension>
	// Must take care not to treat any . characters before the last slash as
	// a file-extension point!

	char *extension;

	char *filename=NULL;
	if(fileext)
		filename=(char *)malloc(strlen(root)+strlen(suffix)+strlen(fileext)+3);
	else
		filename=(char *)malloc(strlen(root)+strlen(suffix)+3);

	char *root2=strdup(root);
	extension = root2 + strlen (root2) - 1;
	while (extension >= root2)
	{
		if(*extension == '/' || *extension == '\\')
			extension=root2;
		if (*extension == '.') break;
		extension--;
	}
	if (extension >= root2 && fileext && strlen(fileext))
	{
		*(extension++) = '\0';
		sprintf(filename,"%s%s.%s", root2, suffix, fileext);
	}
	else
	{
		if(extension>=root2)
			*(extension++) = '\0';
		sprintf(filename,"%s%s", root2, suffix);
	}
	free(root2);

	return(filename);
}


char *SerialiseFilename(const char *fname,int serialno,int max)
{
	int digits=0;
	while(max)
	{
		++digits;
		max/=10;
	}
	char *ftmp=strdup(fname);
	const char *extension="";
	int idx=strlen(ftmp)-1;
	while(idx>0)
	{
		if(ftmp[idx]=='.')
			break;
		--idx;
	}
	if(idx)
	{
		extension=ftmp+idx+1;
		ftmp[idx]=0;
	}

	char *result=(char *)malloc(strlen(ftmp)+strlen(extension)+digits+4);
	if(digits)
		sprintf(result,"%s_%0*d.%s",ftmp,digits,serialno,extension);
	else
		sprintf(result,"%s_%d.%s",ftmp,serialno,extension);
	free(ftmp);
	return(result);
}



int TestNumeric(char *str)
{
	int result=1;
	int c;
	while((c=*str++))
	{
		if((c<'0')||(c>'9'))
			result=0;
	}
	return(result);
}


bool TestHostName(char *str,char **hostname,int *port)
{
	int c;
	char *src=str;
	while((c=*src++))
	{
		if(c==':')
		{
			if(TestNumeric(src))
			{
				int hnl=src-str;
				*port=atoi(src);
				*hostname=(char *)malloc(hnl+1);
				strncpy(*hostname,str,hnl);
				(*hostname)[hnl-1]=0;
				return(true);
			}
		}

	}
	return(false);
}


bool CompareFiles(const char *fn1,const char *fn2)
{
	bool result=true;
	int l1,l2;
	char *buf1,*buf2;
	ifstream i1,i2;
 	i1.open(fn1,ios::binary);
	i2.open(fn2,ios::binary);

	i1.seekg(0, ios::end);
	l1= i1.tellg();
	i1.seekg (0, ios::beg);

	i2.seekg(0, ios::end);
	l2= i2.tellg();
	i2.seekg (0, ios::beg);

	if(l1==l2)
	{
		buf1 = new char [l1];
		buf2 = new char [l2];

		i1.read (buf1,l1);
		i2.read (buf2,l2);

		for(int i=0;i<l1;++i)
		{
			if(buf1[i]!=buf2[i])
			{
				result=false;
				i=l1;
			}
		}
		delete[] buf1;
		delete[] buf2;
	}
	else
		result=false;
	i1.close();
	i2.close();

	return(result);
}


// A "safe" version of strdup which returns a valid pointer to an empty string
// rather than NULL if src is NULL

char *SafeStrdup(const char *src)
{
	if(src)
		return(strdup(src));
	else
		return(strdup(""));
}


// A "safe" version of strcat which returns a valid pointer to an empty string
// if both parameters are null, a copy of the non-null parameter if the other is
// null, or a newly-allocated string containing the concatenated parameters if
// both are valid.
// In all cases the result is in newly-allocated storage and may (and must!) be
// free()ed when no longer needed.

char *SafeStrcat(const char *str1,const char *str2)
{
	if(str1 && str2)
	{
		int l=strlen(str1)+strlen(str2)+1;
		char *result=(char *)malloc(l);
		sprintf(result,"%s%s",str1,str2);
		return(result);
	}
	if(str1)
		return(strdup(str1));
	if(str2)
		return(strdup(str2));
	return(strdup(""));
}


// Utility function to compare two strings, but ignoring differences
// due to spaces.
int StrcasecmpIgnoreSpaces(const char *str1,const char *str2)
{
	while((*str1!=0) && (*str2!=0))
	{
		while(*str1==32)
			++str1;
		while(*str2==32)
			++str2;
		int s1=(*str1++) & ~32;
		int s2=(*str2++) & ~32;
		if(s1!=s2)
		{
			if(s1<s2)
				return(-1);
			else
				return(1);
		}
	}
	// If we reach here, the strings must be equal.
	return(0);
}


// Dummy class for seeding the random number generator.  Declaring a global instance
// of this class ensures that the seed is set once and only once for the entire program.

class RandomSeededClass
{
	public:
	RandomSeededClass()
	{
		srand(time(NULL));
	}
	inline int Random(int max)
	{
		return(rand() % max);
	}
};

RandomSeededClass globalrandomseeded;

// By deferring the call to the class's function we ensure that the global
// is instantiated, and thus the seed is set.

int RandomSeeded(int max)
{
	return(globalrandomseeded.Random(max));
}

