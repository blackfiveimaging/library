// Refcounting smart pointer.

// TODO:
// protect counters with mutex
// create a global list of counters, so multiple smart pointers can safely be created from regular pointers


#ifndef REFCOUNTPTR_H
#define REFCOUNTPTR_H


class RefCountPtr_Counter
{
	public:
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


template <class X> class RefCountPtr
{
	public:
    typedef X element_type;

    explicit RefCountPtr(X* p = 0) // allocate a new counter
        : refcounter(0)
	{
		if(p)
			refcounter=new RefCountPtr_Counter(p);
	}
    ~RefCountPtr()
	{
		release();
	}
    RefCountPtr(const RefCountPtr &r)
    {
		acquire(r.refcounter);
	}
    RefCountPtr &operator=(const RefCountPtr &r)
    {
		if (this != &r)
		{
			release();
			acquire(r.refcounter);
		}
		return *this;
    }

    template <class Y> RefCountPtr(const RefCountPtr<Y>& r)
    {
		acquire(r.refcounter);
	}

    template <class Y> RefCountPtr& operator=(const RefCountPtr<Y>& r)
    {
		// Compare refcounters rather than this and r, to avoid polymorphism problems.
        if (refcounter!=r.refcounter) {
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

	RefCountPtr_Counter *refcounter;
	private:
	void acquire(RefCountPtr_Counter* c) throw()
	{ // increment the count
		std::cerr << "Acquiring reference" << std::endl;
		refcounter = c;
		if(c)
			++(*c);
	}

	void release()
	{
		// decrement the count, delete if it is 0
		std::cerr << "Releasing ptr..." << std::endl;
		if (refcounter)
		{
			if (--(*refcounter)==0)
			{
				std::cerr << "Refcount zero, deleting..." << std::endl;
				delete (X *)refcounter->ptr;
				delete refcounter;
			}
			refcounter = 0;
		}
	}
};

#endif // RefCountPtr_H

