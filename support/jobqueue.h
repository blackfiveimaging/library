#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <iostream>
#include <queue>
#include <list>

#include "support/thread.h"
#include "support/ptmutex.h"

class Worker;

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
	virtual void Run(Worker *worker=NULL)
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
		ReleaseMutex();
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
	bool Dispatch(Worker *worker=NULL)
	{
		Job *j=PopJob();
		if(j)
		{
			j->Run(worker);
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


enum WorkerThreadStatus {WORKERTHREAD_RUN,WORKERTHREAD_CANCEL,WORKERTHREAD_TERMINATE};

class Worker : public ThreadFunction, public PTMutex
{
	public:
	Worker(JobQueue &queue)
		: ThreadFunction(), PTMutex(), queue(queue), thread(this), status(WORKERTHREAD_RUN)
	{
//		std::cerr << "Starting worker thread..." << std::endl;
		thread.Start();
	}
	virtual ~Worker()
	{
		WaitCompletion();
	}
	virtual void Cancel()
	{
		status=WORKERTHREAD_CANCEL;
	}
	virtual void WaitCompletion()
	{
		if(status==WORKERTHREAD_RUN)
			status=WORKERTHREAD_TERMINATE;
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
				if(status!=WORKERTHREAD_RUN)
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
			queue.Dispatch(this);
			ReleaseMutex();

			// Send a pulse to say the thread's adopting a new job
			queue.ObtainMutex();
			queue.Broadcast();
			queue.ReleaseMutex();
		} while(status==WORKERTHREAD_RUN);
//		std::cerr << "Cancelled" << std::endl;
		return(0);
	}
	protected:
	JobQueue &queue;
	Thread thread;
	WorkerThreadStatus status;
};


class JobDispatcher : public JobQueue
{
	public:
	JobDispatcher(int threads) : JobQueue()
	{
		for(int i=0;i<threads;++i)
			AddWorker(new Worker(*this));
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

		while(JobCount())
		{
//				std::cerr << "(" << JobCount() << " jobs remaining...)" << std::endl;
			WaitCondition();
		}
		ReleaseMutex();

		std::list<Worker *>::iterator it=threadlist.begin();
		while(it!=threadlist.end())
		{
			(*it)->WaitCompletion();
			++it;
		}
	}
	void AddWorker(Worker *worker)
	{
		threadlist.push_back(worker);
	}
	protected:
	std::list<Worker *> threadlist;
	friend class Worker;
};


#endif

