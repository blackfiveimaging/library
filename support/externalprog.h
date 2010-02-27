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
		Debug[TRACE] << "Referencing argument " << i << std::endl;

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
		Debug[TRACE] << "Hunting for " << args[0] << std::endl;
		char *prgname=SearchPaths(args[0].c_str());
		if(!prgname)
			throw "Can't find external program";
#ifdef WIN32
		string cmd(prgname);
		cmd+=" ";
		for(int i=1;i<argc;++i)
		{
			if(argv[i])
			{
				cmd+=argv[i];
				cmd+=" ";
			}
		}
		Debug[TRACE] << "Using command " << cmd << std::endl;
		system(cmd.c_str());
#else
		char **arglist=(char **)malloc(sizeof(char *)*(args.size()+1));
		arglist[0]=strdup(prgname);
		for(unsigned int i=1;i<args.size();++i)
		{
			arglist[i]=strdup(args[i].c_str());
			Debug[TRACE] << "Argument: " << i << ": " << args[i].c_str() << std::endl;
		}
		arglist[args.size()]=NULL;

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

		for(unsigned int i=0;i<args.size();++i)
		{
			if(arglist[i])
				free(arglist[i]);
		}
		free(arglist);
#endif
		free(prgname);
	}
	virtual void StopProgram()
	{
		if(forkpid)
			kill(forkpid,SIGTERM);
	}
	virtual void ClearArgs()
	{
		while(args.size()>1)
			args.pop_back();
	}
	protected:
	ExternalProgArgList args;
	int forkpid;
};

#endif

