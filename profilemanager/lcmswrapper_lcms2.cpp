/*
 * lcmswrapper.cpp - encapsulates typical "user" functions of LittleCMS,
 * providing a Profile and Transform class
 *
 * Copyright (c) 2004-2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * 2005-05-01: No longer depends on ini.h - the neccesary data for generating
 *             rough-cast profiles is now encapsulated in suitable classes.
 * 2006-09-04: Added filename, since LCMS doesn't provide pointer to raw data
 *             which is needed for TIFF embedding...
 *
 * TODO: pixel type, support Lab, XYZ, etc.
 *
 */

#include <iostream>
#include <fstream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

#include "lcmswrapper_lcms2.h"
#include "imagesource.h"

using namespace std;

CMSRGBPrimaries CMSPrimaries_Rec709(.64,.33,.3,.6,.15,.06);
CMSRGBPrimaries CMSPrimaries_Adobe(0.64, 0.33,0.21, 0.71,0.15, 0.06);
CMSRGBPrimaries CMSPrimaries_NTSC(0.67, 0.33, 0.21, 0.71,0.14, 0.08);
CMSRGBPrimaries CMSPrimaries_EBU(0.64, 0.33, 0.29, 0.60, 0.15, 0.06);
CMSRGBPrimaries CMSPrimaries_SMPTE(0.63, 0.34, 0.31, 0.595, 0.155, 0.070);
CMSRGBPrimaries CMSPrimaries_HDTV(0.670, 0.330, 0.210, 0.710,0.150, 0.060);
CMSRGBPrimaries CMSPrimaries_CIE(0.7355,0.2645,0.2658,0.7243,0.1669,0.0085);

CMSProfile::CMSProfile(const char *fn) : md5(NULL), generated(false), filename(NULL), buffer(NULL), buflen(0)
{
	if(!fn)
		throw "NULL profile filename provided";

//	cmsErrorAction(LCMS_ERROR_SHOW);

	filename=strdup(fn);

	if(!(prof=cmsOpenProfileFromFile(filename,"r")))
		throw "Can't open profile";

	CalcMD5FromFile();
}


// CalcMD5 has been split because it's getting harder to determine algorithmically which method should be used.
// Much more predictable to call the right variant explicity in the constructors.

void CMSProfile::CalcMD5FromGenerated()
{
	Debug[TRACE] << "Saving profile to RAM for MD5 calculation." << endl;
	cmsUInt32Number plen=0;
	cmsSaveProfileToMem(prof,NULL,&plen);
	if(plen>0)
	{
		cerr << "Plen = " << plen << endl;
		buflen=plen;
		buffer=(char *)malloc(buflen);
		if(cmsSaveProfileToMem(prof,buffer,&plen))
		{
			cerr << "Saved successfully" << endl;
			md5=new MD5Digest(buffer+sizeof(cmsICCHeader),buflen-sizeof(cmsICCHeader));
		}
	}
}

void CMSProfile::CalcMD5FromFile()
{
	BinaryBlob blob(filename);
	md5=new MD5Digest(blob.GetPointer()+sizeof(cmsICCHeader),blob.GetSize()-sizeof(cmsICCHeader));
}

void CMSProfile::CalcMD5FromMem()
{
	md5=new MD5Digest(buffer+sizeof(cmsICCHeader),buflen-sizeof(cmsICCHeader));
}

#if 0
void CMSProfile::CalcMD5()
{
	if(generated)
	{
		cerr << "Saving profile to RAM for MD5 calculation." << endl;
		cmsUInt32Number plen=0;
		cmsSaveProfileToMem(prof,NULL,&plen);
		if(plen>0)
		{
			cerr << "Plen = " << plen << endl;
			buflen=plen;
			buffer=(char *)malloc(buflen);
			if(cmsSaveProfileToMem(prof,buffer,&plen))
			{
				cerr << "Saved successfully" << endl;
				md5=new MD5Digest(buffer+sizeof(cmsICCHeader),buflen-sizeof(cmsICCHeader));
			}
		}
	}
	else if(filename)
	{
		ifstream f(filename);
		f.seekg(0,ios::end);
		int filelen=f.tellg();
		f.seekg(0);

		char *data=(char *)malloc(filelen);
		f.read((char *)data,filelen);
		f.close();
		md5=new MD5Digest(data+sizeof(cmsICCHeader),filelen-sizeof(cmsICCHeader));
		free(data);
	}
	else
	{
		md5=new MD5Digest(buffer+sizeof(cmsICCHeader),buflen-sizeof(cmsICCHeader));
	}
}
#endif


CMSProfile::CMSProfile(CMSRGBPrimaries &primaries,CMSRGBGamma &gamma,CMSWhitePoint &whitepoint)
	: md5(NULL), generated(true), filename(NULL), buffer(NULL), buflen(0)
{
	if(!(prof=cmsCreateRGBProfile(&whitepoint.whitepoint,&primaries,gamma.gammatables)))
		throw "Can't create virtual RGB profile";
	CalcMD5FromGenerated();
}


CMSProfile::CMSProfile(CMSGamma &gamma,CMSWhitePoint &whitepoint)
	: md5(NULL), generated(true), filename(NULL), buffer(NULL), buflen(0)
{
	if(!(prof=cmsCreateGrayProfile(&whitepoint.whitepoint,gamma.GetGammaTable())))
		throw "Can't create virtual Grey profile";
	CalcMD5FromGenerated();
}


CMSProfile::CMSProfile(CMSWhitePoint &whitepoint)
	: md5(NULL), generated(true), filename(NULL), buffer(NULL), buflen(0)
{
	if(!(prof=cmsCreateLab2Profile(&whitepoint.whitepoint)))
		throw "Can't create virtual LAB profile";
	CalcMD5FromGenerated();
}


CMSProfile::CMSProfile(char *srcbuf,int length)
	: md5(NULL), generated(false), filename(NULL), buffer(NULL), buflen(0)
{
	buffer=(char *)malloc(length);
	buflen=length;
	memcpy(buffer,srcbuf,buflen);
	if(!(prof=cmsOpenProfileFromMem(buffer,buflen)))
		throw "Can't open profile";
	CalcMD5FromMem();
}


CMSProfile::CMSProfile(IS_TYPE type)
	: md5(NULL), generated(true), filename(NULL), buffer(NULL), buflen(0)
{
	switch(type)
	{
		case IS_TYPE_RGB:
			Debug[TRACE] << "Generating virtual sRGB profile" << endl;
			if(!(prof=cmsCreate_sRGBProfile()))
				throw "Can't create virtual sRGB profile";
			break;
		case IS_TYPE_GREY:
			Debug[TRACE] << "Generating virtual sGrey profile" << endl;
			{
				CMSGamma gamma(2.4,true);
				CMSWhitePoint wp(6500);
				if(!(prof=cmsCreateGrayProfile(&wp.whitepoint,gamma.GetGammaTable())))
					throw "Can't create virtual sRGB profile";
			}
			break;
			break;
		default:
			throw "CMSProfile: asked to create a default profile for an unhandled colourspace.";
			break;		
	}
	CalcMD5FromGenerated();
	Debug[TRACE] << "Buffer: " << long(buffer) << endl;
}


CMSProfile::CMSProfile(const CMSProfile &src)
	: md5(NULL), generated(src.generated), filename(NULL), buffer(NULL), buflen(0)
{
	cerr << "In CMSProfile Copy Constructor" << endl;
	if(src.filename)
	{
		filename=strdup(src.filename);

		if(!(prof=cmsOpenProfileFromFile(filename,"r")))
			throw "Can't open profile";
	}
	else
	{
		buffer=(char *)malloc(src.buflen);
		buflen=src.buflen;
		memcpy(buffer,src.buffer,src.buflen);
		if(!(prof=cmsOpenProfileFromMem(buffer,buflen)))
			throw "Can't open profile";
	}
	md5=new MD5Digest(*src.md5);
}


CMSProfile::~CMSProfile()
{
	if(filename)
		free(filename);
	if(prof)
		cmsCloseProfile(prof);
	if(buffer)
		free(buffer);
	if(md5)
		delete md5;
}


bool CMSProfile::operator==(const CMSProfile &other)
{
	if(md5&&other.md5)
	{
		return(*md5==*other.md5);
	}
	else
		return(false);
}


bool CMSProfile::IsDeviceLink()
{
	return(cmsGetDeviceClass(prof) == cmsSigLinkClass);
}


#if 0
bool CMSProfile::IsV4()
{
	cerr << "Profile version: " << cmsGetProfileICCversion(prof) << endl;
	return(cmsGetProfileICCversion(prof) >= 0x04000000L);
}
#endif 

enum IS_TYPE CMSProfile::GetColourSpace()
{
	cmsColorSpaceSignature sig=cmsGetColorSpace(prof);
	switch(sig)
	{
		case cmsSigGrayData:
			return(IS_TYPE_GREY);
			break;
		case cmsSigRgbData:
			return(IS_TYPE_RGB);
			break;
		case cmsSigCmykData:
			return(IS_TYPE_CMYK);
			break;
		default:
			return(IS_TYPE_NULL);
			break;
	}
}


enum IS_TYPE CMSProfile::GetDeviceLinkOutputSpace()
{
	if(!IsDeviceLink())
		throw "GetDeviceLinkOutputSpace() can only be used on DeviceLink profiles!";
	cmsColorSpaceSignature sig=cmsGetPCS(prof);
	switch(sig)
	{
		case cmsSigGrayData:
			return(IS_TYPE_GREY);
			break;
		case cmsSigRgbData:
			return(IS_TYPE_RGB);
			break;
		case cmsSigCmykData:
			return(IS_TYPE_CMYK);
			break;
		default:
			return(IS_TYPE_NULL);
			break;
	}
}


const char *CMSProfile::GetName()
{
    char *text=NULL;
    int len;

    len = cmsGetProfileInfo(prof, cmsInfoDescription, "en", "US", NULL, 0);
    if(len)
	{
	    text = (char *)malloc(len);
	    cmsGetProfileInfoASCII(prof, cmsInfoDescription, "en", "US", text, len);
	}
	if(text)
		return(text);
	else
		return("unknown");
}


const char *CMSProfile::GetManufacturer()
{
    char *text=NULL;
    int len;

    len = cmsGetProfileInfo(prof, cmsInfoManufacturer, "en", "US", NULL, 0);
    if(len)
	{
	    text = (char *)malloc(len);
	    cmsGetProfileInfoASCII(prof, cmsInfoManufacturer, "en", "US", text, len);
	}
	if(text)
		return(text);
	else
		return("unknown");
}


const char *CMSProfile::GetModel()
{
    char *text=NULL;
    int len;

    len = cmsGetProfileInfo(prof, cmsInfoModel, "en", "US", NULL, 0);
    if(len)
	{
	    text = (char *)malloc(len);
	    cmsGetProfileInfoASCII(prof, cmsInfoModel, "en", "US", text, len);
	}
	if(text)
		return(text);
	else
		return("unknown");
}


const char *CMSProfile::GetDescription()
{
	return(GetName());
}


const char *CMSProfile::GetCopyright()
{
    char *text=NULL;
    int len;

    len = cmsGetProfileInfo(prof, cmsInfoCopyright, "en", "US", NULL, 0);
    if(len)
	{
	    text = (char *)malloc(len);
	    cmsGetProfileInfoASCII(prof, cmsInfoCopyright, "en", "US", text, len);
	}
	if(text)
		return(text);
	else
		return("unknown");
}


MD5Digest *CMSProfile::GetMD5()
{
	return(md5);
}


BinaryBlob *CMSProfile::GetBlob()
{
	BinaryBlob *result=NULL;
	if(filename)
		result=new BinaryBlob(filename);
	if(!result && buffer)
	{
		Debug[TRACE] << "Creating binary blob from " << long(buffer) << ", " << buflen << endl;
		result=new BinaryBlob(buffer,buflen);
	}
	return(result);
}


const char *CMSProfile::GetFilename()
{
	return(filename);
}


bool CMSProfile::Save(const char *outfn)
{
	try
	{
//		(Since we now save a generated profile to memory so we can calc an MD5 hash
//		we can simply save that, like an embedded profile, if we need to.
//		if(generated)	// If profile was generated on the fly, then we use LCMS function
//		{				// to save it;
//			return(_cmsSaveProfile(prof,outfn));
//		}
//		else
//		{
			if(filename)	// If profile was loaded from disk, we need to load it again
			{
				ifstream f(filename);
				f.exceptions(fstream::badbit);
				f.seekg(0,ios::end);
				buflen=f.tellg();
				f.seekg(0);

				buffer=(char *)malloc(buflen);
				f.read((char *)buffer,buflen);
				f.close();
			}
			if(outfn)
			{
				cerr << "Saving buffer: " << long(buffer) << ", length: " << buflen << endl;
				ofstream f(outfn,ios::out|ios::binary);
				f.write(buffer,buflen);
				f.close();
			}
			if(filename)	// Did we load the profile from disk?  Need to free buffer if so
			{
				free(buffer);
				buffer=NULL;
				buflen=0;
			}
//		}
	}
	catch(fstream::failure e)
	{
		return(false);
	}
	return(true);
}


std::ostream& operator<<(std::ostream &s,CMSProfile &cp)
{
	return(s << cp.GetDescription());
}


CMSTransform::CMSTransform() : transform(NULL)
{
	inputtype=IS_TYPE_NULL;
	outputtype=IS_TYPE_NULL;
}


CMSTransform::CMSTransform(CMSProfile *in,CMSProfile *out,LCMSWrapper_Intent intent) : transform(NULL)
{
	inputtype=in->GetColourSpace();
	outputtype=out->GetColourSpace();
	MakeTransform(in,out,intent);
}


CMSTransform::CMSTransform(CMSProfile *devicelink,LCMSWrapper_Intent intent) : transform(NULL)
{
	inputtype=devicelink->GetColourSpace();
	outputtype=devicelink->GetDeviceLinkOutputSpace();
	MakeTransform(devicelink,NULL,intent);	
}


CMSTransform::CMSTransform(CMSProfile *profiles[],int profilecount,LCMSWrapper_Intent intent) : transform(NULL)
{
	inputtype=profiles[0]->GetColourSpace();
	if(profiles[profilecount-1]->IsDeviceLink())
		outputtype=profiles[profilecount-1]->GetDeviceLinkOutputSpace();
	else
		outputtype=profiles[profilecount-1]->GetColourSpace();

	cmsHPROFILE *p;
	if((p=(cmsHPROFILE *)malloc(sizeof(cmsHPROFILE)*profilecount)))
	{
		for(int i=0;i<profilecount;++i)
		{
			p[i]=profiles[i]->prof;
		}

		int it,ot;

		switch(inputtype)
		{
			case IS_TYPE_GREY:
				it=TYPE_GRAY_16_REV;
				break;
			case IS_TYPE_RGB:
				it=TYPE_RGB_16;
				break;
			case IS_TYPE_CMYK:
				it=TYPE_CMYK_16;
				break;
			default:
				throw "Unsupported colour space (input)";
				break;
		}
	
		switch(outputtype)
		{
			case IS_TYPE_GREY:
				ot=TYPE_GRAY_16_REV;
				break;
			case IS_TYPE_RGB:
				ot=TYPE_RGB_16;
				break;
			case IS_TYPE_CMYK:
				ot=TYPE_CMYK_16;
				break;
			default:
				throw "Unsupported colour space (output)";
				break;
		}
	
		transform = cmsCreateMultiprofileTransformTHR(0,p,profilecount,
			it,
			ot,
			CMS_GetLCMSIntent(intent), CMS_GetLCMSFlags(intent));

		free(p);	
	}
	else
		throw "Can't create multi-profile transform";
}


CMSTransform::~CMSTransform()
{
	if(transform)
		cmsDeleteTransform(transform);
}


void CMSTransform::Transform(unsigned short *src,unsigned short *dst,int pixels)
{
	cmsDoTransform(transform,src,dst,pixels);
}


IS_TYPE CMSTransform::GetInputColourSpace()
{
	return(inputtype);
}


IS_TYPE CMSTransform::GetOutputColourSpace()
{
	return(outputtype);
}


void CMSTransform::MakeTransform(CMSProfile *in,CMSProfile *out,LCMSWrapper_Intent intent)
{
	int it,ot;

	switch(GetInputColourSpace())
	{
		case IS_TYPE_GREY:
			it=TYPE_GRAY_16_REV;
			break;
		case IS_TYPE_RGB:
			it=TYPE_RGB_16;
			break;
		case IS_TYPE_CMYK:
			it=TYPE_CMYK_16;
			break;
		default:
			throw "Unsupported colour space (input)";
			break;
	}

	switch(GetOutputColourSpace())
	{
		case IS_TYPE_GREY:
			ot=TYPE_GRAY_16_REV;
			break;
		case IS_TYPE_RGB:
			ot=TYPE_RGB_16;
			break;
		case IS_TYPE_CMYK:
			ot=TYPE_CMYK_16;
			break;
		default:
			throw "Unsupported colour space (output)";
			break;
	}

	transform = cmsCreateTransformTHR(0,in->prof,
		it,
		(out ? out->prof : NULL),
		ot,
		CMS_GetLCMSIntent(intent), CMS_GetLCMSFlags(intent));
}


// FIXME - this will need to be updated if LCMS supports new intents at some point
// in the future...

static const char *intent_names[]=
{
	N_("Perceptual"),
	N_("Relative Colorimetric"),
	N_("Relative Colorimetric with BPC"),
	N_("Saturation"),
	N_("Absolute Colorimetric")
};

static const char *intent_descriptions[]=
{
	N_("Aims for pleasing photographic results by preserving relationships between colours, and avoiding clipping of unattainable colours"),
	N_("Attempts to preserve the original colours, relative to white-point differences."),
	N_("Attempts to preserve the original colours, relative to white-point differences.  Uses Black Point Compensation to avoid clipping of highlights and shadows."),
	N_("Attempts to provide brighter, more saturated colours, at the expense of colour relationships."),
	N_("Mimics the original colours (including white point) as closely as possible, for side-by-side comparisons.  Clips unattainable colours.")
};


static const int intent_lcmsintents[]=
{
	INTENT_PERCEPTUAL,
	INTENT_RELATIVE_COLORIMETRIC,
	INTENT_RELATIVE_COLORIMETRIC,
	INTENT_SATURATION,
	INTENT_ABSOLUTE_COLORIMETRIC
};


static const int intent_lcmsflags[]=
{
	0,
	0,
	cmsFLAGS_BLACKPOINTCOMPENSATION,
	0,
	0
};


int CMS_GetIntentCount()
{
	return(sizeof(intent_names)/sizeof(intent_names[0]));
}

const char *CMS_GetIntentName(int intent)
{
	if(intent<CMS_GetIntentCount() && intent>=0)
		return(gettext(intent_names[intent]));
	return(NULL);
}

const char *CMS_GetIntentDescription(int intent)
{
	if(intent<CMS_GetIntentCount() && intent>=0)
		return(gettext(intent_descriptions[intent]));
	return(NULL);
}


const int CMS_GetLCMSIntent(int intent)
{
	if(intent<CMS_GetIntentCount() && intent>=0)
		return(intent_lcmsintents[intent]);
	return(0);
}


const int CMS_GetLCMSFlags(int intent)
{
	if(intent<CMS_GetIntentCount() && intent>=0)
		return(intent_lcmsflags[intent]);
	return(0);
}


CMSProofingTransform::CMSProofingTransform(CMSProfile *in,CMSProfile *out,CMSProfile *proof,int proofintent,int viewintent)
{
	inputtype=in->GetColourSpace();
	if(out)
		outputtype=out->GetColourSpace();
	else
		outputtype=in->GetDeviceLinkOutputSpace();
	
	int it,ot;

	switch(inputtype)
	{
		case IS_TYPE_GREY:
			it=TYPE_GRAY_16_REV;
			break;
		case IS_TYPE_RGB:
			it=TYPE_RGB_16;
			break;
		case IS_TYPE_CMYK:
			it=TYPE_CMYK_16;
			break;
		default:
			throw "Unsupported colour space (input)";
			break;
	}

	switch(outputtype)
	{
		case IS_TYPE_GREY:
			ot=TYPE_GRAY_16_REV;
			break;
		case IS_TYPE_RGB:
			ot=TYPE_RGB_16;
			break;
		case IS_TYPE_CMYK:
			ot=TYPE_CMYK_16;
			break;
		default:
			throw "Unsupported colour space (output)";
			break;
	}

//	if(in->GetFilename())
//		cerr << "In: " << in->GetFilename() << endl;
//	cerr << "In: " << in->GetDescription() << endl;
	if(out)
	{
//		if(out->GetFilename())
//			cerr << "Out: " << out->GetFilename() << endl;
//		cerr << "Out: " << out->GetDescription() << endl;
	}
//	if(proof->GetFilename())
//		cerr << "Proof: " << proof->GetFilename() << endl;
//	cerr << "Proof: " << proof->GetDescription() << endl;

//	cerr << "Viewing intent: " << viewintent << endl;
//	cerr << "Rendering intent: " << proofintent << endl;

	transform = cmsCreateProofingTransformTHR(0,in->prof,
		it,
		(out ? out->prof : NULL),
		ot,
		proof->prof,
		CMS_GetLCMSIntent(proofintent),
		CMS_GetLCMSIntent(viewintent), CMS_GetLCMSFlags(proofintent)|cmsFLAGS_SOFTPROOFING);
}


CMSProofingTransform::CMSProofingTransform(CMSProfile *devicelink,CMSProfile *proof,int proofintent,int viewintent)
{
	inputtype=devicelink->GetColourSpace();
	outputtype=devicelink->GetDeviceLinkOutputSpace();
	int it,ot;

	switch(inputtype)
	{
		case IS_TYPE_GREY:
			it=TYPE_GRAY_16_REV;
			break;
		case IS_TYPE_RGB:
			it=TYPE_RGB_16;
			break;
		case IS_TYPE_CMYK:
			it=TYPE_CMYK_16;
			break;
		default:
			throw "Unsupported colour space (input)";
			break;
	}

	switch(outputtype)
	{
		case IS_TYPE_GREY:
			ot=TYPE_GRAY_16_REV;
			break;
		case IS_TYPE_RGB:
			ot=TYPE_RGB_16;
			break;
		case IS_TYPE_CMYK:
			ot=TYPE_CMYK_16;
			break;
		default:
			throw "Unsupported colour space (output)";
			break;
	}

//	if(devicelink->GetFilename())
//		cerr << "DeviceLink: " << devicelink->GetFilename() << endl;
//	cerr << "DeviceLink: " << devicelink->GetDescription() << endl;
//	if(proof->GetFilename())
//		cerr << "Proof: " << proof->GetFilename() << endl;
//	cerr << "Proof: " << proof->GetDescription() << endl;

//	cerr << "Viewing intent: " << viewintent << endl;
//	cerr << "Rendering intent: " << proofintent << endl;

	transform = cmsCreateProofingTransformTHR(0,devicelink->prof,
		it,
		NULL,
		ot,
		proof->prof,
		CMS_GetLCMSIntent(proofintent),
		CMS_GetLCMSIntent(viewintent), CMS_GetLCMSFlags(proofintent)|cmsFLAGS_SOFTPROOFING);
}
