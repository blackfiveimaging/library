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

#include "debug.h"

using namespace std;

void Signature::EqualiseMargins()
{
	PageExtent::EqualiseMargins();
	ReCalc();
}


void Signature::ReCalc()
{
	if(absolutemode)
		ReCalcByCellSize();
	else
	{
		int w,h;
		w=pagewidth-(leftmargin+rightmargin);
		h=pageheight-(topmargin+bottommargin);
		celwidth=(w-(columns-1)*hgutter); celwidth/=columns;
		celheight=(h-(rows-1)*vgutter); celheight/=rows;
		rightpadding=bottompadding=0;
	}
}


void Signature::SetPageExtent(PageExtent &pe)
{
	PageExtent::SetPageExtent(pe);
	ReCalc();
}


void Signature::SetPaperSize(int width,int height)
{
  if(((columns-1)*hgutter)>=(width-(leftmargin+rightmargin)))
    Debug[WARN] << "New papersize too narrow!" << endl;
  else
    pagewidth=width;
  
  if(((rows-1)*vgutter)>=(height-(topmargin+bottommargin)))
    Debug[WARN] << "New papersize too short!" << endl;
  else
    pageheight=height;
}


void Signature::SetMargins(int left,int right,int top,int bottom)
{
	PageExtent::SetMargins(left,right,top,bottom);
	ReCalc();
}


void Signature::SetGutters(int hgutter,int vgutter)
{
	Debug[TRACE] << "Setting gutters to :" << hgutter << ", " << vgutter << endl;
	if(((columns-1)*hgutter)>=(pagewidth-(leftmargin+rightmargin)))
		Debug[WARN] << "Horizontal gutters too wide!" << endl;
	else
		this->hgutter=hgutter;

	if(((rows-1)*vgutter)>=(pageheight-(topmargin+bottommargin)))
		Debug[WARN] << "Horizontal gutters too tall!" << endl;
	else
		this->vgutter=vgutter;
	ReCalc();
	Debug[TRACE] << "After recalc: " << hgutter << ", " << vgutter << endl;
}


void Signature::SetHGutter(int gutter)
{
	Debug[TRACE] << "Setting HGutter to :" << gutter << endl;
	if(((columns-1)*gutter)>=(pagewidth-(leftmargin+rightmargin)))
		Debug[WARN] << "Horizontal gutters too wide!" << endl;
	else
		this->hgutter=gutter;
	ReCalc();
}
  

void Signature::SetVGutter(int gutter)
{
	Debug[TRACE] << "Setting VGutter to :" << gutter << endl;
	if(((rows-1)*gutter)>=(pageheight-(topmargin+bottommargin)))
		Debug[WARN] << "Vertical gutters too wide!" << endl;
	else
		this->vgutter=gutter;
	ReCalc();
}
  
  
void Signature::SetColumns(int columns)
{
	Debug[TRACE] << "Setting Columns to :" << columns << endl;
	absolutemode=false;
	if(((columns-1)*hgutter)>=(pagewidth-(leftmargin+rightmargin)))
		Debug[WARN] << "Too many columns!" << endl;
	else
		this->columns=columns;
	ReCalc();
}


void Signature::SetRows(int rows)
{
	Debug[TRACE] << "Setting Columns to :" << rows << endl;
	absolutemode=false;
	if(((rows-1)*vgutter)>=(pageheight-(topmargin+bottommargin)))
		Debug[WARN] << "Too many rows!" << endl;
	else
		this->rows=rows;
	ReCalc();
}


void Signature::ReCalcByCellSize()
{
	int r=(pageheight-(topmargin+bottommargin))/celheight;
	int c=(pagewidth-(leftmargin+rightmargin))/celwidth;
	Debug[TRACE] << "Rows: " << r << ", cols: " << c << endl;
	Debug[TRACE] << "Page width: " << pagewidth << ", margins: " << (topmargin+bottommargin) << ", celwidth:" << celwidth << endl;
	if(r<1)
	{
		celheight=pageheight-(topmargin+bottommargin);
		r=1;
	}
	else if(r>1)
	{
		int newvgutter=((pageheight-(topmargin+bottommargin))-r*celheight)/(r-1);
		Debug[TRACE] << "Old VGutter = " << vgutter << ", new VGutter = " << newvgutter << endl;
		if(newvgutter<vgutter)
			vgutter=newvgutter;
	}
	bottompadding=pageheight-(topmargin+bottommargin+r*celheight+(r-1)*vgutter);

	if(c<1)
	{
		celwidth=pagewidth-(leftmargin+rightmargin);
		c=1;
	}
	else if(c>1)
	{
		int newhgutter=((pagewidth-(leftmargin+rightmargin))-c*celwidth)/(c-1);
		Debug[TRACE] << "Old HGutter = " << hgutter << ", new HGutter = " << newhgutter << endl;
		if(newhgutter<hgutter)
			hgutter=newhgutter;
	}
	rightpadding=pagewidth-(leftmargin+rightmargin+c*celwidth+(c-1)*hgutter);

	rows=r;
	columns=c;
}


void Signature::SetCellWidth(int width)
{
	absolutemode=true;
	Debug[TRACE] << "Setting cell width to " << width << endl;
	celwidth=width;
	ReCalcByCellSize();
}


void Signature::SetCellHeight(int height)
{
	absolutemode=true;
	Debug[TRACE] << "Setting cell height to " << height << endl;
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
	Debug[TRACE] << "GetCellWidth - returning: " << celwidth << endl;
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
	Debug[TRACE] << "GetHGutter returning: " << hgutter << endl;
	return(hgutter);
}

int Signature::GetVGutter()
{
	Debug[TRACE] << "GetVGutter returning: " << vgutter << endl;
	return(vgutter);
}

bool Signature::GetAbsolute()
{
	return(absolutemode);
}

