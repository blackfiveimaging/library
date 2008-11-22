#include <iostream>

#include <string.h>

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
			if(strcmp(colname,name)==0)
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


DeviceNColorant::DeviceNColorant(DeviceNColorantList &header,const char *name)
	: red(0), green(0), blue(0), header(header), name(NULL), next(NULL), prev(NULL)
{
	if(strcasecmp(name,"Cyan")==0)
	{
		red=(0); green=(190); blue=(255);
	}
	else if(strcasecmp(name,"Magenta")==0)
	{
		red=(255); green=(0); blue=(190);
	}
	else if(strcasecmp(name,"Yellow")==0)
	{
		red=(255); green=(255); blue=(0);
	}
	else if(strcasecmp(name,"Black")==0)
	{
		red=(0); green=(0); blue=(0);
	}
	else if(strcasecmp(name,"Photo Black")==0)
	{
		red=(0); green=(0); blue=(0);
	}
	else if(strcasecmp(name,"Matte Black")==0)
	{
		red=(0); green=(0); blue=(0);
	}
	else if(strcasecmp(name,"Light Cyan")==0)
	{
		red=(127); green=(220); blue=(255);
	}
	else if(strcasecmp(name,"Light Magenta")==0)
	{
		red=(255); green=(127); blue=(220);
	}
	else if(strcasecmp(name,"Light Black")==0)
	{
		red=(127); green=(127); blue=(127);
	}
	else if(strcasecmp(name,"Light Light Black")==0)
	{
		red=(191); green=(191); blue=(191);
	}
	else if(strcasecmp(name,"Medium Black")==0)
	{
		red=(63); green=(63); blue=(63);
	}
	else if(strcasecmp(name,"Red")==0)
	{
		red=(255); green=(0); blue=(0);
	}
	else if(strcasecmp(name,"Blue")==0)
	{
		red=(0); green=(0); blue=(255);
	}
	else
	{
		cerr << "Can't find colorant: " << name << endl;
		throw "Colorant not recognised";
	}
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

