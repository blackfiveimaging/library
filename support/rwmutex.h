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
	protected:
	void Increment();
	void Decrement(bool shared=false);
	void Dump();
	int lockcount;
	int exclusive;
#ifdef WIN32
	struct {void *id; int count;} counttable[RWMUTEX_THREADS_MAX];
#else
	struct {pthread_t id; int count;} counttable[RWMUTEX_THREADS_MAX];
#endif
	int serialno;
	pthread_cond_t cond;
};


#endif

