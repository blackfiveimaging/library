#include <iostream>

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"

#include "thread.h"
#include "rwmutex.h"

using namespace std;

// pthreads-based implementation

RWMutex::RWMutex() : PTMutex(), lockcount(0), exclusive(0)
{
	static int ser=0;
	serialno=++ser;		// Giving each mutex a serial number makes debugging easier.
	for(int i=0;i<RWMUTEX_THREADS_MAX;++i)
	{
		counttable[i].id=0;
		counttable[i].count=0;
	}
	pthread_cond_init(&cond,0);
}


RWMutex::~RWMutex()
{
	Debug[TRACE] << "RWMutex: Destructing" << endl;
}


// ObtainMutex - obtains the mutex in exclusive mode.
// Succeeds if there are no locks currently held, or if all currently held
// locks are held by the current thread.

// Once the lock is held, no other threads will be able to obtain a lock,
// either exclusive or shared.  The thread holding the lock can, however,
// obtain further locks of either exclusive or shared type.
// This differs from pthreads rwlocks - their behaviour is undefined if a
// thread holding a write-lock attempts to obtain a read-lock or vice versa.

// If a thread holds a shared lock, then obtains an exclusive lock, and
// another shared lock, the lock will remain in exclusive mode until the
// inner two locks have been released, then revert to shared mode.

void RWMutex::ObtainMutex()
{
//	Debug[TRACE] << "RWMutex " << serialno << ": ObtainMutex from " << long(Thread::GetThreadID()) << endl;
	PTMutex::ObtainMutex();
	while(!CheckExclusive())
	{
		pthread_cond_wait(&cond,&mutex);
	}
	Increment();
	if(exclusive==0)
		++exclusive;
//	Dump();
	PTMutex::ReleaseMutex();
}


// ObtainMutexShared - obtains the mutex in non-exclusive mode.
// Succeeds if there are no exclusive locks currently held

// Once the lock is held, other threads will be able to obtain shared locks,
// but not exclusive locks.  The thread holding the lock can, however, obtain
// a subsequent exclusive lock.  The lock will revert to shared mode when
// the second lock is released.
// This differs from pthreads rwlocks - their behaviour is undefined if a
// thread holding a write-lock attempts to obtain a read-lock or vice versa.

void RWMutex::ObtainMutexShared()
{
//	Debug[TRACE] << "RWMutex " << serialno << ": ObtainMutexShared from " << long(Thread::GetThreadID()) << endl;
	PTMutex::ObtainMutex();
	while((exclusive!=0) && !CheckExclusive())
	{
//		Dump();

		// We must wait until the exclusive flag is clear;
		pthread_cond_wait(&cond,&mutex);
	}
	Increment();
//	Dump();
	PTMutex::ReleaseMutex();
}


bool RWMutex::AttemptMutex()
{
//	Debug[TRACE] << "RWMutex " << serialno << ": AttemptMutex from " << long(Thread::GetThreadID()) << endl;
	bool result=false;
	PTMutex::ObtainMutex();
	if(CheckExclusive())
	{
//		Debug[TRACE] << "Obtained exclusive lock - lock count: " << lockcount << endl;
		Increment();
		if(exclusive==0)
			++exclusive;
//		Dump();
		result=true;
	}
	PTMutex::ReleaseMutex();
	return(result);
}


bool RWMutex::AttemptMutexShared()
{
//	Debug[TRACE] << "RWMutex " << serialno << ": AttemptMutexShared from " << long(Thread::GetThreadID()) << endl;
	bool result=false;
	PTMutex::ObtainMutex();
	if((exclusive==0) || (CheckExclusive()))
	{
		Increment();
//		Dump();
		result=true;
	}
	PTMutex::ReleaseMutex();
	return(result);
}


void RWMutex::ReleaseMutex()
{
//	Debug[TRACE] << "RWMutex " << serialno << ": ReleaseMutex from " << long(Thread::GetThreadID()) << endl;
	PTMutex::ObtainMutex();
	Decrement();
//	Dump();
	pthread_cond_broadcast(&cond);
	PTMutex::ReleaseMutex();
}


void RWMutex::ReleaseMutexShared()
{
//	Debug[TRACE] << "RWMutex " << serialno << ": ReleaseMutex from " << long(Thread::GetThreadID()) << endl;
	PTMutex::ObtainMutex();
	Decrement(true);
//	Dump();
	pthread_cond_broadcast(&cond);
	PTMutex::ReleaseMutex();
}


bool RWMutex::CheckExclusive()
{
	if(lockcount==0)
		return(true);
//	if(exclusive==0);
//		return(true);
	// If the lockcount is greater than zero, then we have to
	// step through the thread table making sure that the
	// current thread is the only one holding locks.
#ifdef WIN32
	void *current=pthread_self().p;
#else
	pthread_t current=pthread_self();
#endif
//	Debug[TRACE] << "Current thread: " << current << endl;
	for(int i=0;i<RWMUTEX_THREADS_MAX;++i)
	{
		// If any thread other than this one has count>0 return false;
		if(counttable[i].id!=current && counttable[i].count!=0)
			return(false);
		// If the current thread's count == the global lockcount, return true.
		if(counttable[i].id==current && counttable[i].count==lockcount)
			return(true);
	}
	// If we reached here, then the global lockcount is greater than zero, but
	// the thread table is inconsistent.  Succeed grudgingly.
	Debug[TRACE] << "RWMutex " << serialno << ": inconsistent locking data." << endl;
	return(true);
}


void RWMutex::Increment()
{
	++lockcount;
	if(exclusive)
		++exclusive;
	// Find the current thread in the table and increment its count
#ifdef WIN32
	void *current=pthread_self().p;
#else
	pthread_t current=pthread_self();
#endif
//	Debug[TRACE] << "Increment - searching for thread: " << current << endl;
	for(int i=0;i<RWMUTEX_THREADS_MAX;++i)
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
		for(int i=0;i<RWMUTEX_THREADS_MAX;++i)
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
		Debug[WARN] << "RWMutex: thread table full - waiting..." << endl;
		pthread_cond_wait(&cond,&mutex);
		Debug[WARN] << "RWMutex - trying again to find a free slot... " << endl;
	}
}


void RWMutex::Decrement(bool shared)
{
	--lockcount;
	if(exclusive && !shared)
		--exclusive;
#ifdef WIN32
	void *current=pthread_self().p;
#else
	pthread_t current=pthread_self();
#endif
//	Debug[TRACE] << "Decrement - searching for thread: " << current << endl;
	for(int i=0;i<RWMUTEX_THREADS_MAX;++i)
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
//	Debug[TRACE] << "RWMutex: thread not found in table." << endl;
}


void RWMutex::Dump()
{
	Debug[WARN] << "Locks held: " << lockcount << endl;
	Debug[WARN] << "Exclusive count: " << exclusive << endl;
	for(int i=0;i<RWMUTEX_THREADS_MAX;++i)
	{
		if(counttable[i].id)
			Debug[WARN] << "Thread: " << counttable[i].id << ", count: " << counttable[i].count << endl;
	}
}

