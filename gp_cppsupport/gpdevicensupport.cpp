#include <iostream>
#include <string.h>

#include <gutenprint/gutenprint.h>

#include "support/util.h"
#include "gpdevicensupport.h"

#include "gettext.h"
#define _(x) gettext(x)


using namespace std;


int GetPrinterColorantCount(stp_vars_t *vars)
{
	int result=0;
	stp_parameter_t desc;
	stp_describe_parameter(vars,"ChannelNames",&desc);
	stp_string_list_t *strlist=desc.bounds.str;
	if(strlist)
		result=stp_string_list_count(strlist);
	stp_parameter_description_destroy(&desc);
	return(result);
}


//////////////////////////////////////////////////////////////////////////////


GPColorantList::GPColorantList(stp_vars_t *vars) : DeviceNColorantList()
{
	BuildList(vars);
}


GPColorantList::GPColorantList(GPrinterSettings *gp) : DeviceNColorantList()
{
	BuildList(gp->stpvars);
}


GPColorantList::~GPColorantList()
{
}


// Builds a DeviceNColorantList from an stp_vars_t's
// ChannelNames variable.
// Dummy entries will be created for unknown channels,
// to retain the 1:1 mapping between colorant index and
// printer raw channels.
// Should no longer be necessary since 2009-01-04

static const char *ChannelOrder[]=
{
	"Black","Matte Black","Photo Black","Light Black","Light Light Black","Cyan","Light Cyan",
	"Magenta","Vivid Magenta","Light Magenta","Vivid Light Magenta","Yellow","Red","Blue","Gloss Optimizer",NULL
};


void GPColorantList::BuildList(stp_vars_t *vars)
{
	if(!vars)
		vars=cachedvars;
	if(!vars)
		throw "BuildList_Raw: No stp_vars provided!";
	while((*this)[0])
		delete (*this)[0];
	const char *mode=stp_get_string_parameter(vars,"InputImageType");
	if(!mode)
	{
		Debug[TRACE] << "No mode found - falling back to CMYK" << endl;
		stp_set_string_parameter(vars,"InputImageType","CMYK");
		mode="CMYK";
	}
	Debug[TRACE] << "InputImageType set to " << mode << endl;

	if(strcmp(mode,"Raw")==0)
	{
		BuildList_Raw(vars);
	}
	else if(strcmp(mode,"Grayscale")==0)
	{
		new DeviceNColorant(*this,"Black");
	}
	else if(strcmp(mode,"RGB")==0)
	{
		new DeviceNColorant(*this,"Red");
		new DeviceNColorant(*this,"Green");
		new DeviceNColorant(*this,"Blue");
	}
	else if(strcmp(mode,"CMYK")==0)
	{
		new DeviceNColorant(*this,"Cyan");
		new DeviceNColorant(*this,"Magenta");
		new DeviceNColorant(*this,"Yellow");
		new DeviceNColorant(*this,"Black");
	}
}


void GPColorantList::BuildList_Raw(stp_vars_t *vars)
{
	Debug[TRACE] << "GPColorantList - building raw list" << endl;
	if(!vars)
		vars=cachedvars;
	if(!vars)
		throw "BuildList_Raw: No stp_vars provided!";
	// Set printer to use all available colorants before asking for channel names...
	char chans[4]={0};
	sprintf(chans,"%d",GetPrinterColorantCount(vars));
	cerr << "Setting rawchannels to:" << chans << endl;
	stp_set_string_parameter(vars,"RawChannels",chans);
	stp_parameter_t desc;

	// We try to use the new RawChannelNames parameter from Gutenprint_5_2_branch
	stp_describe_parameter(vars,"RawChannelNames",&desc);
	stp_string_list_t *strlist=desc.bounds.str;
	if(strlist)
	{
		int ncol=stp_string_list_count(strlist);
		cerr << "Have " << ncol << " raw channels" << endl;
		cerr << "(Printer has " << GetPrinterColorantCount(vars) << " ink channels in total)" << endl;
		for(int j=0;j<ncol;++j)
		{
			stp_param_string_t *p=stp_string_list_param(strlist,j);
			cerr << "Have " << p->name << " : " << p->text << endl;
			try
			{
				new DeviceNColorant(*this,p->name,p->text);
			}
			catch(const char *err)
			{
				cerr << "Ignoring colorant: " << p->name << endl;
				new DeviceNColorant(*this,NULL);	// We include a dummy colorant for unrecognised
													// channels (gloss optimiser, mainly) so that
													// each colorant's position in the list continues
													// to match its raw channel number.
			}
		}
	}
	else
	{
		cerr << "Falling back to ChannelNames[]" << endl;
		stp_describe_parameter(vars,"ChannelNames",&desc);
		stp_string_list_t *strlist=desc.bounds.str;
		if(strlist)
		{
			int i=0;
			int ncol=stp_string_list_count(strlist);
			cerr << "Have " << ncol << " raw channels" << endl;
			while(ChannelOrder[i])
			{
				for(int j=0;j<ncol;++j)
				{
					stp_param_string_t *p=stp_string_list_param(strlist,j);
					if(StrcasecmpIgnoreSpaces(p->name,ChannelOrder[i])==0)
					{
						try
						{
							new DeviceNColorant(*this,p->name);
						}
						catch(const char *err)
						{
							cerr << "Ignoring colorant: " << p->name << endl;
							new DeviceNColorant(*this,NULL);	// We include a dummy colorant for unrecognised
																// channels (gloss optimiser, mainly) so that
																// each colorant's position in the list continues
																// to match its raw channel number.
						}
						j=ncol;
					}
				}
				++i;
			}
		}
	}
	stp_parameter_description_destroy(&desc);
}


#if 0
void GPColorantList::BuildList(stp_vars_t *vars)
{
	stp_parameter_t desc;
	stp_describe_parameter(vars,"ChannelNames",&desc);
	stp_string_list_t *strlist=desc.bounds.str;
	if(strlist)
	{
		int ncol=stp_string_list_count(strlist);
		cerr << "Have " << ncol << " raw channels" << endl;
		for(int j=0;j<ncol;++j)
		{
			stp_param_string_t *p=stp_string_list_param(strlist,j);
			cerr << "Ink: " << p->name << endl;
			try	
			{
				new GPColorant(*this,p->name);
			}
			catch(const char *err)
			{
				cerr << "Ignoring colorant: " << p->name << endl;
				new GPColorant(*this);	// We include a dummy colorant for unrecognised
										// channels (gloss optimiser, mainly) so that
										// each colorant's position in the list continues
										// to match its raw channel number.
			}
		}
	}
	stp_parameter_description_destroy(&desc);
}
#endif


