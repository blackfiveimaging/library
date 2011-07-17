// Replacement for pthread rwlocks - subclass of PTMutex
// ObtainMutex() and AttemptMutex() behave like a regular mutex,
// obtaining an exclusive (write) lock.
// Use ObtainShared() or AttemptShared() to get a read-lock.
// This differs from pthread rwlocks in that you can obtain
// an exclusive lock even if you already have a shared one,
// and vice versa, provided no other threads hold one.
// Some pthreads implementations behave like this, but not all,
// and not the one in Linux.

// Another difference is that this implementation tracks the
// exclusive attribute in a stack-like manner - so the call sequence
// ObtainMutex(); ObtainMutexShared(); ReleaseMutex(); ReleaseMutex()
// will leave the mutex in exclusive mode until the second release.

// ObtainMutexShared(), ObtainMutex(), ObtainMutexShared(),
// ReleaseMutex(), ReleaseMutex(), ReleaseMutex()
// Will promote the mutex to Exclusive mode at the ObtainMutex()
// call, and revert to shared mode after the second ReleaseMutex().

#ifndef RWMUTEX_H
#define RWMUTEX_H

#include "ptmutex.h"

// #if defined HAVE_LIBPTHREAD || defined HAVE_LIBPTHREADGC2
#include <vector>
#include <pthread.h>

// Use a fairly small thread table - a better option would be to create
// a proper dynamic array for this, but that's overkill for an app
// like PhotoPrint where there's unlikely to be more than 1 main thread
// and 1 helper thread for each image on the page.

#define RWMUTEX_THREADS_MAX 20

class RWMutex : public PTMutex
{
	public:
	RWMutex();
	virtual ~RWMutex();
	virtual void ObtainMutex();
	virtual void ObtainMutexShared();
	virtual bool AttemptMutex();
	virtual bool AttemptMutexShared();
	virtual void ReleaseMutex();
	virtual void ReleaseMutexShared();	// Not needed unless you find yourself in a situation like this:
										// AttemptShared()  ->   Obtain()  ->  Release()  ->  Release() and want
										// the lock to remain exclusive until the second Release.
	bool CheckExclusive();

	// Lock class within the RWMutex namespace.  Use like this:
	// { // Enter critical section
	//   RWMutex::Lock lock(mutex);
	//   ...
	// }
	// thus the lock will be automatically released as the scope ends, even if
	// an exception is thrown within.
	// Since RWMutex derives from PTMutex, you can use PTMutex::Lock if you only
	// need an exclusive lock.

	class SharedLock
	{
		public:
		SharedLock(RWMutex &mutex, bool immediate=true) : mutex(mutex), locked(false)
		{
			if(immediate)
			{
				mutex.ObtainMutexShared();
				locked=true;
			}
		}
		~SharedLock()
		{
			if(locked)
				mutex.ReleaseMutex();
		}
		bool Attempt()
		{
			return(locked=mutex.AttemptMutexShared());
		}
		protected:
		RWMutex &mutex;
		bool locked;
	};


	protected:
	void Increment();
	void Decrement(bool shared=false);
	void Dump();
	int lockcount;
	int exclusive;
#ifdef WIN32
	struct threadtable_entry {void *id; int count;};
#else
	struct threadtable_entry {pthread_t id; int count;};
#endif
	std::vector<threadtable_entry> counttable;
	int serialno;
	pthread_cond_t cond;
};


#endif

