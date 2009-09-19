#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <iostream>
#include <queue>

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
			delete j;
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

#endif

