/*
 * signature.h - class to handle coordinate calculations for n-up rectangular layouts
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include "layoutrectangle.h"
#include "signature.h"

using namespace std;

void Signature::EqualiseMargins()
{
	PageExtent::EqualiseMargins();
	if(absolutemode)
		ReCalcByCellSize();
	else
		ReCalc();
}


void Signature::ReCalc()
{
	absolutemode=false;
	int w,h;
	w=pagewidth-(leftmargin+rightmargin);
	h=pageheight-(topmargin+bottommargin);
	celwidth=(w-(columns-1)*hgutter); celwidth/=columns;
	celheight=(h-(rows-1)*vgutter); celheight/=rows;
	rightpadding=bottompadding=0;
}


void Signature::SetPageExtent(PageExtent &pe)
{
	PageExtent::SetPageExtent(pe);
	if(absolutemode)
		ReCalcByCellSize();
	else
		ReCalc();
}


void Signature::SetPaperSize(int width,int height)
{
  if(((columns-1)*hgutter)>=(width-(leftmargin+rightmargin)))
    cerr << "New papersize too narrow!" << endl;
  else
    pagewidth=width;
  
  if(((rows-1)*vgutter)>=(height-(topmargin+bottommargin)))
    cerr << "New papersize too short!" << endl;
  else
    pageheight=height;
}


void Signature::SetMargins(int left,int right,int top,int bottom)
{
	PageExtent::SetMargins(left,right,top,bottom);
	if(absolutemode)
		ReCalcByCellSize();
	else
		ReCalc();
}


void Signature::SetGutters(int hgutter,int vgutter)
{
	cerr << "Setting gutters to :" << hgutter << ", " << vgutter << endl;
	if(((columns-1)*hgutter)>=(pagewidth-(leftmargin+rightmargin)))
		cerr << "Horizontal gutters too wide!" << endl;
	else
		this->hgutter=hgutter;

	if(((rows-1)*vgutter)>=(pageheight-(topmargin+bottommargin)))
		cerr << "Horizontal gutters too tall!" << endl;
	else
		this->vgutter=vgutter;
	ReCalc();
	cerr << "After recalc: " << hgutter << ", " << vgutter << endl;
}


void Signature::SetHGutter(int gutter)
{
	cerr << "Setting HGutter to :" << gutter << endl;
	if(((columns-1)*gutter)>=(pagewidth-(leftmargin+rightmargin)))
		cerr << "Horizontal gutters too wide!" << endl;
	else
		this->hgutter=gutter;
	ReCalc();
}
  

void Signature::SetVGutter(int gutter)
{
	cerr << "Setting VGutter to :" << gutter << endl;
	if(((rows-1)*gutter)>=(pageheight-(topmargin+bottommargin)))
		cerr << "Vertical gutters too wide!" << endl;
	else
		this->vgutter=gutter;
	ReCalc();
}
  
  
void Signature::SetColumns(int columns)
{
  if(((columns-1)*hgutter)>=(pagewidth-(leftmargin+rightmargin)))
    cerr << "Too many columns!" << endl;
  else
    this->columns=columns;
  ReCalc();
}


void Signature::SetRows(int rows)
{
  if(((rows-1)*vgutter)>=(pageheight-(topmargin+bottommargin)))
    cerr << "Too many rows!" << endl;
  else
    this->rows=rows;
  ReCalc();
}

void Signature::ReCalcByCellSize()
{
	absolutemode=true;
	int r=(pageheight-(topmargin+bottommargin))/celheight;
	int c=(pagewidth-(leftmargin+rightmargin))/celwidth;
	cerr << "Rows: " << r << ", cols: " << c << endl;
	cerr << "Page width: " << pagewidth << ", margins: " << (topmargin+bottommargin) << ", celwidth:" << celwidth << endl;
	if(r<1)
	{
		celheight=pageheight-(topmargin+bottommargin);
		r=1;
	}
	else if(r>1)
	{
		vgutter=((pageheight-(topmargin+bottommargin))-r*celheight)/(r-1);
		cerr << "VGutter = " << vgutter << endl;
	}
	else
	{
		bottompadding=pageheight-(topmargin+bottommargin+celheight);
	}

	if(c<1)
	{
		celwidth=pagewidth-(leftmargin+rightmargin);
		c=1;
	}
	else if(c>1)
	{
		hgutter=((pagewidth-(leftmargin+rightmargin))-c*celwidth)/(c-1);
		cerr << "HGutter = " << hgutter << endl;
	}
	else
	{
		rightpadding=pagewidth-(leftmargin+rightmargin+celwidth);
	}

	rows=r;
	columns=c;
}


void Signature::SetCellWidth(int width)
{
	cerr << "Setting cell width to " << width << endl;
	celwidth=width;
	ReCalcByCellSize();
}


void Signature::SetCellHeight(int height)
{
	cerr << "Setting cell height to " << height << endl;
	celheight=height;
	ReCalcByCellSize();
}


Signature::Signature(int rows,int columns)
	: PageExtent(), hgutter(DEFAULTGUTTER), vgutter(DEFAULTGUTTER),
	rows(rows), columns(columns)
{
	ReCalc();
}


Signature::Signature(PageExtent &extent,int rows,int columns)
	: PageExtent(), hgutter(DEFAULTGUTTER), vgutter(DEFAULTGUTTER),
	rows(rows), columns(columns), rightpadding(0), bottompadding(0), absolutemode(false)
{
	SetPageExtent(extent);
}


int Signature::ColumnAt(int xpos)
{
	xpos-=leftmargin;
	int c=int(xpos/(celwidth+hgutter));
	return(c);
}


int Signature::RowAt(int ypos)
{
	ypos-=topmargin;
	int c=int(ypos/(celheight+vgutter));
	return(c);
}


LayoutRectangle *Signature::GetLayoutRectangle(int row,int column)
{
	int x=int(leftmargin+column*(hgutter+celwidth)+0.5);
	int y=int(topmargin+row*(vgutter+celheight)+0.5);
	int w=int(celwidth);
	int h=int(celheight);
	return(new LayoutRectangle(x,y,w,h));
}


int Signature::GetCellWidth()
{
	cerr << "GetCellWidth - returning: " << celwidth << endl;
	return(celwidth);
}


int Signature::GetCellHeight()
{
	return(celheight);
}


int Signature::GetColumns()
{
	return(columns);
}


int Signature::GetRows()
{
	return(rows);
}


int Signature::GetHGutter()
{
	cerr << "GetHGutter returning: " << hgutter << endl;
	return(hgutter);
}

int Signature::GetVGutter()
{
	cerr << "GetVGutter returning: " << vgutter << endl;
	return(vgutter);
}

