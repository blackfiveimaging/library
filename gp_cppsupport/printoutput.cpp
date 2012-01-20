#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

#include "../miscwidgets/generaldialogs.h"
#include "../miscwidgets/livedisplaycheck.h"

#include "../support/debug.h"

#include "printoutput.h"
#include "printoutputselector.h"


class PODBHandler : public ConfigDBHandler
{
	public:
	PODBHandler(ConfigFile *inif,const char *section,PrintOutput *po)
		: ConfigDBHandler(inif,section,po), printoutput(po)
	{
	}
	~PODBHandler()
	{
	}
	void LeaveSection()
	{
		Debug[TRACE] << "*** Leaving PrintOutput section" << endl;
		printoutput->DBToQueues();
	}
	private:
	PrintOutput *printoutput;	
};

PrintOutput::PrintOutput(ConfigFile *inif,const char *section) : ConfigDB(Template), PrinterQueues()
{
	Debug[TRACE] << "In PrintOutput constructor..." << endl;
	new PODBHandler(inif,section,this);
	const char *defaultqueue=FindString("Queue");
	if(strlen(defaultqueue)==0 && GetPrinterCount()>0)
	{
		char *queue=GetPrinterName(0);
		
		if(queue)
		{
			SetPrinterQueue(queue);
			SetString("Queue",queue);
			char *driver=GetPrinterDriver(queue);
			if(driver)
			{
				SetString("Driver",driver);
				free(driver);
			}
			free(queue);
		}
		else
			SetString("Driver",DEFAULT_PRINTER_DRIVER);
	}
	Debug[TRACE] << "Done..." << endl;
}

class Consumer_Queue : public Consumer
{
	public:
	Consumer_Queue(PrinterQueues &pq,const char *queuename,const char *extendedopts=NULL) : pq(pq)
	{
		pq.SetPrinterQueue(queuename);
		if(!pq.InitialiseJob(extendedopts))
			throw "Can't initialise!";
		pq.InitialisePage();
	}
	virtual ~Consumer_Queue()
	{
		pq.EndPage();
		pq.EndJob();
	}
	virtual bool Write(const char *buffer, int length)
	{
		return(pq.WriteData(buffer,length));
	}
	virtual void Cancel()
	{
		pq.CancelJob();
	}
	protected:
	PrinterQueues &pq;
};


Consumer *PrintOutput::GetConsumer(const char *extendedopts)
{
	const char *str;
	str=FindString("Driver");
	if(str[0]=='p' && str[1]=='s')	// Are we printing in PostScript mode?
		SetDataType(PQINFO_POSTSCRIPT);
	else
		SetDataType(PQINFO_RAW);

	if(strlen(str=FindString("Queue")))
	{
		try
		{
			Consumer *result=new Consumer_Queue(*this,str,extendedopts);	
			return(result);
		}
		catch(const char *err)
		{
			return(NULL);
		}
	}
	else
		return(NULL);
}


void PrintOutput::DBToQueues()
{
	const char *tmp=FindString("Queue");
	if(PrinterQueueExists(tmp))
		Debug[TRACE] << "Printer queue exists" << endl;
	else
	{
		Debug[TRACE] << "Warning - printer queue not found" << endl;
		try
		{
#ifdef HAVE_GTK
			printoutput_queue_dialog(this);
			tmp=FindString("Queue");
#else
			throw "No GTK!";
#endif
		}
		catch (const char *err)
		{
			// If we end up here, there's no display, so pick a "safe" queue...
			tmp=PRINTERQUEUE_SAVETOFILE;
		}
	}
	SetPrinterQueue(tmp);
	tmp=FindString("Command");
	SetCustomCommand(tmp);
}


void PrintOutput::QueuesToDB()
{
	const char *tmp=GetPrinterQueue();
	SetString("Queue",tmp);
	tmp=GetCustomCommand();
	SetString("Command",tmp);
}


ConfigTemplate PrintOutput::Template[]=
{
	ConfigTemplate("Queue",""),
	ConfigTemplate("Driver",""),
	ConfigTemplate("Command",""),
	ConfigTemplate("ResponseHash",""),
	ConfigTemplate()
};

