#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "thread.h"

// #if defined HAVE_LIBPTHREAD || defined HAVE_LIBPTHREADGC2

bool Thread::TestBreak()
{
	bool result=false;
	statemutex.ObtainMutex();
	if(state==THREAD_CANCELLED)
		result=true;
	statemutex.ReleaseMutex();
	return(result);
}


void *Thread::LaunchStub(void *ud)
{
	Thread *t=(Thread *)ud;
	t->threadmutex.ObtainMutex();
	t->statemutex.ObtainMutex();
	t->state=THREAD_RUNNING;
	t->statemutex.ReleaseMutex();
	if(t->entry)
		t->returncode=t->entry(t,t->userdata);
	t->statemutex.ObtainMutex();
	t->state=THREAD_IDLE;
	t->statemutex.ReleaseMutex();
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
	statemutex.ObtainMutex();
	state=THREAD_CANCELLED;
	statemutex.ReleaseMutex();
//	pthread_join(thread,&discarded);
//	state=THREAD_IDLE;
}


void Thread::SendSync()
{
	statemutex.ObtainMutex();
	pthread_cond_broadcast(&cond);
	synced=true;
	statemutex.ReleaseMutex();
}


void Thread::WaitSync()
{
	statemutex.ObtainMutex();
	if(!synced)
	{
		pthread_cond_wait(&cond,&statemutex.mutex);
	}
	synced=false;
	statemutex.ReleaseMutex();
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
		threadmutex.ReleaseMutex();
	return(result);
}


Thread::~Thread()
{
	if(!TestFinished())
		WaitFinished();
//		Stop();
}


Thread::Thread(int (*entry)(Thread *t,void *ud),void *userdata)
	: entry(entry), userdata(userdata), synced(false)
{
	pthread_attr_init(&attr);
//	pthread_attr_setschedpolicy(&tc->attr,SCHED_FIFO);
	pthread_cond_init(&cond,0);
}

// #endif

