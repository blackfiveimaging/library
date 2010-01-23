#ifndef PP_PROGRESSSPIN_H
#define PP_PROGRESSSPIN_H

#include "progress.h"
#include "spinner.h"

class ProgressSpinner : public Progress, public Spinner
{
	public:
	ProgressSpinner() : Progress(), Spinner(), frame(0)
	{
	}
	virtual ~ProgressSpinner()
	{
	}
	bool DoProgress(int i,int maxi)
	{
		++frame;
		SetFrame(frame);
		return(true);
	}
	protected:
	int frame;
};

#endif
