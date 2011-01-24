#ifndef IMAGESOURCE_PARASITE_H
#define IMAGESOURCE_PARASITE_H

#include "binaryblob.h"
#include "debug.h"

enum ISParasiteType {ISPARATYPE_UNKNOWN, ISPARATYPE_PSIMAGERESOURCEBLOCK};
enum ISParasiteApplicability {ISPARA_UNIVERSAL=-1, ISPARA_UNKNOWN=0, ISPARA_TIFF=1, ISPARA_JPEG=2};
class ISParasite : public BinaryBlob
{
	public:
	ISParasite(const char *buf,int bufsize,ISParasiteType type,ISParasiteApplicability applic=ISPARA_UNIVERSAL)
		: BinaryBlob(buf,bufsize), type(type), applic(applic)
	{
	}
	virtual ~ISParasite()
	{
	}
	// Query a parasite for applicability.  For example, the TIFF saver will query if(para.Applicable(ISPARA_TIFF))
	// Returns true if either the parasite or the calling app specifies "universal",
	// Otherwise uses a straightforward bitmask.
	bool Applicable(ISParasiteApplicability app)
	{
		Debug[TRACE] << "*** Testing for parasite applicability " << app << " vs. " << applic << std::endl;
		if(applic==ISPARA_UNIVERSAL || app==ISPARA_UNIVERSAL || applic&app)
			return(true);
		else
			return(false);
	}
	protected:
	ISParasiteType type;
	ISParasiteApplicability applic;
};

#endif

