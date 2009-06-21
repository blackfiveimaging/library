#ifndef CIESUPPORT_H
#define CIESUPPORT_H

// Base types for handling XYZ and CIEL*ab values.

class XYZValue
{
	public:
	XYZValue() : X(0),Y(0),Z(0) {}
	XYZValue(double X,double Y,double Z) : X(X),Y(Y),Z(Z) {}
	~XYZValue() {}
	double X,Y,Z;
};

extern XYZValue D50ReferenceWhite;
extern XYZValue D65ReferenceWhite;


class LabValue
{
	public:
	LabValue();
	LabValue(XYZValue &xyz,XYZValue &refwhite=D50ReferenceWhite);
	LabValue(const LabValue &lab);
	LabValue(double L, double a, double b);
	LabValue operator-(const LabValue &other);
	LabValue &operator-=(const LabValue &other);
	LabValue &operator=(const LabValue &other);
	double Magnitude();	// sqrt(L*L + a*a + b*b);
	double dE(const LabValue &other);	// Euclidean distance between this and other
	double L,a,b;
};

#endif

