#include <iostream>

#include "refcount.h"
#include "debug.h"

using namespace std;

RefCount::RefCount() : refcount(1)
{
}

RefCount::~RefCount()
{
}

void RefCount::ObtainRefMutex()
{
	refmutex.ObtainMutex();
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
}

PTMutex RefCount::refmutex;


