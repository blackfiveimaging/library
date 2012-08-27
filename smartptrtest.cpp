#include <iostream>
#include <cstring>
#include <memory>
#include <cstdlib>

#include "config.h"

#include "debug.h"
#include "thread.h"

using namespace std;


#include <iostream>
#include <map>

#include "ptmutex.h"


// Want to be able to do this:

// SmartPtr<ImageSource> is(ISLoadImage(filename);

// ImageSource_Montage mon(is);	// Now have a local variable.

// is=new ImageSource_Flatten(mon);	// generates a non-deleting smart-pointer since a reference rather than a pointer was provided.
// (Dangerous!  Must (a) track lifetimes, and (b) be very careful to )

// RefCountedPtr<ImageSource> is2=is;	// original pointer is Nulled again as RefCountedPtr adopts it.

// is=new ImageSource_Tee(is2);	// ImageSource_Tee now holds a ref, so you can chain off both is and is2.





enum DeletionSemantics {DELETION_NONE,DELETION_FREE,DELETION_DELETE,DELETION_DELETEARRAY};

#if 0

// Generic smart pointer base class

class SmartPtrBase
{
	public:
	SmartPtrBase(void *p=NULL,DeletionSemantics semantics=DELETION_DELETE) : ptr(p), semantics(semantics)
	{
	}
	~SmartPtrBase()
	{
	}
	operator bool()	// Alows use of expressions such as "if(smartptr) ..."
	{
		return(ptr);
	}
	bool operator==(const SmartPtrBase &other)	// Avoids problem whereby the above bool value would be used for comparisons!
	{
		return(other.ptr==ptr);
	}
	protected:
	void *ptr;
	DeletionSemantics semantics;
};


template <class X> class SmartPtr : public SmartPtrBase
{
	public:
	explicit SmartPtr(X *p = NULL) : SmartPtrBase(p)
	{
		acquire(p);
	}
	virtual ~SmartPtr()
	{
		if(ptr)
			release();
	}
	protected:
	virtual void acquire(void *&p,DeletionSemantics newsem=DELETION_DELETE)
	{
		ptr=p;
		if(semantics!=newsem)
		{
			Debug[ERROR] << "SmartPtr::acquire() - deletion semantics mismatch!" << std::endl;
		}
		p=NULL;
	}

	virtual void release()
	{
		if(ptr)
		{
			switch(semantics)
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
		}
	}

	// Untemplated copy constructor and assignment operators - used when
	// the other smart pointer is to the same type of object.
	SmartPtr(const SmartPtr &r) : SmartPtrBase(NULL)
	{
		Debug[TRACE] << "In untemplated copy constructor" << std::endl;
		acquire(r.ptr);
		r.ptr=NULL; // We "steal" the pointer from the other object, leaving it NULL.
	}

	SmartPtr &operator=(const SmartPtr &r)
	{
		if(ptr != r.ptr)
		{
			release();
			acquire(r.ptr);
			r.ptr=NULL;
		}
		return(*this);
	}

	SmartPtr &operator=(X *p)
	{
		release();
		acquire(p);
		return(*this);
	}

	// Templated copy constructor and assignment operators - used when
	// the other smart pointer is to a derivative class of the one this pointer points to.
	template <class Y> SmartPtr(const SmartPtr<Y>& r) : SmartPtrBase(NULL)
	{
		Debug[TRACE] << "In templated copy constructor" << std::endl;
		acquire(r.ptr);
	}

	template <class Y> SmartPtr& operator=(const SmartPtr<Y>& r)
	{
		// Compare actual pointers rather than this and r, to avoid polymorphism problems.
		// Technically catches situations other than simple self-assignment but the end result
		// is the same.
		if (ptr!=r.ptr)
		{
			release();
			acquire(r.ptr,r.semantics);
        }
		return *this;
	}

	template <class Y> SmartPtr &operator=(Y *p)
    {
		// We make a temporary ptr and assign that, to achieve type-safety
		SmartPtr<Y> tmp(p);
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

	private:
};

#endif


template <class X> class RefCountPtr;


// We use a void * type here so the actual counter doesn't have to be templated
// which makes comparisons for equality much easier.
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

	// Adopt the object pointed to by an auto_ptr.  This allows programs to use the cheaper auto_ptr when refcounting's
	// not needed while still having an easy path to promote the pointer to refcounting if need be.
	template <class Y> RefCountPtr& operator=(auto_ptr<Y>& r)
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
						Debug[TRACE] << "Deletion semantics: none" << std::endl;
						break;
					case DELETION_FREE:
						Debug[TRACE] << "Deletion semantics: free" << std::endl;
						free(ptr);
						break;
					case DELETION_DELETE:
						Debug[TRACE] << "Deletion semantics: delete" << std::endl;
						delete (X *)ptr;
						break;
					case DELETION_DELETEARRAY:
						Debug[TRACE] << "Deletion semantics: delete[]" << std::endl;
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




template<> RefCountPtr<char>::RefCountPtr(char *p) : RefCountPtrBase(p) // allocate a new counter
{
	Debug[TRACE] << "In char * variant constructor" << std::endl;
	if(p)
		acquire(p,DELETION_FREE);
}

PTMutex RefCountPtrBase::mutex;
std::map<void *,RefCountPtr_Counter> RefCountPtrBase::map;




class Class1
{
	public:
	Class1(int val) : val(val)
	{
		std::cout << "Constructing class1" << std::endl;
	}
	virtual ~Class1()
	{
		std::cout << "Deleting class1" << std::endl;
	}
	virtual void DoStuff()
	{
		std::cout << "In Class1::DoStuff - val: " << val << std::endl;
	}
	protected:
	int val;
};

class Class2 : public Class1
{
	public:
	Class2() : Class1(42)
	{
		std::cout << "Constructing class2" << std::endl;
		val2=val*3;
	}
	~Class2()
	{
		std::cout << "Destructing class2" << std::endl;
	}
	virtual void DoStuff()
	{
		std::cout << "In Class2::DoStuff - val: " << val << ", val2: " << val2 << std::endl;
	}
	protected:
	int val2;
};


class TestThread : public ThreadFunction, public Thread
{
	public:
	TestThread(RefCountPtr<Class1 *> ptr) : ThreadFunction(), Thread(this), ptr(ptr)
	{
		Start();
	}
	virtual ~TestThread()
	{
	}
	virtual int Entry(Thread &t)
	{
		t.SendSync();
		{
			std::cerr << "Sub-thread about to sleep..." << std::endl;
#ifdef WIN32
			Sleep(500);
#else
			usleep(500000);
#endif
		}
		if(ptr)
			ptr->DoStuff();
		std::cerr << "Thread finished sleeping - exiting" << std::endl;
		return(0);
	}
	protected:
	RefCountPtr<Class1>ptr;
};

RefCountPtr<Class1> MakeObject(int param)
{
	return(RefCountPtr<Class1>(new Class1(param)));
}

int main(int argc,char **argv)
{
	Debug.SetLevel(TRACE);

	auto_ptr<Class2> ptr1(new Class2);
	ptr1->DoStuff();
#if 1

//	RefCountPtr<Class2> ptr1(new Class2);
	RefCountPtr<Class1> ptr3=MakeObject(20);
	{
		const char *tmp="Hello World";
		RefCountPtr<char>testchar(strdup(tmp));
		Class2 localobject;
		RefCountPtr<Class1> ptr2(localobject);
		ptr2=ptr1;

		RefCountPtr<Class1> ptr4(ptr3);
//		TestThread thread(ptr1);
		if(&*ptr1)
			ptr1->DoStuff();
		ptr2->DoStuff();
		ptr3->DoStuff();
		ptr4->DoStuff();
//		std::cout << "ptr1==ptr2: " << (ptr1==ptr2) << std::endl;
//		std::cout << "ptr1==ptr3: " << (ptr1==ptr3) << std::endl;
		std::cout << "Leaving scope - one pointer will be removed..." << std::endl;
		ptr4=NULL;
	}
	if(&*ptr1)
		ptr1->DoStuff();
	std::cout << "Terminating program..." << std::endl;
#endif

	return(0);
}

