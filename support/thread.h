#ifndef THREADCONTEXT_H
#define THREADCONTEXT_H

#include "ptmutex.h"

enum ThreadState {THREAD_IDLE,THREAD_STARTED,THREAD_RUNNING,THREAD_CANCELLED};
class Thread;


class ThreadCondition : public PTMutex
{
	public:
	ThreadCondition();
	~ThreadCondition();
	virtual void Broadcast();	// Sends the signal - Mutex should be obtained first and released afterwards
	virtual void Wait();		// Waits for the signal - mutex should be obtained first and released afterwards
	protected:
	pthread_cond_t cond;
};


// Demonstrates how to use the ThreadCondition - used to sync between subthread and main thread.
class ThreadSync : public ThreadCondition
{
	public:
	ThreadSync() : ThreadCondition(), received(false)
	{
	}
	~ThreadSync()
	{
	}
	virtual void Broadcast()
	{
		ObtainMutex();
		ThreadCondition::Broadcast();
		received=true;
		ReleaseMutex();
	}
	virtual void Wait()
	{
		ObtainMutex();
		if(!received)
			ThreadCondition::Wait();
		received=false;
		ReleaseMutex();
	}
	protected:
	bool received;
};



#ifdef WIN32
typedef void *ThreadID;
#else
typedef pthread_t ThreadID;
#endif

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
	// Static function - can be called anywhere
	ThreadID GetThreadID();
	protected:
	static void *LaunchStub(void *ud);
	int (*entry)(Thread *t,void *ud);
	void *userdata;
	pthread_t thread;
	ThreadSync cond;
	pthread_attr_t attr;
	PTMutex threadmutex;
	bool synced;
	int returncode;
	enum ThreadState state;
	int error;
};


// Base class for thread function.  Subclass this and override Entry with your own subthread code.
class ThreadFunction
{
	public:
	ThreadFunction(Thread &t) : thread(t)
	{
	}
	virtual ~ThreadFunction()
	{
	}
	virtual int Entry()
	{
		thread.SendSync();
		return(0);
	}
	protected:
	Thread &thread;
};


#endif
