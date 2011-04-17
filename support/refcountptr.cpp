#include "refcountptr.h"

PTMutex RefCountPtrBase::mutex;
std::map<void *,RefCountPtr_Counter> RefCountPtrBase::map;

template<> RefCountPtr<char>::RefCountPtr(char *p) : RefCountPtrBase(p) // allocate a new counter
{
	Debug[MINUTIAE] << "In char * variant constructor" << std::endl;
	if(p)
		acquire(p,DELETION_FREE);
}

