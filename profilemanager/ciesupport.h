#ifndef CIESUPPORT_H
#define CIESUPPORT_H

class XYZValue
{
	public:
	XYZValue() : X(0),Y(0),Z(0) {}
	XYZValue(float X,float Y,float Z) : X(X),Y(Y),Z(Z) {}
	~XYZValue() {}
	float X,Y,Z;
};

extern XYZValue D50ReferenceWhite;
extern XYZValue D65ReferenceWhite;


class LabValue
{
	public:
	LabValue();
	LabValue(XYZValue &xyz,XYZValue &refwhite=D50ReferenceWhite);
	LabValue(float L, float a, float b);
	float L,a,b;
};

#endif

