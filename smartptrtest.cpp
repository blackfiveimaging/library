#include <iostream>

#include "refcountptr.h"


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


int main(int argc,char **argv)
{
	RefCountPtr<Class2> ptr1(new Class2);
	{
		RefCountPtr<Class1> ptr2;
		ptr2=ptr1;
		ptr1->DoStuff();
		ptr2->DoStuff();
		std::cout << "Leaving scope - one pointer will be removed..." << std::endl;
	}	
	ptr1->DoStuff();
	std::cout << "Terminating program..." << std::endl;
	return(0);
}

