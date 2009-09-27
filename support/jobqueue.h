#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <iostream>
#include <queue>
#include <list>

#include "support/thread.h"
#include "support/ptmutex.h"

class Job
{
	public:
	Job()
	{
	}
	Job(Job &other)
	{
	}
	virtual ~Job()
	{
	}
	virtual void Run()
	{
	}
	protected:
};


class JobQueue : public ThreadCondition
{
	public:
	JobQueue() : ThreadCondition()
	{
	}
	~JobQueue()
	{
		ObtainMutex();
		Job *j;
		while((j=PopJob()))
			delete j;
	}
	Job *PopJob()
	{
		ObtainMutex();
		if(myqueue.empty())
		{
			ReleaseMutex();
			return(NULL);
		}
		Job *result=myqueue.front();
		myqueue.pop();
		ReleaseMutex();
		return(result);
	}
	bool Dispatch()
	{
		Job *j=PopJob();
		if(j)
		{
			j->Run();
//			delete j;		// Safer to let the job handle its own demise...
			return(true);
		}
		return(false);
	}
	virtual void PushJob(Job *job)
	{
		ObtainMutex();
		myqueue.push(job);
		Broadcast();
		ReleaseMutex();
	}

	// NOTE - Must hold the mutex while using this function.
	virtual bool IsEmpty()
	{
		return(myqueue.empty());
	}

	// NOTE - Must hold the mutex while using this function.
	virtual int JobCount()
	{
		return(myqueue.size());
	}
	protected:
	std::queue<Job *> myqueue;
};


class Worker : public ThreadFunction, public PTMutex
{
	public:
	Worker(JobQueue &queue)
		: ThreadFunction(), PTMutex(), queue(queue), thread(this), cancelled(false)
	{
//		std::cerr << "Starting worker thread..." << std::endl;
		thread.Start();
	}
	virtual ~Worker()
	{
//		std::cerr << "Destructing worker thread..." << std::endl;
		cancelled=true;
		while(!thread.TestFinished())
		{
			// We obtain this mutex to avoid a busy wait - the subthread holds it while processing a job.
			ObtainMutex();
			queue.ObtainMutex();
			queue.Broadcast();
			queue.ReleaseMutex();
			ReleaseMutex();
		}
	}
	virtual int Entry(Thread &t)
	{
//		std::cerr << "Worker thread running..." << std::endl;
		do
		{
//			std::cerr << "Obtaining mutex" << std::endl;
			queue.ObtainMutex();
			while(queue.IsEmpty())
			{
//				std::cerr << "Waiting for a job" << std::endl;
				queue.WaitCondition();
//				std::cerr << "Signal received" << std::endl;
				if(cancelled)
				{
//					std::cerr << "Received cancellation signal" << std::endl;
					queue.ReleaseMutex();
					return(0);
				}
			}
//			std::cerr << "Releasing mutex" << std::endl;
			queue.ReleaseMutex();
//			std::cerr << "Running job" << std::endl;

			// Obtain a per-thread mutex while running the job, so the destructor can avoid a busy-wait.
			ObtainMutex();
			queue.Dispatch();
			ReleaseMutex();

			// Send a pulse to say the thread's adopting a new job
			queue.ObtainMutex();
			queue.Broadcast();
			queue.ReleaseMutex();
		} while(!cancelled);
//		std::cerr << "Cancelled" << std::endl;
		return(0);
	}
	protected:
	JobQueue &queue;
	Thread thread;
	bool cancelled;
};


class JobDispatcher : public JobQueue
{
	public:
	JobDispatcher(int threads) : JobQueue()
	{
		for(int i=0;i<threads;++i)
			threadlist.push_back(new Worker(*this));
	}
	virtual ~JobDispatcher()
	{
		while(!threadlist.empty())
		{
			Worker *thread=threadlist.front();
			delete thread;
			threadlist.pop_front();
		}
	}
	virtual void WaitCompletion()
	{
		ObtainMutex();
		std::list<Worker *>::iterator it=threadlist.begin();
		while(it!=threadlist.end())
		{
			while(JobCount())
			{
//				std::cerr << "(" << JobCount() << " jobs remaining...)" << std::endl;
				WaitCondition();
			}
			++it;
		}
		ReleaseMutex();
	}
	protected:
	std::list<Worker *> threadlist;
	friend class Worker;
};


#endif

