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
		PTMutex::Lock lock(header.mutex);
		id=Thread::GetThreadID();
		prevsub=header.firstsubscriber;
		if(prevsub)
		{
			while(prevsub->nextsub)
				prevsub=prevsub->nextsub;
			prevsub->nextsub=this;
		}
		else
			header.firstsubscriber=this;
	}
	~ThreadEvent_Subscriber()
	{
		PTMutex::Lock lock(header.mutex);
		if(nextsub)
			nextsub->prevsub=prevsub;
		if(prevsub)
			prevsub->nextsub=nextsub;
		else
			header.firstsubscriber=nextsub;
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
	Debug[TRACE] << "In ThreadEventHandler's destructor" << endl;
	PTMutex::Lock lock(*this);
	while(firstevent)
		delete firstevent;
	Debug[TRACE] << "ThreadEventHandler's destructor completed" << endl;
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
	PTMutex::Lock lock(header);
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
}


ThreadEvent::~ThreadEvent()
{
	// Detach from the header's queue while holding the header's mutex.
	{
		PTMutex::Lock lock(header);
		if(nextevent)
			nextevent->prevevent=prevevent;
		if(prevevent)
			prevevent->nextevent=nextevent;
		else
			header.firstevent=nextevent;
	}

	// Now remove any subscribers while holding an exclusive lock on the ThreadEvent
	PTMutex::Lock lock(mutex);

	while(firstsubscriber)
		delete firstsubscriber;

	if(name)
		free(name);
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
	PTMutex::Lock eventlock(mutex);
	Debug[TRACE] << "Got event mutex" << endl;
	ThreadEvent_Subscriber *sub=firstsubscriber;
	while(sub)
	{
		Debug[TRACE] << "Incrementing subscriber trigger count" << endl;
		sub->Increment();
		sub=sub->NextSubscriber();
	}

	PTMutex::Lock condlock(cond);
	Debug[TRACE] << "Obtained trigger Mutex" << endl;
	cond.Broadcast();
	Debug[TRACE] << "Sent signal - releasing" << endl;
}


int ThreadEvent::Query()
{
	int result=0;
	PTMutex::Lock lock(mutex);
	ThreadEvent_Subscriber *sub=FindSubscriber();
	if(sub)
	{
		result=sub->GetCount();
		Debug[TRACE] << "Subscriber count is " << result << endl;
		sub->Clear();
	}
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

	mutex.ObtainMutex();
	// The event which woke us up should have incremented the subscriber's count, so clear it.
	if((sub=FindSubscriber()))
		sub->Clear();
	mutex.ReleaseMutex();

	return(1);
}


ThreadCondition &ThreadEvent::QueryWaitAndHold()
{
	// Must keep mutex held between query and wait.
	int result=0;
	{
		PTMutex::Lock lock(mutex);

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
			return(cond);

		cond.ObtainMutex();
	}

	// If there is no subscriber, or if it received no events, we wait.
	cond.WaitCondition();

	// The event which woke us up should have incremented the subscriber's count, so clear it.
	ThreadEvent_Subscriber *sub=FindSubscriber();
	if(sub)
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
	PTMutex::Lock lock(mutex);
	Debug[TRACE] << "ThreadEvent - searching for existing subscriber..." << endl;
	if(!FindSubscriber())
		new ThreadEvent_Subscriber(*this);
	Debug[TRACE] << "ThreadEvent - Releasing mutex" << endl;
}


void ThreadEvent::Unsubscribe()
{
	PTMutex::Lock lock(mutex);
	ThreadEvent_Subscriber *sub=FindSubscriber();
	if(sub)
		delete sub;
}


ThreadEvent_Subscriber *ThreadEvent::FindSubscriber()
{
	RWMutex::SharedLock lock(mutex);
	ThreadID id=Thread::GetThreadID();
	ThreadEvent_Subscriber *sub=firstsubscriber;
	while(sub)
	{
		if(sub->GetID()==id)
		{
			return(sub);
		}
		sub=sub->NextSubscriber();
	}
	return(NULL);
}


