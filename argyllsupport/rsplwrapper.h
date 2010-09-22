#ifndef RSPLWRAPPER_H
#define RSPLWRAPPER_H

extern "C"
{
#include "rspl.h"
}


class RSPLWrapper
{
	public:
	RSPLWrapper(int ind, int outd) : interp(NULL), ind(ind), outd(outd)
	{
		interp=new_rspl(0,ind,outd); // Create a new RSPL structure
		for(int i=0;i<ind;++i)
		{
			gres[i]=16;
			avgdev[i]=0.1;
		}
	}
	virtual ~RSPLWrapper()
	{
		if(interp)
			interp->del(interp);
	}
	virtual bool Populate(int count, double *in, double *out,double smoothing)
	{
		cow *myco=new cow[count];
		datai ilow,ihigh;
		datao olow,ohigh;

		for(int i=0;i<ind;++i)
		{
			ilow[i]=ihigh[i]=in[i];
		}
		for(int i=0;i<outd;++i)
		{
			olow[i]=ohigh[i]=out[i];
		}

		for(int i=0;i<count;++i)
		{
			for(int j=0;j<ind;++j)
			{
				myco[i].p[j]=in[i*ind+j];
				if(in[i*ind+j]<ilow[j]) ilow[j]=in[i*ind+j];
				if(in[i*ind+j]>ihigh[j]) ihigh[j]=in[i*ind+j];
			}
			for(int j=0;j<outd;++j)
			{
				myco[i].v[j]=out[i*outd+j];
				if(out[i*outd+j]<olow[j]) olow[j]=out[i*outd+j];
				if(out[i*outd+j]>ohigh[j]) ohigh[j]=out[i*outd+j];
			}
			myco[i].w=1.0;
		}
		// Increase weights of endpoints...
		myco[0].w=10.0;
		myco[count-1].w=10.0;

		// Add some "headroom"
		for(int j=0;j<ind;++j)
		{
			ilow[j]-=1.0;
			ihigh[j]+=1.0;
		}

		for(int j=0;j<outd;++j)
		{
			olow[j]-=1.0;
			ohigh[j]+=1.0;
		}

		interp->fit_rspl_w(interp,
		           RSPL_EXTRAFIT,		/* Non-mon and clip flags */
		           myco,				/* Test points */
		           count,				/* Number of test points */
		           ilow, ihigh, gres,	/* Low, high, resolution of grid */
		           olow, ohigh,			/* Default data scale */
		           smoothing,			/* Smoothing */
		           avgdev,				/* Average deviation */
		           NULL);				/* iwidth */
		delete[] myco;
		return(true);
	}

	virtual void Interp(double *in, double *out)
	{
		co point;
		for(int i=0;i<ind;++i)
			point.p[i]=*in++;
		for(int i=0;i<outd;++i)
			point.v[i]=0.0;
		interp->interp(interp,&point);
		for(int i=0;i<outd;++i)
			*out++=point.v[i];
	}

	virtual bool ReverseInterp(double *out, double *in)
	{
		co point;
		for(int i=0;i<ind;++i)
			point.p[i]=0;
		for(int i=0;i<outd;++i)
			point.v[i]=*out++;
		int result=interp->rev_interp(interp,RSPL_WILLCLIP|RSPL_NEARCLIP,1,NULL,NULL,&point);
		for(int i=0;i<outd;++i)
			*in++=point.p[i];
		if(result==0)
		{
			Debug[TRACE] << "ReverseInterp: no solutions found" << endl;
			return(false);
		}
		return(true);
	}
	protected:
	rspl *interp;
	int ind,outd;
	int gres[MXDI];
	double avgdev[MXDO];
};

#endif

