#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <iostream>
#include <queue>
#include <list>

#include "support/debug.h"
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


enum JobStatus {JOBSTATUS_QUEUED,JOBSTATUS_RUNNING,JOBSTATUS_UNKNOWN};

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
		if(waiting.empty())
		{
			ReleaseMutex();
			return(NULL);
		}
		Job *result=waiting.front();
		waiting.pop_front();
		ReleaseMutex();
		return(result);
	}
	bool Dispatch(Worker *worker=NULL)
	{
		// Must transfer the job to the running queue
		// with the mutex held.
		ObtainMutex();
		if(IsEmpty())
		{
			ReleaseMutex();
			return(false);
		}
		Job *j=waiting.front();

		// Transfer the job to the "running" list
		waiting.pop_front();
		running.push_back(j);

		// Run the job - without mutex held...
		ReleaseMutex();
		j->Run(worker);

		// Now get the mutex again and remove the job from the "running" list.
		ObtainMutex();
		running.remove(j);
		ReleaseMutex();
		return(true);
	}

	virtual void PushJob(Job *job)
	{
		ObtainMutex();
		waiting.push_back(job);
		Broadcast();
		ReleaseMutex();
	}

	// Function to cancel a queued job - returns JOBSTATUS_RUNNING
	// if the job is in progress, and JOBSTATUS_UNKNOWN if not.
	virtual JobStatus CancelJob(Job *job)
	{
		ObtainMutex();
		JobStatus status=GetJobStatus(job);
		if(status==JOBSTATUS_QUEUED)
		{
			waiting.remove(job);
			ReleaseMutex();
			delete job;
			return(JOBSTATUS_UNKNOWN);
		}
		ReleaseMutex();
		return(status);
	}

	// FIXME - if the job self-destructs, it's possible for it to do so between this function
	// determining that the job is running, and this function returning.
	// Must hold the mutex while using this function - result is no longer valid once
	// the mutex is released.
	virtual JobStatus GetJobStatus(Job *job)
	{
		std::list<Job *>::iterator it=waiting.begin();
		while(it!=waiting.end())
		{
			if(*it==job)
				return(JOBSTATUS_QUEUED);
			++it;
		}

		it=running.begin();
		while(it!=running.end())
		{
			if(*it==job)
				return(JOBSTATUS_RUNNING);
			++it;
		}

		return(JOBSTATUS_UNKNOWN);
	}

	// NOTE - Must hold the mutex while using this function.
	virtual bool IsEmpty()
	{
		return(waiting.empty());
	}

	// NOTE - Must hold the mutex while using this function.
	virtual int JobCount()
	{
		return(waiting.size());
	}
	protected:
	std::list<Job *> waiting;
	std::list<Job *> running;
};


enum WorkerThreadStatus {WORKERTHREAD_RUN,WORKERTHREAD_CANCEL,WORKERTHREAD_TERMINATE};

class Worker : public ThreadFunction, public PTMutex
{
	public:
	Worker(JobQueue &queue)
		: ThreadFunction(), PTMutex(), queue(queue), thread(this), status(WORKERTHREAD_RUN)
	{
//		Debug[TRACE] << "Starting worker thread..." << std::endl;
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
//		Debug[TRACE] << "Worker thread running..." << std::endl;
		do
		{
//			Debug[TRACE] << "Obtaining mutex" << std::endl;
			queue.ObtainMutex();
			while(queue.IsEmpty())
			{
//				Debug[TRACE] << "Waiting for a job" << std::endl;
				queue.WaitCondition();
//				Debug[TRACE] << "Signal received" << std::endl;
				if(status!=WORKERTHREAD_RUN)
				{
//					Debug[TRACE] << "Received cancellation signal" << std::endl;
					queue.ReleaseMutex();
					return(0);
				}
			}
//			Debug[TRACE] << "Releasing mutex" << std::endl;
			queue.ReleaseMutex();
//			Debug[TRACE] << "Running job" << std::endl;

			// Obtain a per-thread mutex while running the job, so the destructor can avoid a busy-wait.
			ObtainMutex();
			queue.Dispatch(this);
			ReleaseMutex();

			// Send a pulse to say the thread's adopting a new job
			queue.ObtainMutex();
			queue.Broadcast();
			queue.ReleaseMutex();
		} while(status==WORKERTHREAD_RUN);
//		Debug[TRACE] << "Cancelled" << std::endl;
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
//				Debug[TRACE] << "(" << JobCount() << " jobs remaining...)" << std::endl;
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

