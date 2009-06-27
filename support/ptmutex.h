/* Wrapper around pthread mutices */

#ifndef PTMUTEX_H
#define PTMUTEX_H

#include <glib.h>

// #if defined HAVE_LIBPTHREAD || defined HAVE_LIBPTHREADGC2

#include <pthread.h>

class PTMutex
{
	public:
	PTMutex();
	virtual ~PTMutex();
	virtual void ObtainMutex();
	virtual bool AttemptMutex();
	virtual void ReleaseMutex();
	protected:
	pthread_mutex_t mutex;
	friend class Thread;
};

#endif

