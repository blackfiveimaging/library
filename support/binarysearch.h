#ifndef BINARYSEARCH_H
#define BINARYSEARCH_H

// Binary Search pure virtual base class - for finding a solution to a continuous
// function.

// Subclass this, and provide an Evaluate function.

// Use BINARYSEARCHTYPE_ASCENDING if the Evaluate function increases with input,
// and BINARYSEARCHTYPE_DESCENDING otherwise

enum BinarySearchType {BINARYSEARCHTYPE_ASCENDING,BINARYSEARCHTYPE_DESCENDING};

class BinarySearch
{
	public:
	BinarySearch(enum BinarySearchType type=BINARYSEARCHTYPE_ASCENDING) : type(type)
	{
	}
	virtual ~BinarySearch()
	{
	}
	virtual double Search(double low,double high,double target)
	{
		double result=(low+high)/2.0;
		double val=Evaluate(result);
		double d=(val-target)*(val-target);
		if(d>0.00001 && (high-low)>0.0001)
		{
			// To reverse the sense of this comparison for the Descending case
			// we just multiply each operand by -1
			double m=1.0;
			if(type==BINARYSEARCHTYPE_DESCENDING)
				m=-1.0;
			if((m*val)>(m*target))
			{
				low=result;
			}
			else
			{
				high=result;
			}
			result=Search(low,high,target);
		}	
		return(result);	
	}
	virtual double Evaluate(double input)=0;
	protected:
	BinarySearchType type;
};

#endif

