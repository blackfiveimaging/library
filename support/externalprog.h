#ifndef EXTERNALPROG_H
#define EXTERNALPROG_H

#include <string>
#include <deque>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#ifndef WIN32
#include <sys/wait.h>
#endif
#include "searchpath.h"
#include "pathsupport.h"


class ExternalProgArgList : public std::deque<std::string>
{
	public:
	ExternalProgArgList() : std::deque<std::string>()
	{
	}
	virtual ~ExternalProgArgList()
	{
	}
	virtual std::string &operator[](unsigned int i)
	{
		if(i<0)
			throw "ExternalProgArgList - index must be >= 0";
		while(size()<=i)
		{
			push_back("");
		}
		return(std::deque<std::string>::operator[](i));
	}
};


class ExternalProgram : public SearchPathHandler
{
	public:
	ExternalProgram() : SearchPathHandler(), forkpid(0)
	{
	}
	virtual ~ExternalProgram()
	{
	}
	virtual void SetDefaultPath()
	{
		// If the application knows where a particular program should be stored,
		// such as /usr/lib/cups/backend, or c:/Program Files/PhotoPrint/
		// then override and set in here.
		// Note: Constructor doesn't call this, because the vtables aren't set up in time!
		// Thus, you should call this in your own constructor.
	}
	virtual void AddArg(const std::string &arg)
	{
		args[args.size()]=arg;
	}
	virtual void RunProgram()
	{
		Debug.PushLevel(TRACE);	// Promote debuglevel to TRACE for this call
		Debug[TRACE] << "Hunting for " << args[0] << std::endl;
		char *prgname=SearchPaths(args[0].c_str());
		if(!prgname)
		{
			Debug.PopLevel();
			throw "Can't find external program";
		}
		Debug[TRACE] << "Found external program at " << prgname << std::endl;
		char **arglist=(char **)malloc(sizeof(char *)*(args.size()+1));
		for(unsigned int i=0;i<args.size();++i)
		{
			arglist[i]=strdup(args[i].c_str());
			Debug[TRACE] << "Argument: " << i << ": " << args[i].c_str() << std::endl;
		}
		arglist[args.size()]=NULL;

#ifdef WIN32
		Debug[TRACE] << "Launching subprocess and waiting for completion..." << std::endl;
		int status=_spawnv(_P_WAIT,prgname,arglist);
		Debug[TRACE] << "Subprocess returned code " << status << std::endl;
#else
		switch((forkpid=fork()))
		{
			case -1:
				throw "Unable to launch subprocess";
				break;
			case 0:
				Debug[TRACE] << "Subprocess running..." << std::endl;
				execv(prgname,arglist);
				break;
			default:
				Debug[TRACE] << "Waiting for subprocess to complete..." << std::endl;
				int status;
				waitpid(forkpid,&status,0);
				Debug[TRACE] << "Subprocess complete." << std::endl;
				break;
		}		
#endif
		for(unsigned int i=0;i<args.size();++i)
		{
			if(arglist[i])
				free(arglist[i]);
		}
		free(arglist);
		free(prgname);
		Debug.PopLevel();
	}
	virtual void StopProgram()
	{
#ifdef WIN32
		
#else
		if(forkpid)
			kill(forkpid,SIGTERM);
#endif
	}
	virtual void ClearArgs()
	{
		while(args.size()>1)
			args.pop_back();
	}
	protected:
	ExternalProgArgList args;
#ifdef WIN32
	intptr_t forkpid;
#else
	int forkpid;
#endif
};

#endif

