#include <iostream>

#include "configdb.h"


class MyClassThatNeedsConfigData : public ConfigDB
{
	public:
	MyClassThatNeedsConfigData(ConfigFile *myfile)
		: ConfigDB(Template)
	{
		new ConfigDBHandler(myfile,"[SectionName]",this);
		// (This object is owned by the ConfigFile and will be freed by it.)
	}
	void MemberFunction()
	{
		std::cout << "Int value: " << FindInt("AnIntValue") << std::endl;
	}
	private:
	static ConfigTemplate Template[];
};


class MyOtherClass : public ConfigDB
{
	public:
	MyOtherClass(ConfigFile *myfile)
		: ConfigDB(Template)
	{
		new ConfigDBHandler(myfile,"[SectionName]",this);
	}
	void MemberFunction()
	{
		std::cout << "Int value: " << FindInt("AnIntValue") << std::endl;
	}
	MyOtherClass &operator=(MyClassThatNeedsConfigData &other)
	{
		this->ConfigDB::operator=(other);
		return(*this);
	}
	private:
	static ConfigTemplate Template[];
};


ConfigTemplate MyClassThatNeedsConfigData::Template[]=
{
	ConfigTemplate("AnIntValue",int(17)), // Default value is 17
	ConfigTemplate("AStringValue","Default"),
	ConfigTemplate("AFloatValue",float(3.5)),
	ConfigTemplate() // NULL terminated...
};


ConfigTemplate MyOtherClass::Template[]=
{
	ConfigTemplate("AnIntValue",int(13)), // Default value is 17
	ConfigTemplate("AStringValue","DefString"),
	ConfigTemplate("AFloatValue",float(1.3)),
	ConfigTemplate() // NULL terminated...
};


int main(int argc,char **argv)
{
	ConfigFile myconfig;
	MyClassThatNeedsConfigData myclass(&myconfig);
//	myconfig.ParseConfigFile("/path/to/myconfigfile");

	myclass.MemberFunction();
	std::cout << "String value: " << myclass.FindString("AStringValue") << std::endl;

	myclass.SetFloat("AFloatValue",1.45);
	
//	myconfig.SaveConfigFile("/path/to/myconfigfile");

	ConfigFile dummy;
	MyOtherClass otherclass(&dummy);

	otherclass.MemberFunction();

	std::cout << "Copying data from myclass to otherclass..." << std::endl;
	otherclass=myclass;
	otherclass.MemberFunction();

	return(0);
}


