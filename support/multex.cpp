#include <iostream>

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"

#include "thread.h"
#include "multex.h"

using namespace std;

// pthreads-based implementation

Multex::Multex(int limit) : PTMutex(), lockcount(0), limit(limit)
{
	static int ser=0;
	serialno=++ser;		// Giving each multex a serial number makes debugging easier.
	for(int i=0;i<MULTEX_THREADS_MAX;++i)
	{
		counttable[i].id=0;
		counttable[i].count=0;
	}
	pthread_cond_init(&cond,0);
}


Multex::~Multex()
{
}


// ObtainMultex - obtains the multex if the thread limit has not yet been reached, 
// waits if this thread doesn't already have a lock and no further threads are allowed.

void Multex::ObtainMultex()
{
//	Debug[TRACE] << "Multex " << serialno << ": Obtaining from " << long(Thread::GetThreadID()) << endl;
	PTMutex::ObtainMutex();
	while(lockcount>=limit)
	{
		pthread_cond_wait(&cond,&mutex);
	}
	Increment();
//	Dump();
	PTMutex::ReleaseMutex();
}


bool Multex::AttemptMultex()
{
//	Debug[TRACE] << "Multex " << serialno << ": AttemptMutex from " << long(Thread::GetThreadID()) << endl;
	bool result=false;
	PTMutex::ObtainMutex();
	if(lockcount<limit)
	{
//		Debug[TRACE] << "Obtained multex lock - lock count: " << lockcount << endl;
		Increment();
//		Dump();
		result=true;
	}
	PTMutex::ReleaseMutex();
	return(result);
}


void Multex::ReleaseMultex()
{
//	Debug[TRACE] << "Multex " << serialno << ": ReleaseMutex from " << long(Thread::GetThreadID()) << endl;
	PTMutex::ObtainMutex();
	Decrement();
//	Dump();
	pthread_cond_broadcast(&cond);
	PTMutex::ReleaseMutex();
}


void Multex::SetThreadLimit(int l)
{
	limit=l;
}


void Multex::Increment()
{
	++lockcount;
	// Find the current thread in the table and increment its count
#ifdef WIN32
	void *current=pthread_self().p;
#else
	pthread_t current=pthread_self();
#endif
//	Debug[TRACE] << "Increment - searching for thread: " << current << endl;
	for(int i=0;i<MULTEX_THREADS_MAX;++i)
	{
		if(counttable[i].id==current)
		{
//			Debug[TRACE] << "found" << endl;
			++counttable[i].count;
			return;
		}
	}
	// If none found, add a new entry, and set the count to 1;
//	Debug[TRACE] << "Increment - searching for free slot" << endl;
	while(1)
	{
		for(int i=0;i<MULTEX_THREADS_MAX;++i)
		{
			if(counttable[i].id==0)
			{
//				Debug[TRACE] << "found" << endl;
				counttable[i].id=current;
				counttable[i].count=1;
				return;
			}
		}
		// If there were no free slots, complain.
		Debug[WARN] << "Multex: thread table full - waiting..." << endl;
		pthread_cond_wait(&cond,&mutex);
		Debug[WARN] << "Multex - trying again to find a free slot... " << endl;
	}
}


void Multex::Decrement(bool shared)
{
	--lockcount;
#ifdef WIN32
	void *current=pthread_self().p;
#else
	pthread_t current=pthread_self();
#endif
//	Debug[TRACE] << "Decrement - searching for thread: " << current << endl;
	for(int i=0;i<MULTEX_THREADS_MAX;++i)
	{
		if(counttable[i].id==current)
		{
			--counttable[i].count;
			if(counttable[i].count==0)
				counttable[i].id=0;
			return;
		}
	}
	// If thread was not found, complain.
//	Debug[TRACE] << "Multex: thread not found in table." << endl;
}


void Multex::Dump()
{
	Debug[TRACE] << "Locks held: " << lockcount << endl;
	for(int i=0;i<MULTEX_THREADS_MAX;++i)
	{
		if(counttable[i].id)
			Debug[TRACE] << "Thread: " << counttable[i].id << ", count: " << counttable[i].count << endl;
	}
}

