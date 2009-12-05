#ifndef ERRORDIALOGQUEUE_H
#define ERRORDIALOGQUEUE_H

#include <deque>
#include <string>

#include <gtk/gtk.h>

#include "miscwidgets/generaldialogs.h"

#include "support/ptmutex.h"

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
	virtual ~ErrorDialogQueue();
	virtual void SetParent(GtkWidget *newparent);
	virtual void AddMessage(const char *message);
	protected:
	static gboolean displaymessages(gpointer ud);
	GtkWidget *parent;
};

extern ErrorDialogQueue ErrorDialogs;

#endif

