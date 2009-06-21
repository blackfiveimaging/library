#ifndef THREADEVENT_H
#define THREADEVENT_H

#include "thread.h"
#include "rwmutex.h"

class ThreadEvent;
class ThreadEventHandler : public PTMutex
{
	public:
	ThreadEventHandler();
	~ThreadEventHandler();
	// If you're going to access this from multiple threads, surround calls to these
	// functions with an ObtainMutex() / ReleaseMutex() pair.
	ThreadEvent *FirstEvent();
	ThreadEvent *FindEvent(const char *eventname);
	protected:
	ThreadEvent *firstevent;
	friend class ThreadEvent;
};


class ThreadEvent_Subscriber;
class ThreadEvent : public RWMutex
{
	public:
	ThreadEvent(ThreadEventHandler &header,const char *eventname);
	~ThreadEvent();
	ThreadEvent *NextEvent();
	const char *GetName();
	void Trigger();
	void Wait();
	// Use this if you want to block other threads from responding to the
	// event.  Simply call ReleaseMutex() on the returned ThreadCondition to
	// unblock other threads.
	ThreadCondition &WaitAndHold();
	// If you want to know whether the signal's been triggered, but not wait, use this:
	int Query();
	// If you want to wait but only if the signal hasn't already been triggered, use this:
	int QueryAndWait();
	ThreadCondition &QueryWaitAndHold();

	// To use the query functions you must "Subscribe" a given thread to the Event.
	void Subscribe();
	void Unsubscribe();

	ThreadEvent_Subscriber *FindSubscriber();
	protected:
	ThreadEventHandler &header;
	ThreadEvent *nextevent,*prevevent;
	ThreadCondition cond;
	char *name;
	ThreadEvent_Subscriber *firstsubscriber;
	friend class ThreadEvent_Subscriber;
};

#endif

