#include <iostream>

#include "breakhandler.h"

void BreakSignalHandler::sighandler(int sig)
{
	time_t now=time(NULL);
	if((now-oldtime)<2)
	{
		Debug[WARN] << "Multiple Ctrl-C signals received, quitting at user's insistence." << std::endl;
		exit(0);
	}
	oldtime=now;
//	signal(SIGINT, SIG_DFL);
	breakreceived=true;
}

bool BreakSignalHandler::breakreceived;
time_t BreakSignalHandler::oldtime;

BreakSignalHandler BreakHandler;

