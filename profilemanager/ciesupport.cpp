#include <iostream>
#include <math.h>

#include "ciesupport.h"

using namespace std;

XYZValue D50ReferenceWhite(96.42,100.00,82.49);
XYZValue D65ReferenceWhite(95.05,100.00,108.90);


LabValue::LabValue() : L(0.0),a(0.0),b(0.0)
{
}

LabValue::LabValue(double L, double a, double b) : L(L),a(a),b(b)
{
}

#define kappa 903.3
#define epsilon 0.008856

LabValue::LabValue(XYZValue &xyz,XYZValue &refwhite) : L(0.0), a(0.0), b(0.0)
{
	double x_r=xyz.X/refwhite.X;
	double y_r=xyz.Y/refwhite.Y;
	double z_r=xyz.Z/refwhite.Z;

	double fx,fy,fz;
	if(x_r<=epsilon)
		fx=(kappa*x_r+16.0)/116.0;
	else
		fx=pow(x_r,1.0/3.0);
	if(y_r<=epsilon)
		fy=(kappa*y_r+16.0)/116.0;
	else
		fy=pow(y_r,1.0/3.0);
	if(z_r<=epsilon)
		fz=(kappa*z_r+16.0)/116.0;
	else
		fz=pow(z_r,1.0/3.0);

	L=116.0*fy-16.0;
	a=500.0*(fx-fy);
	b=200.0*(fy-fz);
}


LabValue::LabValue(const LabValue &lab)
{
	L=lab.L;
	a=lab.a;
	b=lab.b;
}


double LabValue::Magnitude()
{
	double result=sqrt(L*L+a*a+b*b);
	return(result);
}


double LabValue::dE(const LabValue &other)
{
	double dL=other.L-L;
	double da=other.a-a;
	double db=other.b-b;
	double result=sqrt(dL*dL+da*da+db*db);
	return(result);
}


LabValue &LabValue::operator-=(const LabValue &other)
{
	L-=other.L;
	a-=other.a;
	b-=other.b;
	return(*this);
}


LabValue &LabValue::operator=(const LabValue &other)
{
	L=other.L;
	a=other.a;
	b=other.b;
	return(*this);
}



LabValue LabValue::operator-(const LabValue &other)
{
	LabValue result(*this);
	result-=other;
	return(result);
}


std::ostream& operator<<(std::ostream &s,LabValue &l)
{
	s << "(" << l.L << ", " << l.a << ", " << l.b << ")";
}


