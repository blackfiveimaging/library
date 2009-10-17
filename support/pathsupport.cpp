#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string.h>
#include <glib.h>

#ifdef WIN32
#include <w32api.h>
#define _WIN32_IE IE5
#define _WIN32_WINNT Windows2000
#include <shlobj.h>
#endif

#include "debug.h"
#include "searchpath.h"
#include "pathsupport.h"

using namespace std;


const char *get_homedir()
{
//	return(getenv("HOME"));
#ifdef WIN32
	static char homedir[MAX_PATH]={0};
	static bool init=false;
	if(!init)
	{
		SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE(SHGFP_TYPE_CURRENT),homedir);
	}
	return(homedir);
#else
	return(g_get_home_dir());
#endif
}


char *substitute_homedir(const char *path)
{
	char *result=NULL;
	if(path)
	{
		if(path[0]=='~')
			++path;

		else if(strncmp(path,"$HOME",5)==0)
			path+=5;

		else	// No substitution to be done...
			return(strdup(path));

		if(path[0]=='/' || path[0]=='\\')
			++path;

		// If we get this far, then we need to substitute - and path now points
		// to the beginning of the path proper...
		const char *hd=get_homedir();

		result=(char *)malloc(strlen(path)+strlen(hd)+2);	

		sprintf(result,"%s%c%s",hd,SEARCHPATH_SEPARATOR,path);
	}
	return(result);
}


char *substitute_xdgconfighome(const char *path)
{
	const char *envvar="$XDG_CONFIG_HOME";
	char *result=NULL;
	if(path)
	{
		if(path[0]=='~')
			++path;

		else if(strncmp(path,envvar,strlen(envvar))==0)
			path+=strlen(envvar);

		else	// No substitution to be done...
			return(strdup(path));

		if(path[0]=='/' || path[0]=='\\')
			++path;

		// If we get this far, then we need to substitute - and path now points
		// to the beginning of the path proper...
		const char *hd=NULL;
		if((hd=getenv(envvar+1)))
		{
//			Debug[TRACE] << "Got XDG_CONFIG_HOME: " << hd << endl;
			result=(char *)malloc(strlen(path)+strlen(hd)+2);
			sprintf(result,"%s%c%s",hd,SEARCHPATH_SEPARATOR,path);
		}
		else
		{
//			Debug[TRACE] << "No XDG_CONFIG_HOME set - using $HOME/.config instead" << endl;
			const char *hd=g_get_home_dir();
			result=(char *)malloc(strlen(hd)+strlen("/.config/")+strlen(path)+strlen(hd)+2);
			sprintf(result,"%s%c.config%c%s",hd,SEARCHPATH_SEPARATOR,SEARCHPATH_SEPARATOR,path);
		}
		
	}
	return(result);
}

