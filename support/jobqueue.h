#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <iostream>
#include <queue>
#include <list>

#include "support/debug.h"
#include "support/thread.h"
#include "support/ptmutex.h"

class Worker;

enum JobStatus {JOBSTATUS_QUEUED,JOBSTATUS_RUNNING,JOBSTATUS_CANCELLED,JOBSTATUS_COMPLETED,JOBSTATUS_UNKNOWN};

class Job
{
	public:
	Job() : jobstatus(JOBSTATUS_UNKNOWN)
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
	virtual JobStatus GetJobStatus()
	{
		return(jobstatus);
	}
	virtual void SetJobStatus(JobStatus s)
	{
//		Debug[TRACE] << "Job::SetJobStatus(" << s << ")" << endl;
		jobstatus=s;
	}
	virtual void CancelJob()
	{
		jobstatus=JOBSTATUS_CANCELLED;
	}
	protected:
	JobStatus jobstatus;
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
		DeleteCompleted();
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

//		Debug[TRACE] << "JobQueue::Dispatch() - Obtaining mutex" << endl;

		ObtainMutex();
		if(waiting.empty())
		{
			ReleaseMutex();
			return(false);
		}

//		Debug[TRACE] << "JobQueue::Dispatch() - Getting first job" << endl;

		Job *j=waiting.front();

		// Transfer the job to the "running" list
		waiting.pop_front();
		j->SetJobStatus(JOBSTATUS_RUNNING);
		running.push_back(j);

		// Run the job - without mutex held...
		ReleaseMutex();
		j->Run(worker);

		// Now get the mutex again and remove the job from the "running" list.
		// and move it to the "completed" list, from where it can be safely deleted.
		ObtainMutex();
		running.remove(j);

		completed.push_back(j);
		if(j->GetJobStatus()==JOBSTATUS_RUNNING)	// Don't set status to COMPLETED unless it's currently RUNNING.
			j->SetJobStatus(JOBSTATUS_COMPLETED);	// - don't want to change CANCELLED to COMPLETED.

		ReleaseMutex();
		return(true);
	}

	virtual void AddJob(Job *job)
	{
		ObtainMutex();

		completed.remove(job);	// FIXME - is this legal if the job's not on the list?

		job->SetJobStatus(JOBSTATUS_QUEUED);
		waiting.push_back(job);
		Broadcast();
		ReleaseMutex();
		DeleteCompleted();
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
		if(status!=JOBSTATUS_UNKNOWN)
			job->SetJobStatus(JOBSTATUS_CANCELLED);
		ReleaseMutex();
		return(status);
	}

	// DONE - if the job self-destructs, it's possible for it to do so between this function
	// determining that the job is running, and this function returning.

	// Proposed fix:  Disallow self-destruction - instead, transfer completed jobs to a new
	// queue, and delete from there.  DONE

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

		it=completed.begin();
		while(it!=completed.end())
		{
			if(*it==job)
				return(JOBSTATUS_COMPLETED);
			++it;
		}

		return(JOBSTATUS_UNKNOWN);
	}

	// If your jobs need to be deleted from a specific thread,
	// call this function from that thread.
	void DeleteCompleted()
	{
		ObtainMutex();
		while(completed.size())
		{
			Job *j=completed.front();
			completed.remove(j);
			if(j)
			{
				delete j;
			}
		}
		ReleaseMutex();
	}

	// NOTE - Must hold the mutex while using this function.
	virtual int JobCount()
	{
		return(waiting.size());
	}
	protected:
	std::list<Job *> waiting;
	std::list<Job *> running;
	std::list<Job *> completed;
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
			while(queue.JobCount()==0)
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

