#include "refcountptr.h"

PTMutex RefCountPtrBase::mutex;
std::map<void *,RefCountPtr_Counter> RefCountPtrBase::map;

