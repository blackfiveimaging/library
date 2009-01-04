#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "../support/util.h"
#include "devicencolorant.h"


using namespace std;


DeviceNColorantList::DeviceNColorantList() : first(NULL)
{
}


DeviceNColorantList::~DeviceNColorantList()
{
	while(first)
		delete first;
}


int DeviceNColorantList::GetColorantCount()
{
	int result=0;
	DeviceNColorant *c=first;
	while(c)
	{
		++result;
		c=c->NextColorant();
	}
	return(result);
}


DeviceNColorant *DeviceNColorantList::FirstColorant()
{
	return(first);
}


int DeviceNColorantList::GetColorantIndex(const char *name)
{
	if(!name)
		return(-1);
	DeviceNColorant *col=FirstColorant();
	int result=0;
	while(col)
	{
		const char *colname;
		if((colname=col->GetName()))
		{
			if(StrcasecmpIgnoreSpaces(colname,name)==0)
				return(result);
		}		
		++result;
		col=col->NextColorant();
	}
	return(-1);
}


DeviceNColorant *DeviceNColorantList::operator[](int idx)
{
	DeviceNColorant *result=FirstColorant();
	while(idx && result)
	{
		result=result->NextColorant();
		--idx;
	}
	return(result);
}


/////////////////////////////////////////////////////////////////////////////

struct colorantdefinition
{
	const char *name;
	int red,green,blue;
};

static struct colorantdefinition colorantdefinitions[]=
{
	{"Cyan",0,190,255},
	{"Magenta",255,0,190},
	{"Yellow",255,255,0},
	{"Black",0,0,0},
	{"Photo Black",0,0,0},
	{"Matte Black",0,0,0},
	{"Light Cyan",127,220,255},
	{"Light Magenta",255,127,220},
	{"Light Black",127,127,127},
	{"Light Light Black",191,191,191},
	{"Medium Black",63,63,63},
	{"Dark Yellow",160,140,0},
	{"Red",255,0,0},
	{"Blue",0,0,255},
	{"Green",0,255,0},
	{"Orange",255,128,0},
	{NULL,0,0,0}
};


DeviceNColorant::DeviceNColorant(DeviceNColorantList &header,const char *name)
	: red(0), green(0), blue(0), header(header), name(NULL), next(NULL), prev(NULL)
{
	struct colorantdefinition *c=colorantdefinitions;
	while(c->name)
	{
		if(StrcasecmpIgnoreSpaces(name,c->name)==0)
		{
			if(name)
				this->name=strdup(name);
			red=c->red;
			green=c->green;
			blue=c->blue;

			prev=header.first;
			if(prev)
			{
				while(prev->next)
					prev=prev->next;
				prev->next=this;
			}
			else
				header.first=this;

			return;
		}
		++c;
	}
	cerr << "Can't find colorant: " << name << endl;
	throw "Colorant not recognised";
}


DeviceNColorant::DeviceNColorant(DeviceNColorantList &header,const char *name,int r,int g, int b)
	: red(r), green(g), blue(b), header(header), name(NULL), next(NULL), prev(NULL)
{
	if(name)
		this->name=strdup(name);
	prev=header.first;
	if(prev)
	{
		while(prev->next)
			prev=prev->next;
		prev->next=this;
	}
	else
		header.first=this;
}


DeviceNColorant::~DeviceNColorant()
{
	if(name)
		free(name);
	if(prev)
		prev->next=next;
	else
		header.first=next;
	if(next)
		next->prev=prev;
}


const char *DeviceNColorant::GetName()
{
	return(name);
}


DeviceNColorant *DeviceNColorant::NextColorant()
{
	return(next);
}


DeviceNColorant *DeviceNColorant::PrevColorant()
{
	return(prev);
}


#ifdef STANDALONETEST

int main(int argc,char **argv)
{
	DeviceNColorantList list;

	try
	{
		new GPColorant(list,"Yellow");
		new GPColorant(list,"Cyan");
		new GPColorant(list,"Magenta");
		new GPColorant(list,"Sky blue pink");
	}
	catch(const char *err)
	{
		cerr << "Caught error: " << err << endl;
	}
	cerr << "Have " << list.GetColorantCount() << " colorants" << endl;

	cerr << "Name of colorant 2: " << list[2]->GetName() << endl;


	DeviceNColorant *c=list.FirstColorant();
	while(c)
	{
		cerr << c->GetName() << endl;
		c=c->NextColorant();
	}

	return(0);
}

#endif

