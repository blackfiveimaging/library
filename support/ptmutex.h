/* Wrapper around pthread mutices */

#ifndef PTMUTEX_H
#define PTMUTEX_H

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

	// Lock class within the PTMutex namespace.  Use like this:
	// { // Enter critical section
	//   PTMutex::Lock lock(mutex);
	//   ...
	// }
	// thus the lock will be automatically released as the scope ends, even if
	// an exception is thrown within.

	class Lock
	{
		public:
		Lock(PTMutex &mutex,bool immediate=true) : mutex(mutex), locked(false)
		{
			if(immediate)
			{
				mutex.ObtainMutex();
				locked=true;
			}
		}
		~Lock()
		{
			if(locked)
				mutex.ReleaseMutex();
		}
		bool Attempt()
		{
			return(locked=mutex.AttemptMutex());
		}
		protected:
		PTMutex &mutex;
		bool locked;
	};

	protected:
	pthread_mutex_t mutex;
	friend class Thread;
};

#endif

