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
	ReCalc();
}


void Signature::ReCalc()
{
  int w,h;
  w=pagewidth-(leftmargin+rightmargin);
  h=pageheight-(topmargin+bottommargin);
  celwidth=(w-(columns-1)*hgutter); celwidth/=columns;
  celheight=(h-(rows-1)*vgutter); celheight/=rows;
}


void Signature::SetPageExtent(PageExtent &pe)
{
	PageExtent::SetPageExtent(pe);
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
	ReCalc();
}


void Signature::SetGutters(int hgutter,int vgutter)
{
  if(((columns-1)*hgutter)>=(pagewidth-(leftmargin+rightmargin)))
    cerr << "Horizontal gutters too wide!" << endl;
  else
    this->hgutter=hgutter;
  
  if(((rows-1)*vgutter)>=(pageheight-(topmargin+bottommargin)))
    cerr << "Horizontal gutters too tall!" << endl;
  else
    this->vgutter=vgutter;
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
	int r=(pageheight-(topmargin+bottommargin))/celheight;
	int c=(pagewidth-(leftmargin+rightmargin))/celwidth;
	if(r<1)
		celheight=pageheight-(topmargin+bottommargin);
	if(c<1)
		celwidth=pagewidth-(leftmargin+rightmargin);

	if(r>1)
	{
		vgutter=((pageheight-(topmargin+bottommargin))-r*celheight)/(r-1);
	}
	if(c>1)
	{
		hgutter=((pagewidth-(leftmargin+rightmargin))-c*celwidth)/(c-1);
	}
}


void Signature::SetCellWidth(int width)
{
	celwidth=width;
	ReCalcByCellSize();
}


void Signature::SetCellHeight(int height)
{
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
	rows(rows), columns(columns)
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
