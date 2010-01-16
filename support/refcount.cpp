#include <iostream>

#include "refcount.h"
#include "debug.h"

using namespace std;

RefCount::RefCount() : refcount(1)
{
}

RefCount::~RefCount()
{
	Debug[TRACE] << "In RefCount destructor" << endl;
}

void RefCount::ObtainRefMutex()
{
	Debug[TRACE] << "In ObtainRefMutex" << endl;
	refmutex.ObtainMutex();
	Debug[TRACE] << "Obtained" << endl;
}

void RefCount::ReleaseRefMutex()
{
	refmutex.ReleaseMutex();
}

void RefCount::Ref()
{
	ObtainRefMutex();
	++refcount;
	Debug[TRACE] << "Ref: count is " << refcount << endl;
	ReleaseRefMutex();
}

void RefCount::UnRef()
{
	ObtainRefMutex();
	--refcount;

	Debug[TRACE] << "UnRef: count is " << refcount << endl;

	ReleaseRefMutex();

	if(refcount==0)
	{
		Debug[TRACE] << "UnRef: deleting object" << endl;
		delete this;
	}
	Debug[TRACE] << "UnRef complete" << endl;
}

PTMutex RefCount::refmutex;


