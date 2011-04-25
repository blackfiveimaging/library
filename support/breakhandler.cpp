#include <iostream>
#include <cstdlib>

#include "breakhandler.h"

void BreakSignalHandler::sighandler(int sig)
{
	Debug[TRACE] << "BreakSignalHandler - ctrl-c received" << std::endl;
	time_t now=time(NULL);
	if((now-oldtime)<2)
	{
		Debug[WARN] << "Multiple Ctrl-C signals received, quitting at user's insistence." << std::endl;
		signal(SIGINT, SIG_DFL);
		raise(SIGINT);
//		exit(0);
	}
	oldtime=now;
	breakreceived=true;
}

bool BreakSignalHandler::breakreceived;
time_t BreakSignalHandler::oldtime;

BreakSignalHandler BreakHandler;

