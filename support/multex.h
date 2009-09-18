// Multex - a variant of Mutex but which allows a limited number
// of threads to hold a lock.

#ifndef MULTEX_H
#define MULTEX_H

#include "ptmutex.h"

// #if defined HAVE_LIBPTHREAD || defined HAVE_LIBPTHREADGC2

#include <pthread.h>

// Use a fairly small thread table - a better option would be to create
// a proper dynamic array for this, but that's overkill for an app
// like PhotoPrint where there's unlikely to be more than 1 main thread
// and 1 helper thread for each image on the page.

#define MULTEX_THREADS_MAX 20

class Multex : public PTMutex
{
	public:
	Multex(int threadlimit);
	virtual ~Multex();
	virtual void ObtainMultex();
	virtual bool AttemptMultex();
	virtual void ReleaseMultex();
	virtual void SetThreadLimit(int limit);
	protected:
	void Increment();
	void Decrement(bool shared=false);
	void Dump();
	int lockcount;
	int limit;
	int serialno;
#ifdef WIN32
	struct {void *id; int count;} counttable[MULTEX_THREADS_MAX];
#else
	struct {pthread_t id; int count;} counttable[MULTEX_THREADS_MAX];
#endif
	pthread_cond_t cond;
};


#endif

