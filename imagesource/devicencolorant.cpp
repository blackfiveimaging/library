#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "devicencolorant.h"

#include "debug.h"

#include "config.h"
#include "gettext.h"

using namespace std;


DeviceNColorantList::DeviceNColorantList() : first(NULL)
{
}


DeviceNColorantList::DeviceNColorantList(IS_TYPE type) : first(NULL)
{
	switch(STRIP_ALPHA(type))
	{
		case IS_TYPE_GREY:
		case IS_TYPE_BW:
			new DeviceNColorant(*this,"Black");
			break;			
		case IS_TYPE_RGB:
			new DeviceNColorant(*this,"Red");
			new DeviceNColorant(*this,"Green");
			new DeviceNColorant(*this,"Blue");
			break;
		case IS_TYPE_CMYK:
			new DeviceNColorant(*this,"Cyan");
			new DeviceNColorant(*this,"Magenta");
			new DeviceNColorant(*this,"Yellow");
			new DeviceNColorant(*this,"Black");
			break;
		default:
			throw "Can't yet automatically construct a DeviceNColorantList for non-standard colourspaces";
			break;
	}
	if(HAS_ALPHA(type))
		new DeviceNColorant(*this,"Alpha");
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


// Create a string representation of which colorants are active,
// suitable for storing in a config file.

char *DeviceNColorantList::GetEnabledColorants()
{
	char *result=NULL;
	DeviceNColorant *col=FirstColorant();
	int len=2;  // Initial ":" and null-terminator
	while(col)
	{
		if(col->GetEnabled())
		{
			const char *name=col->GetName();
			if(name)
				len+=strlen(name)+2;
		}
		col=col->NextColorant();
	}
	result=(char *)malloc(len);
	result[0]=':';
	result[1]=0;
	col=FirstColorant();
	while(col)
	{
		if(col->GetEnabled())
		{
			const char *name=col->GetName();
			if(name)
			{
				strcat(result,name);
				strcat(result,":");
			}
		}
		col=col->NextColorant();
	}
	if(!result)
		result=strdup("");
	return(result);
}


// Given a string respresention as produced by GetEnabledColorants()
// sets the "enabled" flag in the colorants

void DeviceNColorantList::SetEnabledColorants(const char *colstr)
{
	if(colstr && strlen(colstr))
	{
		DeviceNColorant *col=FirstColorant();
		while(col)
		{
			col->Disable();
			const char *name=col->GetName();
			if(name && strlen(name))
			{
				const char *tmp=strstr(colstr,name);
				while(tmp)
				{
					if(tmp[-1]==':' && tmp[strlen(name)]==':')
					{
						col->Enable();
						tmp=NULL;
					}
					else
						tmp=strstr(tmp+1,name);
				}
			}
			col=col->NextColorant();
		}
	}
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
	{"Vivid Magenta",255,0,190},
	{"Yellow",255,255,0},
	{"Black",0,0,0},
	{"Photo Black",0,0,0},
	{"Matte Black",0,0,0},
	{"Light Cyan",127,220,255},
	{"Light Magenta",255,127,220},
	{"Vivid Light Magenta",255,127,220},
	{"Light Black",127,127,127},
	{"Light Light Black",191,191,191},
	{"Medium Black",63,63,63},
	{"Dark Yellow",160,140,0},
	{"Red",255,0,0},
	{"Blue",0,0,255},
	{"Green",0,255,0},
	{"Orange",255,128,0},
	{"Alpha",192,192,192},
	{"White",255,255,255},
	{"Gloss",240,240,240},
	{NULL,0,0,0}
};


DeviceNColorant::DeviceNColorant(DeviceNColorantList &header,const char *name,const char *displayname)
	: red(0), green(0), blue(0), header(header), enabled(true), name(NULL), displayname(NULL), next(NULL), prev(NULL)
{
	struct colorantdefinition *c=colorantdefinitions;
	if(name)
	{
		while(c->name)
		{
			if(StrcasecmpIgnoreSpaces(name,c->name)==0)
			{
				if(name)
					this->name=strdup(name);

				if(displayname)
					this->displayname=strdup(displayname);
				else
					this->displayname=strdup(gettext(name));

				red=c->red;
				green=c->green;
				blue=c->blue;

				linknode();

				return;
			}
			++c;
		}
		Debug[WARN] << "Can't find colorant: " << name << endl;
		throw "Colorant not recognised";
	}
	else
	{
		red=c->red;
		green=c->green;
		blue=c->blue;

		linknode();
	}
}


DeviceNColorant::DeviceNColorant(DeviceNColorantList &header,const char *name,const char *displayname,int r,int g, int b)
	: red(r), green(g), blue(b), header(header), enabled(true), name(NULL), displayname(NULL), next(NULL), prev(NULL)
{
	if(name)
		this->name=strdup(name);
	if(displayname)
		this->displayname=strdup(displayname);
	else
		this->displayname=strdup(gettext(name));
	linknode();
}


DeviceNColorant::~DeviceNColorant()
{
	if(name)
		free(name);
	if(displayname)
		free(displayname);
	if(prev)
		prev->next=next;
	else
		header.first=next;
	if(next)
		next->prev=prev;
}


void DeviceNColorant::linknode()
{
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


const char *DeviceNColorant::GetName()
{
	return(name);
}


const char *DeviceNColorant::GetDisplayName()
{
	return(displayname);
}


void DeviceNColorant::Enable()
{
	enabled=true;
}


void DeviceNColorant::Disable()
{
	enabled=false;
}


bool DeviceNColorant::GetEnabled()
{
	return(enabled);
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
		Debug[ERROR] << "Caught error: " << err << endl;
	}
	Debug[COMMENT] << "Have " << list.GetColorantCount() << " colorants" << endl;

	Debug[COMMENT] << "Name of colorant 2: " << list[2]->GetName() << endl;


	DeviceNColorant *c=list.FirstColorant();
	while(c)
	{
		Debug[COMMENT] << c->GetName() << endl;
		c=c->NextColorant();
	}

	return(0);
}

#endif

