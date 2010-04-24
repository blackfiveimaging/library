#include <iostream>
#include <cstring>
#include <cstdlib>

#include "debug.h"

#include "threadevent.h"

using namespace std;

class ThreadEvent_Subscriber
{
	public:
	ThreadEvent_Subscriber(ThreadEvent &header) : count(0), header(header), nextsub(NULL), prevsub(NULL)
	{
		id=Thread::GetThreadID();
		header.mutex.ObtainMutex();
		prevsub=header.firstsubscriber;
		if(prevsub)
		{
			while(prevsub->nextsub)
				prevsub=prevsub->nextsub;
			prevsub->nextsub=this;
		}
		else
			header.firstsubscriber=this;
		header.mutex.ReleaseMutex();
	}
	~ThreadEvent_Subscriber()
	{
		header.mutex.ObtainMutex();
		if(nextsub)
			nextsub->prevsub=prevsub;
		if(prevsub)
			prevsub->nextsub=nextsub;
		else
			header.firstsubscriber=nextsub;
		header.mutex.ReleaseMutex();
	}
	ThreadEvent_Subscriber *NextSubscriber()
	{
		return(nextsub);
	}
	int GetCount()
	{
		return(count);
	}
	void Clear()
	{
		count=0;
	}
	void Increment()
	{
		++count;
	}
	ThreadID GetID()
	{
		return(id);
	}
	bool operator==(const ThreadID other)
	{
		return(other==id);
	}
	protected:
	int count;
	ThreadID id;
	ThreadEvent &header;
	ThreadEvent_Subscriber *nextsub,*prevsub;
};


ThreadEventHandler::ThreadEventHandler() : PTMutex(), firstevent(NULL)
{
}


ThreadEventHandler::~ThreadEventHandler()
{
	ObtainMutex();
	while(firstevent)
		delete firstevent;
	ReleaseMutex();
}


ThreadEvent *ThreadEventHandler::FirstEvent()
{
	return(firstevent);
}


ThreadEvent *ThreadEventHandler::FindEvent(const char *name)
{
	ThreadEvent *result=firstevent;
	while(result)
	{
		if(strcmp(name,result->GetName())==0)
			return(result);
		result=result->NextEvent();
	}
	return(result);
}

//------------------------------------


ThreadEvent::ThreadEvent(ThreadEventHandler &header,const char *eventname)
	: mutex(), header(header), nextevent(NULL), prevevent(NULL), name(NULL), firstsubscriber(NULL)
{
	header.ObtainMutex();
	if(eventname)
		name=strdup(eventname);

	prevevent=header.FirstEvent();
	if(prevevent)
	{
		while(prevevent->nextevent)
			prevevent=prevevent->nextevent;
		prevevent->nextevent=this;
	}
	else
		header.firstevent=this;

	header.ReleaseMutex();
}


ThreadEvent::~ThreadEvent()
{
	// Detach from the header's queue while holding the header's mutex.
	header.ObtainMutex();
	if(nextevent)
		nextevent->prevevent=prevevent;
	if(prevevent)
		prevevent->nextevent=nextevent;
	else
		header.firstevent=nextevent;
	header.ReleaseMutex();

	// Now remove any subscribers while holding an exclusive lock on the ThreadEvent
	mutex.ObtainMutex();
	while(firstsubscriber)
		delete firstsubscriber;

	if(name)
		free(name);

	mutex.ReleaseMutex();
}


ThreadEvent *ThreadEvent::NextEvent()
{
	return(nextevent);
}


const char *ThreadEvent::GetName()
{
	return(name);
}


void ThreadEvent::WaitEvent()
{
	WaitAndHold().ReleaseMutex();
}


ThreadCondition &ThreadEvent::WaitAndHold()
{
	cond.ObtainMutex();
	cond.WaitCondition();
	return(cond);
}


void ThreadEvent::Trigger()
{
	Debug[TRACE] << "Triggering event..." << endl;
	mutex.ObtainMutex();
	Debug[TRACE] << "Got event mutex" << endl;
	ThreadEvent_Subscriber *sub=firstsubscriber;
	while(sub)
	{
		Debug[TRACE] << "Incrementing subscriber trigger count" << endl;
		sub->Increment();
		sub=sub->NextSubscriber();
	}

	cond.ObtainMutex();
	Debug[TRACE] << "Obtained trigger Mutex" << endl;
	cond.Broadcast();
	Debug[TRACE] << "Sent signal - releasing" << endl;
	cond.ReleaseMutex();
	mutex.ReleaseMutex();
	Debug[TRACE] << "Released event Mutex" << endl;
}


int ThreadEvent::Query()
{
	int result=0;
	mutex.ObtainMutex();
	ThreadEvent_Subscriber *sub=FindSubscriber();
	if(sub)
	{
		result=sub->GetCount();
		Debug[TRACE] << "Subscriber count is " << result << endl;
		sub->Clear();
	}
	mutex.ReleaseMutex();
	return(result);
}


int ThreadEvent::QueryAndWait()
{
	int result=0;

	// Must keep mutex held between query and wait.

	mutex.ObtainMutex();

	// Do we have a subscriber for this thread?
	ThreadEvent_Subscriber *sub=FindSubscriber();
	if(sub)
	{
		result=sub->GetCount();
		Debug[TRACE] << "QueryAndWait: Subscriber count is " << result << endl;
		sub->Clear();
	}

	// If the subscriber's event count is non-zero we release the mutex and return the count
	if(result)
	{
		mutex.ReleaseMutex();
		return(result);
	}

	// If there is no subscriber, or if it received no events, we wait.
	cond.ObtainMutex();
	mutex.ReleaseMutex();
	cond.WaitCondition();
	cond.ReleaseMutex();

	// The event which woke us up should have incremented the subscriber's count, so clear it.
	if((sub=FindSubscriber()))
		sub->Clear();

	return(1);
}


ThreadCondition &ThreadEvent::QueryWaitAndHold()
{
	// Must keep mutex held between query and wait.
	int result=0;
	mutex.ObtainMutex();

	cond.ObtainMutex();

	// Do we have a subscriber for this thread?
	ThreadEvent_Subscriber *sub=FindSubscriber();
	if(sub)
	{
		result=sub->GetCount();
		Debug[TRACE] << "QueryAndWait: Subscriber count is " << result << endl;
		sub->Clear();
	}

	// If the subscriber's event count is non-zero we release the mutex and return the count
	if(result)
	{
		mutex.ReleaseMutex();
		return(cond);
	}

	mutex.ReleaseMutex();

	// If there is no subscriber, or if it received no events, we wait.
	cond.WaitCondition();

	// The event which woke us up should have incremented the subscriber's count, so clear it.
	if((sub=FindSubscriber()))
		sub->Clear();

	return(cond);
}


void ThreadEvent::Release()
{
	cond.ReleaseMutex();
}


void ThreadEvent::Subscribe()
{
	Debug[TRACE] << "ThreadEvent - obtaining mutex" << endl;
	mutex.ObtainMutex();
	Debug[TRACE] << "ThreadEvent - searching for existing subscriber..." << endl;
	if(!FindSubscriber())
		new ThreadEvent_Subscriber(*this);
	Debug[TRACE] << "ThreadEvent - Releasing mutex" << endl;
	mutex.ReleaseMutex();
}


void ThreadEvent::Unsubscribe()
{
	mutex.ObtainMutex();
	ThreadEvent_Subscriber *sub=FindSubscriber();
	if(sub)
		delete sub;
	mutex.ReleaseMutex();
}


ThreadEvent_Subscriber *ThreadEvent::FindSubscriber()
{
	mutex.ObtainMutexShared();
	ThreadID id=Thread::GetThreadID();
	ThreadEvent_Subscriber *sub=firstsubscriber;
	while(sub)
	{
		if(sub->GetID()==id)
		{
			mutex.ReleaseMutex();
			return(sub);
		}
		sub=sub->NextSubscriber();
	}
	mutex.ReleaseMutex();
	return(NULL);
}


