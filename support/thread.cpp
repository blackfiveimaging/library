#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "thread.h"

using namespace std;

bool Thread::TestBreak()
{
	bool result=false;
	cond.ObtainMutex();
	if(state==THREAD_CANCELLED)
		result=true;
	cond.ReleaseMutex();
	return(result);
}


void *Thread::LaunchStub(void *ud)
{
	Thread *t=(Thread *)ud;
	t->threadmutex.ObtainMutex();
	t->cond.ObtainMutex();
	t->state=THREAD_RUNNING;
	t->cond.ReleaseMutex();
	if(t->threadfunc)
		t->returncode=t->threadfunc->Entry(*t);
	t->cond.ObtainMutex();
	t->state=THREAD_FINISHED;
	t->cond.ReleaseMutex();
	t->threadmutex.ReleaseMutex();
	return(NULL);
}


void Thread::Start()
{
	returncode=0;
	state=THREAD_STARTED;
	pthread_create(&thread,0,LaunchStub,this);
}


void Thread::Stop()
{
	cond.ObtainMutex();
	state=THREAD_CANCELLED;
	cond.ReleaseMutex();
//	pthread_join(thread,&discarded);
//	state=THREAD_IDLE;
}


void Thread::SendSync()
{
	cond.Broadcast();
}


void Thread::WaitSync()
{
	cond.Wait();
}


int Thread::WaitFinished()
{
	void *discarded=NULL;
	pthread_join(thread,&discarded);
	state=THREAD_IDLE;
	return(returncode);
}


int Thread::GetReturnCode()
{
	return(returncode);
}


bool Thread::TestFinished()
{
	bool result=threadmutex.AttemptMutex();
	if(result)
	{
		if(state!=THREAD_IDLE)
		{
			void *discarded;
			pthread_join(thread,&discarded);
			state=THREAD_IDLE;
		}
		threadmutex.ReleaseMutex();
	}
	return(result);
}


Thread::~Thread()
{
	if(!TestFinished())
		WaitFinished();
}


ThreadID Thread::GetThreadID()
{
#ifdef WIN32
	return(pthread_self().p);
#else
	return(pthread_self());
#endif
}

Thread::Thread(ThreadFunction *threadfunc)
	: threadfunc(threadfunc), synced(false)
{
	pthread_attr_init(&attr);
}


ThreadCondition::ThreadCondition() : PTMutex()
{
	pthread_cond_init(&cond,0);
}

ThreadCondition::~ThreadCondition()
{
	pthread_cond_destroy(&cond);
}

void ThreadCondition::Broadcast()
{
	pthread_cond_broadcast(&cond);
}

void ThreadCondition::Wait()
{
	pthread_cond_wait(&cond,&mutex);
}

