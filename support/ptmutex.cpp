#include <iostream>

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"

using namespace std;

using namespace std;

#include "ptmutex.h"

// #if defined HAVE_LIBPTHREAD || defined HAVE_LIBPTHREADGC2

// pthreads-based implementation

PTMutex::PTMutex()
{
	pthread_mutexattr_t pmi;

	pthread_mutexattr_init(&pmi);
	pthread_mutexattr_settype(&pmi,PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&mutex,&pmi);
}


PTMutex::~PTMutex()
{
	pthread_mutex_destroy(&mutex);
}


void PTMutex::ObtainMutex()
{
	pthread_mutex_lock(&mutex);
}


bool PTMutex::AttemptMutex()
{
	int result=pthread_mutex_trylock(&mutex);
	if(result==0)
		return(true);
	else
		return(false);
}


void PTMutex::ReleaseMutex()
{
	pthread_mutex_unlock(&mutex);
}

// #else
#if 0
// Dummy implementation.  Obtaining the mutex always succeeds.

PTMutex::PTMutex()
{
	Debug[TRACE] << "Warning - building a dummy mutex" << endl;
}


PTMutex::~PTMutex()
{
}


void PTMutex::ObtainMutex()
{
	Debug[TRACE] << "Warning - obtaining a dummy mutex" << endl;
}


bool PTMutex::AttemptMutex()
{
	Debug[TRACE] << "Warning - attempting a dummy mutex" << endl;
	return(true);
}


void PTMutex::ReleaseMutex()
{
	Debug[TRACE] << "Warning - releasing a dummy mutex" << endl;
}


#endif

