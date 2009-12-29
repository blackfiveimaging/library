#ifndef PRINTOUTPUT_H
#define PRINTOUTPUT_H

#include "support/configdb.h"
#include "support/consumer.h"
#include "printerqueueswrapper.h"

#define DEFAULT_PRINTER_DRIVER "ps2"

class PrintOutput : public ConfigDB, public PrinterQueues
{
	public:
	PrintOutput(ConfigFile *inif,const char *section);
	Consumer *GetConsumer(const char *extendedopts=NULL);
	void DBToQueues();
	void QueuesToDB();
	private:
	static ConfigTemplate Template[];
	char *str2;
};

#endif
