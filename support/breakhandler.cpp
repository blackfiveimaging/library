#include <iostream>

#include "breakhandler.h"

void BreakSignalHandler::sighandler(int sig)
{
	std::cerr << "Break signal received within handler..." << std::endl;
	breakreceived=true;
}

bool BreakSignalHandler::breakreceived;

BreakSignalHandler BreakHandler;

