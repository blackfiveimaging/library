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
	cond1.ObtainMutex();
	if(state==THREAD_CANCELLED)
		result=true;
	cond1.ReleaseMutex();
	return(result);
}


void *Thread::LaunchStub(void *ud)
{
	Thread *t=(Thread *)ud;
	t->subthreadid=GetThreadID();
	t->threadmutex.ObtainMutex();
	t->cond1.ObtainMutex();
	t->state=THREAD_RUNNING;
	t->cond1.ReleaseMutex();
	if(t->threadfunc)
		t->returncode=t->threadfunc->Entry(*t);
	t->cond1.ObtainMutex();
	t->state=THREAD_FINISHED;
	t->cond1.ReleaseMutex();
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
	cond1.ObtainMutex();
	state=THREAD_CANCELLED;
	cond1.ReleaseMutex();
//	pthread_join(thread,&discarded);
//	state=THREAD_IDLE;
}


void Thread::SendSync()
{
	if(GetThreadID()==subthreadid)
		cond2.Broadcast();
	else
		cond1.Broadcast();
}


void Thread::WaitSync()
{
	if(GetThreadID()==subthreadid)
		cond1.WaitCondition();
	else
		cond2.WaitCondition();
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
	: threadfunc(threadfunc)
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

void ThreadCondition::WaitCondition()
{
	pthread_cond_wait(&cond,&mutex);
}

