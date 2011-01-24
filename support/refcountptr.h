// Refcounting smart pointer.

// TODO:
// DONE: create a global list of counters, so multiple smart pointers can safely be created from regular pointers
// Specialise char * type to use free() instead of delete, for c-string compatibility.  (Allocators?)
// Perhaps add constructor taking references, that doesn't delete - for compatibility with local objects?  (Dangerous)
// Make a smart-pointer superclass

#ifndef REFCOUNTPTR_H
#define REFCOUNTPTR_H

#include <iostream>
#include <map>
#include <memory>
#include <cstdlib>

#include "ptmutex.h"
#include "debug.h"

template <class X> class RefCountPtr;


// We use a void * type here so the actual counter doesn't have to be templated
// which makes comparisons for equality much easier.
enum DeletionSemantics {DELETION_NONE,DELETION_FREE,DELETION_DELETE,DELETION_DELETEARRAY};
class RefCountPtr_Counter
{
	public:
	template <class X> friend class RefCountPtr;
	friend class std::map<void *,RefCountPtr_Counter>;
	private:
	RefCountPtr_Counter(DeletionSemantics semantics=DELETION_DELETE,unsigned c = 0) : semantics(semantics), count(c)
	{
		Debug[TRACE] << "In RefCountptr_Counter constructor" << std::endl;
	}
	unsigned int operator--()
	{
		Debug[TRACE] << "Decrementing count from " << count << std::endl;
		return(--count);
	}
	unsigned int operator++()
	{
		Debug[TRACE] << "Incrementing count from " << count << std::endl;
		return(++count);
	}
	DeletionSemantics semantics;
	unsigned int count;
};


// Generic smart pointer base class



// Base class used to make the mutex visible to all templated variants of the RefCountPtr
// Note: the mutex protects against nothing more than data races on the count variable.

class RefCountPtrBase
{
	public:
	RefCountPtrBase(void *p=NULL) : ptr(p), count(NULL)
	{
	}
	~RefCountPtrBase()
	{
	}
	operator bool()	// Alows use of expressions such as "if(smartptr) ..."
	{
		return(ptr);
	}
	bool operator==(const RefCountPtrBase &other)	// Avoids problem whereby the above bool value would be used for comparisons!
	{
		return(other.ptr==ptr);
	}
	protected:
	void *ptr;
	RefCountPtr_Counter *count;
	static PTMutex mutex;
	static std::map<void *,RefCountPtr_Counter> map;
	template <class X> friend class RefCountPtr;
};


template <class X> class RefCountPtr : public RefCountPtrBase
{
	public:
	explicit RefCountPtr(X *p = NULL) : RefCountPtrBase(p) // allocate a new counter
	{
		if(p)
			acquire(p);
	}

	// Dangerous!  Create a pointer from a reference.
	// Deletion is ignored, so you're responsible for tracking lifetime.
	explicit RefCountPtr(X &p) : RefCountPtrBase(&p) // allocate a new counter
	{
		acquire(&p,DELETION_NONE);
	}
	~RefCountPtr()
	{
		release();
	}

	// Untemplated copy constructor and assignment operators - used when
	// the other smart pointer is to the same type of object.
	RefCountPtr(const RefCountPtr &r) : RefCountPtrBase(NULL)
	{
		Debug[TRACE] << "In untemplated copy constructor" << std::endl;
		acquire(r.ptr);
	}

    RefCountPtr &operator=(const RefCountPtr &r)
    {
		PTMutex::Lock lock(mutex);
		if(ptr != r.ptr)
		{
			release();
			acquire(r.ptr);
		}
		return(*this);
    }

    RefCountPtr &operator=(X *p)
    {
		PTMutex::Lock lock(mutex);
		release();
		acquire(p);
		return(*this);
	}

	// Templated copy constructor and assignment operators - used when
	// the other smart pointer is to a derivative class of the one this pointer points to.
	template <class Y> RefCountPtr(const RefCountPtr<Y>& r) : RefCountPtrBase(NULL)
	{
		Debug[TRACE] << "In templated copy constructor" << std::endl;
		acquire(r.ptr);
	}

	// Adopt the object pointed to by an auto_ptr.  This allows programs to use the cheaper auto_ptr when refcounting's
	// not needed while still having an easy path to promote the pointer to refcounting if need be.
	template <class Y> RefCountPtr& operator=(std::auto_ptr<Y>& r)
	{
		// Compare actual pointers rather than this and r, to avoid polymorphism problems.
		// Technically catches situations other than simple self-assignment but the end result
		// is the same.
		if (ptr!=&*r)
		{
			Y *p=r.release();
			PTMutex::Lock lock(mutex);
			release();
			if(count)
				acquire(p,count->semantics);
			else
				acquire(p);
        }
		return *this;
	}

	template <class Y> RefCountPtr& operator=(const RefCountPtr<Y>& r)
	{
		// Compare actual pointers rather than this and r, to avoid polymorphism problems.
		// Technically catches situations other than simple self-assignment but the end result
		// is the same.
		if (ptr!=r.ptr)
		{
			PTMutex::Lock lock(mutex);
			release();
			if(count)
				acquire(r.ptr,count->semantics);
			else
				acquire(r.ptr);
        }
		return *this;
	}

	template <class Y> RefCountPtr &operator=(Y *p)
    {
		// We make a temporary ptr and assign that, to achieve type-safety
		RefCountPtr<Y> tmp(p);
		*this=tmp;
		return(*this);
	}

	inline X& operator*() const
	{
		return(*(X *)ptr);
	}
	inline X* operator->() const
	{
		return((X *)ptr);
	}
	inline X* GetPtr() const
	{
		return ((X *)ptr);
	}
	inline unsigned int GetCount() const
	{
		return(count ? count->count : 0);
	}

	private:
	void acquire(void *p,DeletionSemantics semantics=DELETION_DELETE)
	{
		ptr=p;
		if(p)
		{
			// If we have a pointer, we lock the mutex, search the map for a matching
			// count record and increment its reference count if found.
			PTMutex::Lock lock(mutex);
			Debug[TRACE] << "Acquiring reference" << std::endl;
			count=&map[p];
			Debug[TRACE] << "Map element " << long(count) << std::endl;

			// Ensure the pointer doesn't already exist with conflicting semantics.
			if(count->count && (count->semantics!=semantics))
			{
				Debug[ERROR] << "RefCountPointer has conflicting deletion semantics!" << std::endl;
			}
			else
				count->semantics=semantics;

			// increment the count
			++(*count);
		}
		Debug[TRACE] << "Acquisition complete" << std::endl;
	}

	void release()
	{
		if(count)
		{
			PTMutex::Lock lock(mutex);

			// decrement the count, delete if it is 0
			Debug[TRACE] << "Releasing ptr..." << std::endl;
			if (--(*count)==0)
			{
				Debug[TRACE] << "Refcount zero, deleting..." << std::endl;
				switch(count->semantics)
				{
					case DELETION_NONE:
						break;
					case DELETION_FREE:
						free(ptr);
						break;
					case DELETION_DELETE:
						delete (X *)ptr;
						break;
					case DELETION_DELETEARRAY:
						delete[] (X *)ptr;
						break;
				}
				map.erase(ptr);
			}
			count = NULL;
			ptr = NULL;
		}
	}
};

template<> RefCountPtr<char>::RefCountPtr(char *p);

#endif // RefCountPtr_H

