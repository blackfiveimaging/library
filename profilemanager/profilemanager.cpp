#include <iostream>

#include <libgen.h>

#ifdef WIN32
#include <windows.h>
#else
#include <X11/Xatom.h>
#endif

#include "../support/debug.h"

#include "profilemanager.h"
#include "naivetransforms.h"
#include "searchpathdbhandler.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)

using namespace std;


// ProfileManager


ConfigTemplate ProfileManager::Template[]=
{
 	ConfigTemplate("DefaultRGBProfile",BUILTINSRGB_ESCAPESTRING),
	ConfigTemplate("DefaultRGBProfileActive",int(1)),
	ConfigTemplate("DefaultCMYKProfile","coated_FOGRA39L_argl.icc"),
	ConfigTemplate("DefaultCMYKProfileActive",int(1)),
	ConfigTemplate("DefaultGreyProfile",BUILTINSGREY_ESCAPESTRING),
	ConfigTemplate("DefaultGreyProfileActive",int(1)),
	ConfigTemplate("PrinterProfile",""),
	ConfigTemplate("PrinterProfileActive",int(0)),
	ConfigTemplate("ExportProfile",""),
	ConfigTemplate("ExportProfileActive",int(0)),
	ConfigTemplate("MonitorProfile",SYSTEMMONITORPROFILE_ESCAPESTRING),
	ConfigTemplate("MonitorProfileActive",int(1)),
	ConfigTemplate("RenderingIntent",int(LCMSWRAPPER_INTENT_PERCEPTUAL)),
	ConfigTemplate("ProofMode",int(CM_PROOFMODE_NONE)),
#ifdef WIN32
	ConfigTemplate("ProfilePath","c:\\winnt\\system32\\spool\\drivers\\color\\;c:\\windows\\system32\\spool\\drivers\\color;$HOME\\.color\\icc;profiles"),
#else
	ConfigTemplate("ProfilePath","/usr/share/color/icc:/usr/local/share/color/icc:$HOME/.color/icc"),
#endif
	ConfigTemplate()
};


ProfileManager::ProfileManager(ConfigFile *inifile,const char *section) :
	ConfigDB(Template), SearchPathHandler(), PTMutex(), first(NULL), proffromdisplay_size(0), spiter(*this)
{
#ifndef WIN32
	xdisplay = XOpenDisplay(NULL);
 	proffromdisplay=NULL;
#endif
	new SearchPathHandlerDBHandler(inifile,section,this,this,"ProfilePath");
	AddPath(FindString("ProfilePath"));
	GetProfileFromDisplay();
	if(proffromdisplay_size)
		SetInt("MonitorProfileActive",1);
}


ProfileManager::~ProfileManager()
{
#ifndef WIN32
	if(proffromdisplay)
		XFree(proffromdisplay);
	proffromdisplay=NULL;
	if(xdisplay)
		XCloseDisplay (xdisplay);
#endif
	ProfileInfo *pi;
	while((pi=first))
		delete first;
}


CMSProfile *ProfileManager::GetProfile(const char *name)
{
	CMSProfile *result=NULL;
	if(name && strlen(name))
	{
		if(strcmp(name,SYSTEMMONITORPROFILE_ESCAPESTRING)==0)
		{
#ifdef WIN32
			if(displayprofilename)
				result=new CMSProfile(displayprofilename);
#else
			if(proffromdisplay_size)
				result=new CMSProfile((char *)proffromdisplay,proffromdisplay_size);
#endif
			if(!result)
			{
				Debug[TRACE] << "Couldn't open monitor profile - falling back to builtin sRGB" << endl;
				result=new CMSProfile();
			}
		}
		else if(strcmp(name,BUILTINSRGB_ESCAPESTRING)==0)
		{
			result=new CMSProfile();
		}
		else if(strcmp(name,BUILTINSGREY_ESCAPESTRING)==0)
		{
			result=new CMSProfile(IS_TYPE_GREY);
		}
		else
		{
			char *fn=SearchPaths(name);
			if(!fn)
			{
				char *tmp=strdup(name);
				fn=SearchPaths(basename(tmp));
				free(tmp);
			}
			if(fn && strlen(fn))
			{
				try
				{
					result=new CMSProfile(fn);
				}
				catch(const char *err)
				{
					Debug[ERROR] << err << endl;
					result=NULL;
				}
			}
			if(fn)
				free(fn);
		}
	}
	return(result);
}


CMSProfile *ProfileManager::GetProfile(enum CMColourDevice target)
{
	const char *profilename=NULL;
	CMSProfile *result=NULL;
	switch(target)
	{
		case CM_COLOURDEVICE_DISPLAY:
			// If we can't open the monitor's profile, we fall back to the default RGB profile.
			if(FindInt("MonitorProfileActive"))
			{
				profilename=FindString("MonitorProfile");
				result=GetProfile(profilename);
			}
			if(!result)
				result=GetProfile(CM_COLOURDEVICE_DEFAULTRGB);
			return(result);
			break;
		case CM_COLOURDEVICE_PRINTERPROOF:
			if(FindInt("MonitorProfileActive"))
				profilename=FindString("MonitorProfile");
			break;
		case CM_COLOURDEVICE_EXPORT:
			if(FindInt("ExportProfileActive"))
				profilename=FindString("ExportProfile");
			break;
		case CM_COLOURDEVICE_PRINTER:
			if(FindInt("PrinterProfileActive"))
				profilename=FindString("PrinterProfile");
			break;
		case CM_COLOURDEVICE_DEFAULTGREY:
			if(FindInt("DefaultGreyProfileActive"))
				profilename=FindString("DefaultGreyProfile");
			break;
		case CM_COLOURDEVICE_DEFAULTRGB:
			if(FindInt("DefaultRGBProfileActive"))
				profilename=FindString("DefaultRGBProfile");
			break;
		case CM_COLOURDEVICE_DEFAULTCMYK:
			if(FindInt("DefaultCMYKProfileActive"))
				profilename=FindString("DefaultCMYKProfile");
			break;
		default:
			break;
	}
	if(profilename)
		return(GetProfile(profilename));
	else
		return(NULL);
}


CMSProfile *ProfileManager::GetDefaultProfile(IS_TYPE colourspace)
{
	const char *profilename=NULL;
	switch(STRIP_ALPHA(colourspace))
	{
		case IS_TYPE_GREY:
			if(FindInt("DefaultGreyProfileActive"))
				profilename=FindString("DefaultGreyProfile");
			break;
		case IS_TYPE_RGB:
			if(FindInt("DefaultRGBProfileActive"))
				profilename=FindString("DefaultRGBProfile");
			break;
		case IS_TYPE_CMYK:
			if(FindInt("DefaultCMYKProfileActive"))
				profilename=FindString("DefaultCMYKProfile");
			break;
		default:
			throw "Image colourspace not yet handled...";
			break;
	}
	if(profilename)
		return(GetProfile(profilename));
	else
		return(NULL);
}


void ProfileManager::SetProofMode(enum CMProofMode mode)
{
	if(mode!=CM_PROOFMODE_NONE)
	{
		const char *err=NULL;
		CMSProfile *t=NULL;

		if((t=GetProfile(CM_COLOURDEVICE_PRINTER)))
			delete t;
		else
			err=_("Can't do proofing without a valid Printer profile!");

		if((t=GetProfile(CM_COLOURDEVICE_DISPLAY)))
			delete t;
		else
			err=_("Can't do proofing without a valid Monitor profile!");

		if((t=GetProfile(CM_COLOURDEVICE_DEFAULTRGB)))
			delete t;
		else
			err=_("Can't do proofing without a valid Default RGB profile!");
		if(err)
			throw err;
	}
	SetInt("ProofMode",mode);
}


CMTransformFactory *ProfileManager::GetTransformFactory()
{
	return(new CMTransformFactory(*this));
}



// CMTransformFactory

// CMTransformFactoryNode

CMTransformFactoryNode::CMTransformFactoryNode(CMTransformFactory *header,RefCountPtr<CMSTransform> transform,MD5Digest &d1,MD5Digest &d2,
	LCMSWrapper_Intent intent,bool proof,LCMSWrapper_Intent proofintent)
	: header(header), prev(NULL), next(NULL), transform(transform), digest1(d1), digest2(d2), intent(intent), proof(proof),proofintent(proofintent)
{
	prev=header->first;
	if((prev=header->first))
	{
		while(prev->next)
			prev=prev->next;
		prev->next=this;
	}
	else
		header->first=this;
}


CMTransformFactoryNode::~CMTransformFactoryNode()
{
	if(next)
		next->prev=prev;
	if(prev)
		prev->next=next;
	else
		header->first=next;
}


// CMTransformFactory proper


CMTransformFactory::CMTransformFactory(ProfileManager &pm)
	: manager(pm), first(NULL)
{
}


CMTransformFactory::~CMTransformFactory()
{
	while(first)
		delete first;
}


RefCountPtr<CMSTransform> CMTransformFactory::Search(MD5Digest *srcdigest,MD5Digest *dstdigest,
	LCMSWrapper_Intent intent,bool proof,LCMSWrapper_Intent proofintent)
{
	CMTransformFactoryNode *tfn=first;
	while(tfn)
	{
		Debug[TRACE] << "Evaluating transform from " << tfn->digest1.GetPrintableDigest() << " to " << tfn->digest2.GetPrintableDigest() << " and intent " << tfn->intent << endl;
		if((*srcdigest==tfn->digest1)&&(*dstdigest==tfn->digest2)&&(intent==tfn->intent)&&(proof==tfn->proof)&&(proofintent==tfn->proofintent))
			return(tfn->transform);
		tfn=tfn->next;
	}
	RefCountPtr<CMSTransform> tmp;
	return(tmp);
}


RefCountPtr<CMSTransform> CMTransformFactory::GetTransform(enum CMColourDevice target,IS_TYPE type,LCMSWrapper_Intent intent)
{
	Debug[TRACE] << "TransformFactory getting default profile for image of type: " << type << endl;
	CMSProfile *srcprofile=manager.GetDefaultProfile(type);
	if(srcprofile)
		Debug[TRACE] << "Have source profile with input space" << srcprofile->GetColourSpace() << endl;
	else
		Debug[TRACE] << "Unable to open default profile" << endl;

	RefCountPtr<CMSTransform> t;
	try
	{
		t=GetTransform(target,srcprofile,intent);
	}
	catch(const char *err)
	{
	}
	delete srcprofile;
	return(t);
}


RefCountPtr<CMSTransform> CMTransformFactory::GetTransform(enum CMColourDevice target,ImageSource *src,LCMSWrapper_Intent intent)
{
	Debug[TRACE] << "TransformFactory trying embedded profile..." << endl;
	RefCountPtr<CMSProfile> srcprofile=src->GetEmbeddedProfile();
	if(srcprofile)
		return(GetTransform(target,&*srcprofile,intent));
	else
		return(GetTransform(target,IS_TYPE(STRIP_ALPHA(src->type)),intent));
}


RefCountPtr<CMSTransform> CMTransformFactory::GetTransform(enum CMColourDevice target,CMSProfile *srcprofile,LCMSWrapper_Intent intent)
{
	RefCountPtr<CMSTransform> result;
	// If a NULL profile is supplied, we currently bail out.
	// Theoretically we could continue if the target's profile is a DeviceLink,
	// or we could assume a colourspace to match the target's profile,
	// and fall back gracefully.

	if(!srcprofile)
		return(result);

	Debug[TRACE] << "TransformFactory using source profile of type: " << srcprofile->GetColourSpace() << endl;
	
	CMSProfile *destprofile=manager.GetProfile(target);

	if(target==CM_COLOURDEVICE_PRINTERPROOF)
	{
		CMSProfile *proofprofile=NULL;
		switch(manager.FindInt("ProofMode"))
		{
			case CM_PROOFMODE_NONE:
//				Debug[TRACE] << "Proofmode: None - using normal transform" << endl;
				result=GetTransform(destprofile,srcprofile,intent);
				break;
			case CM_PROOFMODE_SIMULATEPRINT:
//				Debug[TRACE] << "Proofmode: Simulate Printer - using abs.col. proof transform" << endl;
				proofprofile=manager.GetProfile(CM_COLOURDEVICE_PRINTER);
				if(proofprofile)
					result=GetTransform(destprofile,srcprofile,proofprofile,intent,LCMSWRAPPER_INTENT_ABSOLUTE_COLORIMETRIC);
				break;
			case CM_PROOFMODE_SIMULATEPRINTADAPTWHITE:
//				Debug[TRACE] << "Proofmode: Simulate Printer, Adapt White - using rel.col. proof transform" << endl;
				proofprofile=manager.GetProfile(CM_COLOURDEVICE_PRINTER);
				if(proofprofile)
					result=GetTransform(destprofile,srcprofile,proofprofile,intent,LCMSWRAPPER_INTENT_RELATIVE_COLORIMETRIC);
				break;
			default:
				break;
		}
		if(proofprofile)
			delete proofprofile;
	}
	else
		result=GetTransform(destprofile,srcprofile,intent);

	delete destprofile;

	return(result);
}


RefCountPtr<CMSTransform> CMTransformFactory::GetTransform(CMSProfile *destprofile,CMSProfile *srcprofile,LCMSWrapper_Intent intent)
{
	RefCountPtr<CMSTransform> transform;
	// No point whatever in continuing without an output device profile...
	if(!destprofile)
		return(transform);

	if(srcprofile)
		Debug[TRACE] << "TransformFactory using source profile of type: " << srcprofile->GetColourSpace() << endl;
	else
		Debug[TRACE] << "TransformFactory - no source profile present." << endl;

	if(intent==LCMSWRAPPER_INTENT_DEFAULT)
		intent=LCMSWrapper_Intent(manager.FindInt("RenderingIntent"));
	if(intent==LCMSWRAPPER_INTENT_DEFAULT)
		intent=LCMSWRAPPER_INTENT_PERCEPTUAL;

//	Debug[TRACE] << "Using intent: " << intent << endl;

	// We use MD5 digests to compare profiles for equality.
	MD5Digest *d1,*d2;
	d2=destprofile->GetMD5();

	const char *fn=destprofile->GetFilename();
	Debug[TRACE] << "Destination profile (" << (fn ? fn : "") << ")" << "has hash: " << d2->GetPrintableDigest() << endl;

	if(destprofile->IsDeviceLink())
	{
		Debug[TRACE] << "Device link profile detected" << endl;
		// Device link profiles make life awkward if we have to use a source profile
		// (which we must do in the case of an image having an embedded profile).
		// What we do here is convert from the source profile to the appropriate
		// colour space's default profile, and then apply the devicelink.

		CMSProfile *defprofile=NULL;
		if(srcprofile)
		{
			// Need to use default profile for the devicelink's *input* -
			// thus taking care of RGB -> CMYK devlinks used with CMYK input files.
			defprofile=manager.GetDefaultProfile(destprofile->GetColourSpace());
		}

		// If we have both source and default profiles, and they're not equal,
		// create a multi-profile transform: src -> default -> devicelink.
		if((srcprofile)&&(defprofile)&&(*srcprofile->GetMD5()!=*defprofile->GetMD5()))
		{
			Debug[TRACE] << "Source and default profiles don't match - building transform chain..." << endl;
			CMSProfile *profiles[3];
			profiles[0]=srcprofile;
			profiles[1]=defprofile;
			profiles[2]=destprofile;
			d1=srcprofile->GetMD5();

			const char *fn=srcprofile->GetFilename();
			Debug[TRACE] << "Source profile (" << (fn ? fn : "") << ")" << "has hash: " << d1->GetPrintableDigest() << endl;
			
			// Search for an existing transform by source / devicelink MD5s...
			transform=Search(d1,d2,intent);
			if(!transform)
			{
				Debug[TRACE] << "No suitable cached transform found - creating a new one..." << endl;
				transform=new CMSTransform(profiles,3,intent);
				new CMTransformFactoryNode(this,transform,*d1,*d2,intent);
			}
		}
		else
		{
			Debug[TRACE] << "Source and default profiles match - using devicelink in isolation..." << endl;
			// If there's no default profile, or the source and default profiles match
			// then we can just use the devicelink profile in isolation.
			d1=d2;
			transform=Search(d1,d2,intent);
			if(!transform)
			{
				Debug[TRACE] << "No suitable cached transform found - creating a new one..." << endl;
				transform=new CMSTransform(destprofile,intent);
				new CMTransformFactoryNode(this,transform,*d1,*d2,intent);
			}
		}
		if(defprofile)
			delete defprofile;
	}
	else
	{
		// The non-device link case is much easier to deal with...
		d1=srcprofile->GetMD5();

		const char *fn=srcprofile->GetFilename();
		Debug[TRACE] << "Source profile (" << (fn ? fn : "") << ")" << "has hash: " << d1->GetPrintableDigest() << endl;

		// Don't bother transforming if src/dest are the same profile...
		if(*d1==*d2)
		{
			Debug[TRACE] << "Source and target profiles are identical - no need to transform" << endl;
			// Instead of returning NULL, return a NULL transform.
			transform=new NullCMSTransform(srcprofile->GetColourSpace());
			return(transform);
		}

		transform=Search(d1,d2,intent);
		if(!transform)
		{
			Debug[TRACE] << "No suitable cached transform found - creating a new one..." << endl;
			transform=new CMSTransform(srcprofile,destprofile,intent);
			new CMTransformFactoryNode(this,transform,*d1,*d2,intent);
		}
	}

	return(transform);
}


RefCountPtr<CMSTransform> CMTransformFactory::GetTransform(CMSProfile *destprofile,CMSProfile *srcprofile,CMSProfile *proofprofile,
	LCMSWrapper_Intent intent,LCMSWrapper_Intent displayintent)
{
//	Debug[TRACE] << "Getting proofing transform - Using intent: " << intent << endl;
	RefCountPtr<CMSTransform> transform;

	// No point whatever in continuing without an output device profile...
	if(!destprofile)
		return(transform);

	if(!proofprofile)
		throw _("No Proof profile provided!");

	if(intent==LCMSWRAPPER_INTENT_DEFAULT)
		intent=LCMSWrapper_Intent(manager.FindInt("RenderingIntent"));
	if(intent==LCMSWRAPPER_INTENT_DEFAULT)
		intent=LCMSWRAPPER_INTENT_PERCEPTUAL;

	if(displayintent==LCMSWRAPPER_INTENT_DEFAULT)
		displayintent=LCMSWRAPPER_INTENT_ABSOLUTE_COLORIMETRIC;

//	Debug[TRACE] << "Using intent: " << intent << endl;

	// We use MD5 digests to compare profiles for equality.
	MD5Digest *d1,*d2;
	d2=destprofile->GetMD5();

	if(destprofile->IsDeviceLink())
	{
//		Debug[TRACE] << "Device link profile detected" << endl;
		// Device link profiles make life awkward if we have to use a source profile
		// (which we must do in the case of an image having an embedded profile).
		// What we do here is convert from the source profile to the appropriate
		// colour space's default profile, and then apply the devicelink.

		CMSProfile *defprofile=NULL;
		if(srcprofile)
			defprofile=manager.GetDefaultProfile(srcprofile->GetColourSpace());

		// If we have both source and default profiles, and they're not equal,
		// create a multi-profile transform: src -> default -> devicelink.
		if((srcprofile)&&(defprofile)&&(*srcprofile->GetMD5()!=*defprofile->GetMD5()))
		{
//			Debug[TRACE] << "Source and default profiles don't match - building transform chain..." << endl;
			CMSProfile *profiles[3];
			profiles[0]=srcprofile;
			profiles[1]=defprofile;
			profiles[2]=destprofile;
			d1=srcprofile->GetMD5();
			
			// Search for an existing transform by source / devicelink MD5s...
			// FIXME - what about display intent?
			transform=Search(d1,d2,intent,true,displayintent);
			if(!transform)
			{
//				Debug[TRACE] << "No suitable cached transform found - creating a new one..." << endl;
//				Debug[TRACE] << "But can't (yet?) create embedded->default->devicelink->proof transform!" << endl;
				// FIXME - need a version of CMSProofingTransform that can cope with
				// multiple profiles!
				transform=new CMSTransform(profiles,3,intent);
				new CMTransformFactoryNode(this,transform,*d1,*d2,intent,displayintent);
			}
		}
		else
		{
//			Debug[TRACE] << "Source and default profiles match - using devicelink in isolation..." << endl;
			// If there's no default profile, or the source and default profiles match
			// then we can just use the devicelink profile in isolation.
			d1=d2;
			transform=Search(d1,d2,intent,true,displayintent);
			if(!transform)
			{
//				Debug[TRACE] << "No suitable cached transform found - creating a new one..." << endl;
				// FIXME - need a version of CMSProofingTransform that can cope with
				// devicelink profiles
				transform=new CMSProofingTransform(destprofile,proofprofile,intent,displayintent);
				new CMTransformFactoryNode(this,transform,*d1,*d2,intent,true,displayintent);
			}
		}
		if(defprofile)
			delete defprofile;
	}
	else
	{
		// The non-device link case is much easier to deal with...
		d1=srcprofile->GetMD5();

		// Don't bother transforming if src/dest are the same profile...
		// (Actually, if we're proofing, we still need to transform, after all!
//		if(*d1==*d2)
//			return(NULL);

		transform=Search(d1,d2,intent,true,displayintent);
		if(!transform)
		{
//			Debug[TRACE] << "No suitable cached transform found - creating a new proofing transform..." << endl;
			transform=new CMSProofingTransform(srcprofile,destprofile,proofprofile,intent,displayintent);
			new CMTransformFactoryNode(this,transform,*d1,*d2,intent,true,displayintent);
		}
	}

	return(transform);
}


void CMTransformFactory::Flush()
{
	while(first)
		delete first;
}


ProfileManager &CMTransformFactory::GetManager()
{
	return(manager);
}


// Path handling

static const char *findextension(const char *filename)
{
	int t=strlen(filename)-1;
	int c;
	for(c=t;c>0;--c)
	{
		if(filename[c]=='.')
			return(filename+c);
	}
	return(filename);
}


const char *ProfileManager::GetNextFilename(const char *prev)
{
	const char *result=prev;
	while((result=spiter.GetNextFilename(result)))
	{
		const char *ext=findextension(result);
		if(strncasecmp(ext,".ICM",4)==0)
			return(result);
		if(strncasecmp(ext,".icm",4)==0)
			return(result);
		if(strncasecmp(ext,".ICC",4)==0)
			return(result);
		if(strncasecmp(ext,".icc",4)==0)
			return(result);
	}
	return(result);
}


void ProfileManager::AddPath(const char *path)
{
	FlushProfileInfoList();
	SearchPathHandler::AddPath(path);
}


void ProfileManager::RemovePath(const char *path)
{
	FlushProfileInfoList();
	SearchPathHandler::RemovePath(path);
}


void ProfileManager::ClearPaths()
{
	FlushProfileInfoList();
	SearchPathHandler::ClearPaths();
}


char *ProfileManager::SearchPaths(const char *file)
{
	if(file)
	{
		if(strcmp(file,SYSTEMMONITORPROFILE_ESCAPESTRING)==0)
			return(strdup(file));
		if(strcmp(file,BUILTINSRGB_ESCAPESTRING)==0)
			return(strdup(file));
		if(strcmp(file,BUILTINSGREY_ESCAPESTRING)==0)
			return(strdup(file));
	}
	return(SearchPathHandler::SearchPaths(file));
}


ProfileInfo *ProfileManager::GetFirstProfileInfo()
{
	if(!first)
		BuildProfileInfoList();
	return(first);
}


void ProfileManager::BuildProfileInfoList()
{
	Debug[TRACE] << "Building ProfileInfo List:" << endl;
	const char *f=NULL;
	Debug[TRACE] << "Flushed existing list - if any." << endl;
	FlushProfileInfoList();
	new ProfileInfo(*this,BUILTINSRGB_ESCAPESTRING);
	new ProfileInfo(*this,BUILTINSGREY_ESCAPESTRING);
	Debug[TRACE] << "Added builtins." << endl;
	while((f=GetNextFilename(f)))
	{
		Debug[TRACE] << "Adding " << f << endl;
		if(!(FindProfileInfo(f)))
			new ProfileInfo(*this,f);
	}
	if(!proffromdisplay_size)	// We don't refresh the system monitor profile here, to avoid threading problems
	{							// if the list is built from a sub-thread.
		Debug[TRACE] << "Finished adding disk-based profiles - getting display profile..." << endl;
		GetProfileFromDisplay();
		Debug[TRACE] << "Done." << endl;
	}
	if(proffromdisplay_size)
	{
		Debug[TRACE] << "Got system monitor profile - adding..." << endl;
		new ProfileInfo(*this,SYSTEMMONITORPROFILE_ESCAPESTRING);
	}
	else
		Debug[TRACE] << "No system monitor profile." << endl;
	Debug[TRACE] << "ProfileInfoList building complete." << endl;
}


void ProfileManager::FlushProfileInfoList()
{
	while(first)
		delete first;
}


ProfileInfo *ProfileManager::FindProfileInfo(const char *filename)
{
	ObtainMutex();
	ProfileInfo *pi=first;
	while(pi)
	{
		const char *fn=pi->filename;
		if(strcmp(fn,filename)==0)
		{
			Debug[TRACE] << "Found " << filename << endl;
			ReleaseMutex();
			return(pi);
		}
		pi=pi->Next();
	}
	ReleaseMutex();
	return(NULL);
}


int ProfileManager::GetProfileInfoCount()
{
	int c=0;
	ProfileInfo *pi=GetFirstProfileInfo();
	while(pi)
	{
		++c;
		pi=pi->Next();
	}
	return(c);
}


ProfileInfo *ProfileManager::GetProfileInfo(int i)
{
	ProfileInfo *pi=GetFirstProfileInfo();
	while(i && pi)
	{
		--i;
		pi=pi->Next();
	}
	return(pi);
}


void ProfileManager::ReleaseMutex()
{
	ProfileInfo *pi=first;
	while(pi)
	{
		if(pi->remove)
		{
			ProfileInfo *n=pi->next;
			delete pi;
			pi=n;
		}
		else
			pi=pi->next;
	}
	PTMutex::ReleaseMutex();
}


// ProfileInfo


ProfileInfo::ProfileInfo(ProfileManager &pm,const char *filename)
	: profilemanager(pm), next(NULL), prev(NULL), filename(NULL), iscached(false), description(NULL), isdevicelink(false), remove(false), virt(false)
{
	if(!filename)
		throw "ProfileInfo: Null Filename";

	if(strcmp(filename,SYSTEMMONITORPROFILE_ESCAPESTRING)==0 || strcmp(filename,BUILTINSRGB_ESCAPESTRING)==0 || strcmp(filename,BUILTINSGREY_ESCAPESTRING)==0)
		virt=true;

	profilemanager.ObtainMutex();
	if((next=profilemanager.first))
		next->prev=this;
	profilemanager.first=this;
	this->filename=strdup(filename);
	profilemanager.ReleaseMutex();
}


ProfileInfo::~ProfileInfo()
{
	profilemanager.ObtainMutex();
	if(prev)
		prev->next=next;
	else
		profilemanager.first=next;
	if(next)
		next->prev=prev;
	if(filename)
		free(filename);
	if(description)
		free(description);
	profilemanager.ReleaseMutex();
}


ProfileInfo *ProfileInfo::Next()
{
	return(next);
}


bool ProfileInfo::IsVirtual()
{
	return(virt);
}


void ProfileInfo::GetInfo()
{
	if(iscached)
		return;

	CMSProfile *profile=profilemanager.GetProfile(filename);
	if(profile)
	{
		colourspace=profile->GetColourSpace();
		isdevicelink=profile->IsDeviceLink();
		description=strdup(profile->GetDescription());
		delete profile;
		iscached=true;
	}
	else
	{
		remove=true;
		throw "ProfileInfo: Can't open profile";
	}
}


const char *ProfileInfo::GetFilename()
{
	GetInfo();
	return(filename);
}


const char *ProfileInfo::GetDescription()
{
	GetInfo();
	return(description);
}


bool ProfileInfo::IsDeviceLink()
{
	GetInfo();
	return(isdevicelink);
}


IS_TYPE ProfileInfo::GetColourSpace()
{
	GetInfo();
	return(colourspace);
}


int ProfileManager::GetIntentCount()
{
	return(CMS_GetIntentCount());
}

const char *ProfileManager::GetIntentName(LCMSWrapper_Intent intent)
{
	if(intent==LCMSWRAPPER_INTENT_DEFAULT)
		return(_("Default"));
	else
		return(CMS_GetIntentName(intent));
}

const char *ProfileManager::GetIntentDescription(LCMSWrapper_Intent intent)
{
	if(intent==LCMSWRAPPER_INTENT_DEFAULT)
		return(_("Default rendering intent"));
	else
		return(CMS_GetIntentDescription(intent));
}


void ProfileManager::GetProfileFromDisplay()
{
#ifdef WIN32
	HDC handle=GetDC(0);	// Get the default screen handle
	DWORD dpsize=sizeof(displayprofilename)-1;

	// Older Mingw wingdi.h files have a faulty definition of the following function,
	// with the second param as DWORD instead of LPDWORD.
	if(GetICMProfile(handle,&dpsize,displayprofilename))
	{
		proffromdisplay_size=dpsize;
		Debug[TRACE] << "Got profile: " << displayprofilename << ", " << displayprofilename << " characters" << endl;
	}
	else
		Debug[TRACE] << "No profile associated with default display." << endl;
#else
	Debug[TRACE] << "Getting system monitor profile." << endl;
	if(proffromdisplay)
	{
		Debug[TRACE] << "Freeing old system monitor profile." << endl;
		XFree(proffromdisplay);
	}
	proffromdisplay=NULL;

	if(xdisplay)
	{
		Debug[TRACE] << "Got display - fetching atom" << endl;
		Atom icc_atom;
		icc_atom = XInternAtom (xdisplay, "_ICC_PROFILE", False);
		if (icc_atom != None)
		{
			Debug[TRACE] << "Got atom" << endl;
			Window w=DefaultRootWindow(xdisplay);
			if(w)
			{
				Debug[TRACE] << "Got window" << endl;
				Atom type;
				int format=0;
				unsigned long nitems=0;
				unsigned long bytes_after=0;
				int result=0;

				Debug[TRACE] << "Calling XGetWindowPropery" << endl;

				result = XGetWindowProperty (xdisplay, w, icc_atom, 0,
					0x7fffffff,0, XA_CARDINAL,
					&type, &format, &nitems,
					&bytes_after, &proffromdisplay);
				proffromdisplay_size=nitems*(format/8);

				Debug[ERROR] <<"Result: " << result << ", size: " << proffromdisplay_size << endl;

				if(result!=Success)
				{
					Debug[WARN] <<"Failed to retrieve ICC Profile from display..." << endl;
					proffromdisplay=NULL;
					proffromdisplay_size=0;
				}
			}
		}
	}
//	XCloseDisplay (dpy);
#endif
}
