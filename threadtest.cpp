#include <iostream>
#include <cstring>

#include "config.h"

#include "support/debug.h"
#include "support/progressthread.h"
#include "support/threadevent.h"
#include "rwmutex.h"

#include "breakhandler.h"

using namespace std;

#if 0
class TestThread : public ThreadFunction
{
	public:
	TestThread() : ThreadFunction(), thread(this)
	{
		cerr << "Starting thread" << endl;
		thread.Start();
		cerr << "Started" << endl;
	}
	virtual ~TestThread()
	{
		cerr << "Waiting for thread to finish" << endl;
		thread.WaitFinished();
		cerr << "Thread finished" << endl;
	}
	virtual int Entry(Thread &t)
	{
#ifdef WIN32
			Sleep(1000);
#else
			sleep(1);
#endif
		return(0);
	}
	protected:
	Thread thread;
};


int main(int argc, char **argv)
{
	TestThread t;
	return(0);
}
#endif


#if 1

//------------------------------------


class TestThread_WaitTH : public ThreadFunction
{
	public:
	TestThread_WaitTH(ThreadEvent &event) : ThreadFunction(), event(event), thread(this)
	{
		thread.Start();
	}
	virtual ~TestThread_WaitTH()
	{
		cerr << "Destructing WaitTH" << endl;
		thread.WaitFinished();
	}
	virtual int Entry(Thread &t)
	{
//		cerr << "Thread " << t.GetThreadID() << " initializing" << endl;

		event.WaitAndHold();

//		cerr << "Thread " << t.GetThreadID() << " pausing..." << endl;

#ifdef WIN32
			Sleep(1000);
#else
			sleep(1);
#endif

		event.Release();
//		cerr << "Thread " << t.GetThreadID() << " exiting" << endl;
		return(0);
	}
	protected:
	ThreadEvent &event;
	Thread thread;
};



class TestThread_SendTH : public ThreadFunction
{
	public:
	TestThread_SendTH(ThreadEvent &event) : ThreadFunction(), event(event), thread(this)
	{
		thread.Start();
	}
	virtual ~TestThread_SendTH()
	{
		cerr << "Destructing SendTH" << endl;
		thread.WaitFinished();
	}
	virtual int Entry(Thread &t)
	{
//		cerr << "Thread " << t.GetThreadID() << " initializing" << endl;

		event.Trigger();

//		cerr << "Thread " << t.GetThreadID() << " exiting" << endl;
		return(0);
	}
	protected:
	ThreadEvent &event;
	Thread thread;
};


int main(int argc, char **argv)
{
	Debug.SetLevel(TRACE);

	if(BreakHandler.TestBreak())
		std::cerr << "Break signal received..." << std::endl;
	else
		std::cerr << "No break signal received..." << std::endl;

	ThreadEventHandler tehandler;
	ThreadEvent e1(tehandler,"Event1");
	ThreadEvent *e2=new ThreadEvent(tehandler,"Event2");

	e1.Subscribe();	// Subscribing allows us to keep track of how many times the signal's
					// been received, and more importantly, whether it was received before we started waiting.

	TestThread_WaitTH tt1(e1);
	TestThread_WaitTH tt2(e1);
	TestThread_WaitTH tt3(e1);
	TestThread_WaitTH tt4(e1);

	cerr << "Sending signal..." << endl;
	TestThread_SendTH tt5(e1);
	cerr << "Main thread waiting..." << endl;

	e1.QueryAndWait();

	cerr << "Main thread woken" << endl;

	return(0);
}
#endif

#if 0

class TestThread1 : public ThreadFunction, public Thread
{
	public:
	TestThread1(RWMutex &mutex) : ThreadFunction(), Thread(this), mutex(mutex)
	{
		cerr << "Creating TestThread1" << endl;
	}
	virtual ~TestThread1()
	{
		cerr << "Disposing of TestThread1" << endl;
	}
	virtual int Entry(Thread &t)
	{
		{
			PTMutex::Lock lock1(mutex,false);

			Debug[TRACE] << "Subthread attempting write lock" << endl;
			if(lock1.Attempt())
			{
				Debug[TRACE] << "Obtained write-lock" << endl;
			}
			else
				Debug[TRACE] << "Subthread Can't get write lock - read lock held elsewhere?" << endl;
		}

		RWMutex::SharedLock lock2(mutex);
		Debug[TRACE] << "Subthread Got shared lock - about to sleep" << endl;

		ProgressThread p(*this);
		SendSync();
		for(int i=0;i<20;++i)
		{
#ifdef WIN32
			Sleep(20);
#else
			usleep(20000);
#endif
			if(!p.DoProgress())
			{
				Debug[TRACE] << "Thread cancelled - exiting" << endl;
				return(0);
			}
		}

		t.SendSync();
#ifdef WIN32
			Sleep(5000);
#else
			sleep(5);
#endif
		cerr << "Thread finished sleeping - exiting" << endl;
		return(0);
	}
	protected:
	RWMutex &mutex;
};


class TestThread2 : public ThreadFunction, public Thread
{
	public:
	TestThread2(RWMutex &mutex) : ThreadFunction(), Thread(this), mutex(mutex)
	{
		cerr << "Creating TestThread1" << endl;
	}
	virtual ~TestThread2()
	{
		cerr << "Disposing of TestThread1" << endl;
	}
	virtual int Entry(Thread &t)
	{
		t.SendSync();
		cerr << "Subthread attempting write lock" << endl;
		{
			RWMutex::SharedLock lock(mutex);
			cerr << "Sub-thread about to sleep..." << endl;
#ifdef WIN32
			Sleep(50);
#else
			usleep(50000);
#endif
}
		cerr << "Woken up - attempting exclusive lock (with shared lock still held)" << endl;
		cerr << "Thread finished sleeping - exiting" << endl;
		return(0);
	}
	protected:
	RWMutex &mutex;
};


int main(int argc, char **argv)
{
	Debug.SetLevel(ERROR);
	RWMutex mutex;
	TestThread1 tt1(mutex);
	{
		// Obtaining shared lock...
		Debug[TRACE] << "Obtaining shared lock from main thread..." << endl;
		RWMutex::SharedLock lock(mutex);
		Debug[TRACE] << "Starting thread..." << endl;
		tt1.Start();
		tt1.WaitSync();
		Debug[TRACE] << "Main thread received signal..." << endl;
	}
	tt1.WaitFinished();

	Debug[TRACE] << "Done" << endl;

	return(0);
}

#endif

