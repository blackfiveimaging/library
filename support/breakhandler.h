#ifndef BREAKHANDLER_H
#define BREAKHANDLER_H

#include <iostream>
#include <signal.h>

using namespace std;

class BreakSignalHandler
{
	public:
	BreakSignalHandler()
	{
		breakreceived=false;
		std::cerr << "Installing signal handler..." << std::endl;
		signal(SIGINT, &sighandler);
	}
	inline bool TestBreak()	// Returns true if break has been received
	{
		std::cerr << "Checking break signal..." << std::endl;
		return(breakreceived);
	}
	protected:
	static void sighandler(int sig);
	static bool breakreceived;
};

extern BreakSignalHandler BreakHandler;

#endif

