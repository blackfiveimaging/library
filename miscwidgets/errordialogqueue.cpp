#include <iostream>

#include "debug.h"
#include "errordialogqueue.h"
#include "generaldialogs.h"
#include "livedisplaycheck.h"


ErrorMessageQueue::ErrorMessageQueue() : PTMutex()
{
}

ErrorMessageQueue::~ErrorMessageQueue()
{
}

void ErrorMessageQueue::AddMessage(const char *message)
{
	Debug[WARN] << "***Queuing error message: " << message << std::endl;
	ObtainMutex();
	messages.push_back(message);
	ReleaseMutex();
}

std::string ErrorMessageQueue::GetMessages()	// Returns a string with all currently queued messages concatenated, newline separated
{
	std::string result;
	ObtainMutex();
	while(messages.size())
	{
		result+=messages[0];
		messages.pop_front();
		if(messages.size())
			result+=std::string("\n");
	}
	ReleaseMutex();
	return(result);
}


ErrorDialogQueue::ErrorDialogQueue(GtkWidget *parent) : ErrorMessageQueue(), parent(parent)
{
}

ErrorDialogQueue::~ErrorDialogQueue()
{
}

void ErrorDialogQueue::SetParent(GtkWidget *newparent)
{
	parent=newparent;
}

void ErrorDialogQueue::AddMessage(const char *message)
{
	if(LiveDisplay.HaveDisplay())
	{
		ErrorMessageQueue::AddMessage(message);
		g_timeout_add(1,displaymessages,this);
	}
	else
		Debug[ERROR] << message << endl;
}

gboolean ErrorDialogQueue::displaymessages(gpointer ud)
{
	ErrorDialogQueue *q=(ErrorDialogQueue *)ud;
	q->ObtainMutex();
	if(q->messages.size())
	{
		std::string m=q->GetMessages();
		ErrorMessage_Dialog(m.c_str(),q->parent);
	}
	q->ReleaseMutex();
	return(FALSE);
}


ErrorDialogQueue ErrorDialogs;


