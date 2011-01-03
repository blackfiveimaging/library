#include <iostream>
#include "debug.h"
#include "thread.h"
#include "refcountptr.h"

using namespace std;

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
			Sleep(50);
#else
			usleep(50000);
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



int main(int argc,char **argv)
{
//	Debug.SetLevel(TRACE);
	RefCountPtr<Class2> ptr1(new Class2);
	RefCountPtr<Class1> ptr3(new Class1(30));
	{
		RefCountPtr<Class1> ptr2;
		ptr2=ptr1;
		RefCountPtr<Class1> ptr4(ptr1);
		TestThread thread(ptr1);
		ptr1->DoStuff();
		ptr2->DoStuff();
		ptr3->DoStuff();
		ptr4->DoStuff();
		std::cout << "ptr1==ptr2: " << (ptr1==ptr2) << std::endl;
		std::cout << "ptr1==ptr3: " << (ptr1==ptr3) << std::endl;
		std::cout << "Leaving scope - one pointer will be removed..." << std::endl;
	}	
	ptr1->DoStuff();
	std::cout << "Terminating program..." << std::endl;
	return(0);
}

