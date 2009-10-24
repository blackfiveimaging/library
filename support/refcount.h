#ifndef REFCOUNT_H
#define REFCOUNT_H

#include "ptmutex.h"

class RefCount
{
	public:
	RefCount();
	virtual ~RefCount();
	virtual void ObtainRefMutex();
	virtual void ReleaseRefMutex();
	virtual void Ref();
	virtual void UnRef();
	protected:
	int refcount;
	static PTMutex refmutex;	// We use a single mutex to protect all refcounts, since they'll only be held momentarily.
								// Also avoids the issue of trying to destruct a member mutex object while it's locked.
};

#endif

