// Refcounting smart pointer.

// TODO:
// create a global list of counters, so multiple smart pointers can safely be created from regular pointers

#ifndef REFCOUNTPTR_H
#define REFCOUNTPTR_H

#include <iostream>

#include "ptmutex.h"
#include "debug.h"

template <class X> class RefCountPtr;

// We use a void * type here so the actual counter doesn't have to be templated
// which makes comparisons for equality much easier.
class RefCountPtr_Counter
{
	public:
	template <class X> friend class RefCountPtr;
	private:
	RefCountPtr_Counter(void *p = 0, unsigned c = 1) : ptr(p), count(c)
	{
	}
	unsigned int operator--()
	{
		return(--count);
	}
	unsigned int operator++()
	{
		return(++count);
	}
	void *ptr;
	unsigned int count;
};


// Base class used to make the mutex visible to all templated variants of the RefCountPtr
// Note: the mutex protects against nothing more than data races on the count variable.
class RefCountPtrBase
{
	public:
	RefCountPtrBase() : refcounter(NULL)
	{
	}
	~RefCountPtrBase()
	{
	}
	protected:
	RefCountPtr_Counter *refcounter;
	static PTMutex mutex;
	template <class X> friend class RefCountPtr;
};


template <class X> class RefCountPtr : public RefCountPtrBase
{
	public:
	explicit RefCountPtr(X *p = NULL) : RefCountPtrBase() // allocate a new counter
	{
		if(p)
			refcounter=new RefCountPtr_Counter(p);
	}
	~RefCountPtr()
	{
		release();
	}

	// Copy constructor - should never be called in practice.
	RefCountPtr(const RefCountPtr &r) : RefCountPtrBase()
	{
		PTMutex::Lock lock(mutex);
		acquire(r.refcounter);
	}

    RefCountPtr &operator=(const RefCountPtr &r)
    {
		PTMutex::Lock lock(mutex);
		if(refcounter != r.refcounter)
		{
			release();
			acquire(r.refcounter);
		}
		return *this;
    }

	template <class Y> RefCountPtr(const RefCountPtr<Y>& r) : RefCountPtrBase()
	{
		PTMutex::Lock lock(mutex);
		acquire(r.refcounter);
	}

	template <class Y> RefCountPtr& operator=(const RefCountPtr<Y>& r)
	{
		PTMutex::Lock lock(mutex);
		// Compare refcounters rather than this and r, to avoid polymorphism problems.
		if (refcounter!=r.refcounter)
		{
			release();
			acquire(r.refcounter);
        }
		return *this;
	}

	inline X& operator*() const
	{
		return(*(X *)refcounter->ptr);
	}
	inline X* operator->() const
	{
		return((X *)refcounter->ptr);
	}
	inline X* GetPtr() const
	{
		return (refcounter ? refcounter->ptr : 0);
	}
	inline unsigned int GetCount() const
	{
		return(refcounter ? refcounter->count : 0);
	}

	private:
	void acquire(RefCountPtr_Counter* c) throw()
	{
		// increment the count
		Debug[TRACE] << "Acquiring reference" << std::endl;
		refcounter = c;
		if(c)
			++(*c);
	}

	void release()
	{
		// decrement the count, delete if it is 0
		Debug[TRACE] << "Releasing ptr..." << std::endl;
		if (refcounter)
		{
			if (--(*refcounter)==0)
			{
				Debug[TRACE] << "Refcount zero, deleting..." << std::endl;
				delete (X *)refcounter->ptr;
				delete refcounter;
			}
			refcounter = NULL;
		}
	}
};

#endif // RefCountPtr_H

