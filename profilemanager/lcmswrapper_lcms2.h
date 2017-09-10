/*
 * lcmswrapper.cpp - encapsulates typical "user" functions of LittleCMS,
 * providing a Profile and Transform class
 *
 * Copyright (c) 2004-2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * TODO: Report pixel type, support Lab, XYZ, etc.
 *
 */

#ifndef LCMSWRAPPER_H
#define LCMSWRAPPER_H

#ifdef HAVE_CONFIG_H
#ifndef VERSION
#include "config.h"
#endif
#endif

#include "lcms2.h"

#include "md5.h"
#include "imagesource_types.h"


class BinaryBlob;

enum LCMSWrapper_Intent
{
	LCMSWRAPPER_INTENT_NONE=-2,		// useful if user-code wants to use an "any intent will do" value when matching/caching.
	LCMSWRAPPER_INTENT_DEFAULT=-1,
	LCMSWRAPPER_INTENT_PERCEPTUAL,
	LCMSWRAPPER_INTENT_RELATIVE_COLORIMETRIC,
	LCMSWRAPPER_INTENT_RELATIVE_COLORIMETRIC_BPC,
	LCMSWRAPPER_INTENT_SATURATION,
	LCMSWRAPPER_INTENT_ABSOLUTE_COLORIMETRIC,
};


class CMSWhitePoint;
class CMSRGBPrimaries;
class CMSRGBGamma;
class CMSGamma;

class CMSProfile
{
	public:
	CMSProfile(const char *filename);
	CMSProfile(char *buffer,int length); // Should remain valid for lifetime of CMSProfile
	CMSProfile(CMSRGBPrimaries &primaries,CMSRGBGamma &gamma,CMSWhitePoint &whitepoint); // Create virtual RGB profile
	CMSProfile(CMSGamma &gamma,CMSWhitePoint &whitepoint); // Create virtual Grey profile
	CMSProfile(CMSWhitePoint &whitepoint); // Create a virtual LAB profile
	CMSProfile(IS_TYPE type=IS_TYPE_RGB); // Create a virtual sRGB / sGray profile
	CMSProfile(const CMSProfile &src); // Copy constructor
	~CMSProfile();
	enum IS_TYPE GetColourSpace();
	enum IS_TYPE GetDeviceLinkOutputSpace();
	bool IsDeviceLink();
	bool IsV4();
	const char *GetName();
	const char *GetManufacturer();
	const char *GetModel();
	const char *GetDescription();
	const char *GetInfo();
	const char *GetCopyright();
	const char *GetFilename();
	MD5Digest *GetMD5();
	BinaryBlob *GetBlob();	// Owned by caller, must be deleted when done.
	bool Save(const char *filename);
	bool operator==(const CMSProfile &other);
	protected:
	void CalcMD5FromGenerated();
	void CalcMD5FromFile();
	void CalcMD5FromMem();
	MD5Digest *md5;
	cmsHPROFILE prof;
	bool generated;	// Was this profile generated on the fly?
	char *filename;	// Only used if profile is on disk.
	char *buffer;	// Only used if profile
	int buflen;		// is loaded from memory
	friend class CMSTransform;
	friend class CMSProofingTransform;
	friend std::ostream& operator<<(std::ostream &s,CMSProfile &sp);
};


class CMSTransform
{
	public:
	CMSTransform();
	CMSTransform(CMSProfile *in,CMSProfile *out,LCMSWrapper_Intent intent=LCMSWRAPPER_INTENT_PERCEPTUAL);
	CMSTransform(CMSProfile *devicelink,LCMSWrapper_Intent intent=LCMSWRAPPER_INTENT_PERCEPTUAL);
	CMSTransform(CMSProfile *profiles[],int profilecount,LCMSWrapper_Intent intent=LCMSWRAPPER_INTENT_PERCEPTUAL);
	virtual ~CMSTransform();
	virtual void Transform(unsigned short *src,unsigned short *dst,int pixels);
	enum IS_TYPE GetInputColourSpace();
	enum IS_TYPE GetOutputColourSpace();
	protected:
	virtual void MakeTransform(CMSProfile *in,CMSProfile *out,LCMSWrapper_Intent intent);
	enum IS_TYPE inputtype;
	enum IS_TYPE outputtype;
	cmsHTRANSFORM transform;
};


class CMSProofingTransform : public CMSTransform
{
	public:
	CMSProofingTransform(CMSProfile *in,CMSProfile *out,CMSProfile *proof,int proofintent=INTENT_PERCEPTUAL,int viewintent=INTENT_ABSOLUTE_COLORIMETRIC);
	CMSProofingTransform(CMSProfile *devicelink,CMSProfile *proof,int proofintent=INTENT_PERCEPTUAL,int viewintent=INTENT_ABSOLUTE_COLORIMETRIC);
};


class CMSWhitePoint
{
	public:
	CMSWhitePoint(int degk)
	{
		cmsWhitePointFromTemp(&whitepoint,degk);
	}
	protected:
	cmsCIExyY whitepoint;
	friend class CMSProfile;
};


class CMSRGBPrimaries : public cmsCIExyYTRIPLE
{
	public:
	CMSRGBPrimaries()
	{
	}
	CMSRGBPrimaries(float rx,float ry,float gx,float gy,float bx,float by)
	{
		Red.x=rx;
		Red.y=ry;
		Red.Y=1.0;
		Green.x=gx;
		Green.y=gy;
		Green.Y=1.0;
		Blue.x=bx;
		Blue.y=by;
		Blue.Y=1.0;	
	}
	protected:
	friend class CMSProfile;
};


class CMSGamma
{
	public:
	CMSGamma(float gamma, bool sRGB=false)
	{
		if(sRGB)
		{
			throw "FIXME: sRGB gamma curve not yet supported.";
//			double params[]={2.4,1.0/1.055,0.055/1.055,1.0/12.92,0.04045};
//			gammatable=cmsBuildParametricGamma(256,4,params);
		}
//                    Y = (aX + b)^Gamma | X >= d
//                    Y = cX             | X < d
//a=1/1.055;
//b=0.055/1.055;
//c=1.0/12.92
		else
			gammatable=cmsBuildGamma(NULL,gamma);
	}
	~CMSGamma()
	{
		cmsFreeToneCurve(gammatable);
	}
	cmsToneCurve *GetGammaTable()
	{
		return(gammatable);
	}
	protected:
	cmsToneCurve *gammatable;
	friend class CMSProfile;
	friend class CMSRGBGamma;
};

class CMSRGBGamma
{
	public:
	CMSRGBGamma(float rgamma,float ggamma,float bgamma)
		: redgamma(rgamma),greengamma(ggamma),bluegamma(bgamma)
	{
		gammatables[0]=redgamma.GetGammaTable();
		gammatables[1]=greengamma.GetGammaTable();
		gammatables[2]=bluegamma.GetGammaTable();
	}
	CMSRGBGamma(float gamma)
		: redgamma(gamma),greengamma(gamma),bluegamma(gamma)
	{
		gammatables[0]=redgamma.GetGammaTable();
		gammatables[1]=greengamma.GetGammaTable();
		gammatables[2]=bluegamma.GetGammaTable();
	}
	protected:
	CMSGamma redgamma,greengamma,bluegamma;
	cmsToneCurve *gammatables[3];
	friend class CMSProfile;
};


extern CMSRGBPrimaries CMSPrimaries_Rec709;
extern CMSRGBPrimaries CMSPrimaries_Adobe;
extern CMSRGBPrimaries CMSPrimaries_NTSC;
extern CMSRGBPrimaries CMSPrimaries_EBU;
extern CMSRGBPrimaries CMSPrimaries_SMPTE;
extern CMSRGBPrimaries CMSPrimaries_HDTV;
extern CMSRGBPrimaries CMSPrimaries_CIE;


int CMS_GetIntentCount();
const char *CMS_GetIntentName(int intent);
const char *CMS_GetIntentDescription(int intent);
const int CMS_GetLCMSIntent(int intent);
const int CMS_GetLCMSFlags(int intent);

#endif
