#ifndef CYCLIC_BUFFER_H
#define CYCLIC_BUFFER_H

// Cyclic buffer - a class for handling the successive use of several fixed size buffers in a thread-synchronised manner.
// The Cyclic buffer consists of three or more buffers, linked in a circular list, so a client application can use them like so:
// (There must be at least one more buffer than there are threads accessing the circular buffer, because each buffer's lock
// is acquired before the previous one is released.)
//
// CyclicBuffer Cyclicbuffer;
// Cyclicbuffer.AddBuffer(new CyclicBuffer_Instance(1024));
// Cyclicbuffer.AddBuffer(new CyclicBuffer_Instance(1024));
// Cyclicbuffer.AddBuffer(new CyclicBuffer_Instance(1024));	// We now have a Cyclic buffer with three fixed size buffers.
//
// CyclicBuffer_Instance *rb=Cyclicbuffer.Start();
// while(condition)
// {
//    Do stuff with buffer...
//    rb=rb->NextBuffer();
// }
// rb->Finish;
//
// Provided an application sticks to this pattern, thread-synchronisation is handled automatically, so you can
// have one thread writing to the buffer, and another reading from it. To do this safely, follow these steps:
// * Hold a lock on the third (or last) buffer,
// * Start the writing thread, pausing the main thread until the writing thread has obtained the first buffer.
// * Start the reading thread.
// * Release the lock on the last buffer.


class CyclicBuffer;

class CyclicBuffer_Instance : public PTMutex
{
	public:
	CyclicBuffer_Instance(int bufsize) : buf(NULL), bufsize(bufsize)
	{
		buf=new char[bufsize];
	}
	~CyclicBuffer_Instance()
	{
		PTMutex::Lock lock(*this);
		if(buf)
			delete[] buf;
	}
	char *GetBuffer()
	{
		return(buf);
	}
	int GetBufSize()
	{
		return(bufsize);
	}

	// Handles takeover from this buffer to the next, including locking.
	CyclicBuffer_Instance *NextBuffer()
	{
		// We obtain the lock for the next buffer before releasing this one
		next->ObtainMutex();
		ReleaseMutex();
		return(next);
	}
	void Finish()
	{
		ReleaseMutex();
	}
	protected:
	void Chain(CyclicBuffer_Instance *next)
	{
		this->next=next;
	}
	char *buf;
	int bufsize;
	CyclicBuffer_Instance *next;
	friend class CyclicBuffer;
};


class CyclicBuffer
{
	public:
	CyclicBuffer() : first(NULL)
	{
	}
	virtual ~CyclicBuffer()
	{
		// Since the list is circular, we use the pointer equalling the first pointer as
		// a terminating condition.

		CyclicBuffer_Instance *ptr=first;
		while(ptr)
		{
			CyclicBuffer_Instance *ptr2=ptr->next;
			delete ptr;
			if(ptr2==first)
				ptr=NULL;
			else
				ptr=ptr2;
		}

	}
	virtual void AddBuffer(CyclicBuffer_Instance *buf)
	{
		PTMutex::Lock lock(*buf);
		CyclicBuffer_Instance *ptr=first;
		while(ptr)
		{
			CyclicBuffer_Instance *ptr2=ptr->next;
			if(ptr2==first)
			{
				// Since the list is circular, we use the pointer equalling the first pointer as
				// a terminating condition.
				PTMutex::Lock lock(*ptr);
				ptr->Chain(buf);
				buf->Chain(first);
				ptr=NULL;
			}
			else
				ptr=ptr2;
		}
		if(!first)
		{
			buf->Chain(buf);
			first=buf;
		}
	}
	virtual CyclicBuffer_Instance *Start()
	{
		first->ObtainMutex();
		return(first);
	}
	protected:
	CyclicBuffer_Instance *first;
};

#endif

