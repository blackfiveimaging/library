#include <iostream>
#include <cstring>

#include "config.h"

#include "support/progressthread.h"
#include "support/threadevent.h"

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
		thread.WaitFinished();
	}
	virtual int Entry(Thread &t)
	{
		cerr << "Thread " << t.GetThreadID() << " initializing" << endl;

		ThreadCondition &cond=event.WaitAndHold();

		cerr << "Thread " << t.GetThreadID() << " pausing..." << endl;

#ifdef WIN32
			Sleep(1000);
#else
			sleep(1);
#endif

		cond.ReleaseMutex();
		cerr << "Thread " << t.GetThreadID() << " exiting" << endl;
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
		thread.WaitFinished();
	}
	virtual int Entry(Thread &t)
	{
		cerr << "Thread " << t.GetThreadID() << " initializing" << endl;

		event.Trigger();

		cerr << "Thread " << t.GetThreadID() << " exiting" << endl;
		return(0);
	}
	protected:
	ThreadEvent &event;
	Thread thread;
};


int main(int argc, char **argv)
{
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

class TestThread1 : public ThreadFunction
{
	public:
	TestThread1(RWMutex &mutex) : ThreadFunction(), mutex(mutex)
	{
		cerr << "Creating TestThread1" << endl;
	}
	virtual ~TestThread1()
	{
		cerr << "Disposing of TestThread1" << endl;
	}
	virtual int Entry(Thread &t)
	{
		cerr << "Subthread attempting write lock" << endl;
		if(mutex.AttemptMutex())
		{
			cerr << "Obtained write-lock" << endl;
			mutex.ReleaseMutex();
		}
		else
			cerr << "Subthread Can't get write lock - read lock held elsewhere?" << endl;
		mutex.ObtainMutexShared();
		cerr << "Got shared mutex" << endl;

		cerr << "Sub-thread about to sleep..." << endl;
		ProgressThread p(t);
		t.SendSync();
		for(int i=0;i<100;++i)
		{
#ifdef WIN32
			Sleep(20);
#else
			usleep(20000);
#endif
			if(!p.DoProgress())
			{
				cerr << "Thread cancelled - exiting" << endl;
				mutex.ReleaseMutex();
				return(0);
			}
		}

		cerr << "Woken up - attempting exclusive lock (with shared lock still held)" << endl;
		mutex.ObtainMutex();
		cerr << "Subthread got exclusive lock" << endl;
		mutex.ObtainMutexShared();
		cerr << "Subthread got shared lock (with exclusive lock still held)" << endl;

		mutex.ReleaseMutex();
		mutex.ReleaseMutex();

		t.SendSync();
#ifdef WIN32
			Sleep(5000);
#else
			sleep(5);
#endif
		mutex.ReleaseMutex();
		cerr << "Thread finished sleeping - exiting" << endl;
	//	cerr << "Sub-thread ID: " << (long)pthread_self() << endl;
		return(0);
	}
	protected:
	RWMutex &mutex;
};


int main(int argc, char **argv)
{
	RWMutex mutex;

	mutex.ObtainMutexShared();	// Get a non-exclusive lock...

	cerr << "Got shared lock" << endl;

	TestThread1 tt1(mutex);
	Thread t(&tt1);
	t.Start();
	t.WaitSync();

	mutex.ReleaseMutex();

	t.WaitSync();

	mutex.ObtainMutexShared();

#if 0
	while(!t.TestFinished())
	{
		cerr << "Thread still running - sleeping for 1 second" << endl;
		sleep(1);
		cerr << "Sending break..." << endl;
		t.Stop();
	}
#endif
	t.WaitFinished();

	cerr << "Done" << endl;

	return(0);
}

#endif

