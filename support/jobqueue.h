#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <iostream>
#include <queue>

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


class JobQueue : public PTMutex
{
	public:
	JobQueue() : PTMutex()
	{
	}
	~JobQueue()
	{
		ObtainMutex();
		Job *j;
		while((j=Pop()))
			delete j;
	}
	Job *Pop()
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
		Job *j=Pop();
		if(j)
		{
			j->Run();
			delete j;
			return(true);
		}
		return(false);
	}
	void Push(Job *job)
	{
		ObtainMutex();
		myqueue.push(job);
		ReleaseMutex();
	}
	// NOTE - Must hold the mutex while using this function.
	bool IsEmpty()
	{
		return(myqueue.empty());
	}
	protected:
	std::queue<Job *> myqueue;
};

#endif

