#ifndef GPDEVICENSUPPORT_H
#define GPDEVICENSUPPORT_H

#include <gutenprint/gutenprint.h>
#include "gp_cppsupport/gprintersettings.h"
#include "imagesource/devicencolorant.h"
#include "imagesource/imagesource_devicen_preview.h"

// List header subclass - not strictly necessary,
// but avoids the need for untidy casts elsewhere.
class GPColorantList : public DeviceNColorantList
{
	public:
	GPColorantList(stp_vars_t *vars);
	GPColorantList(GPrinterSettings *gp);
	virtual ~GPColorantList();
	// Virtual so we can override with a version which provides greys for dropsize calibration
	virtual void BuildList(stp_vars_t *vars=NULL);
	protected:
	void BuildList_Raw(stp_vars_t *vars=NULL);
	stp_vars_t *cachedvars;
};


int GetPrinterColorantCount(stp_vars_t *vars);

#endif

