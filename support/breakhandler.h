#ifndef BREAKHANDLER_H
#define BREAKHANDLER_H

#include <iostream>
#include <signal.h>
#include <time.h>
#include "debug.h"

using namespace std;

class BreakSignalHandler
{
	public:
	BreakSignalHandler()
	{
		breakreceived=false;
		oldtime=time(NULL);
		signal(SIGINT, &sighandler);
	}
	inline bool TestBreak()	// Returns true if break has been received
	{
		return(breakreceived);
	}
	protected:
	static void sighandler(int sig);
	static bool breakreceived;
	static time_t oldtime;
};

extern BreakSignalHandler BreakHandler;

#endif

