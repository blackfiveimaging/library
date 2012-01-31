#ifndef ERRORDIALOGQUEUE_H
#define ERRORDIALOGQUEUE_H

// ErrorDialogQueue provides a thread-safe method of presenting a simple error message dialog to the user.
// Because on Win32 all GTK+ calls are supposed to happen from the main thread, the global ErrorDialogs object
// queues up error messages, and also queues a GTK timeout handler, which when triggers displays all queued messages
// in a single dialog.

// Use like this:

//	#include "errordialogqueue.h"
//	...
// 	ErrorDialogs.SetParent(window);	// optional
//	ErrorDialogs.AddMessage("My error messages...");
//
//	The message will be displayed the next time the main thread enters the gtk_main loop.
//
//  FIXME - add mutex protection for multi-threaded use.


#include <deque>
#include <string>

#include "gtkstub.h"

#include "ptmutex.h"

using namespace std;

class ErrorMessageQueue : public PTMutex
{
	public:
	ErrorMessageQueue();
	virtual ~ErrorMessageQueue();
	virtual void AddMessage(const char *message);
	virtual std::string GetMessages();	// Returns a string with all currently queued messages concatenated, newline separated
	protected:
	std::deque<std::string> messages;
};


class ErrorDialogQueue : public ErrorMessageQueue
{
	public:
	ErrorDialogQueue(GtkWidget *parent=NULL);
	virtual void SetParent(GtkWidget *newparent);
	virtual ~ErrorDialogQueue();
	virtual void AddMessage(const char *message);
	protected:
	static gboolean displaymessages(gpointer ud);
	GtkWidget *parent;
};

extern ErrorDialogQueue ErrorDialogs;

#endif

