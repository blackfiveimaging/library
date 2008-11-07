#ifndef THREADCONTEXT_H
#define THREADCONTEXT_H

#include "ptmutex.h"

enum ThreadState {THREAD_IDLE,THREAD_STARTED,THREAD_RUNNING,THREAD_CANCELLED};

#if defined HAVE_LIBPTHREAD || defined HAVE_LIBPTHREADGC2

class Thread
{
	public:
	Thread(int (*entry)(Thread *t,void *ud),void *UserData);
	Thread();
	~Thread();
	void Stop();
	void Start();
	void WaitSync();
	int WaitFinished();
	int GetReturnCode();
	bool TestFinished();
	// Methods to be used from within threads
	void SendSync();
	bool TestBreak();
	protected:
	static void *LaunchStub(void *ud);
	int (*entry)(Thread *t,void *ud);
	void *userdata;
	pthread_t thread;
	pthread_cond_t cond;
	pthread_attr_t attr;
	PTMutex statemutex;
	PTMutex threadmutex;
	bool synced;
	int returncode;
	enum ThreadState state;
	int error;
};


#endif

#endif
