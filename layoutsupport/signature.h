/*
 * signature.h - class to handle coordinate calculations for n-up rectangular layouts
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "pageextent.h"
#include "layoutrectangle.h"

#define DEFAULTGUTTER 15


class LayoutRectangle;

class Signature : public virtual PageExtent
{
	public:
	Signature(int rows=1,int columns=1);
	Signature(PageExtent &extent,int rows=1,int columns=1);
	virtual ~Signature()
	{
	}
	virtual void SetPageExtent(PageExtent &pe);
	virtual void SetPaperSize(int width,int height);
	virtual void SetMargins(int left,int right,int top,int bottom);
	virtual void SetGutters(int hgutter,int vgutter);
	virtual void SetHGutter(int gutter);
	virtual void SetVGutter(int gutter);
	virtual void SetColumns(int columns);
	virtual void SetRows(int rows);
	virtual void SetCellWidth(int width);
	virtual void SetCellHeight(int height);
	virtual int GetCellWidth();
	virtual int GetCellHeight();
	virtual int GetColumns();
	virtual int GetRows();
	virtual int GetHGutter();
	virtual int GetVGutter();
	virtual bool GetAbsolute();
	virtual LayoutRectangle *GetLayoutRectangle(int row,int column);
	virtual void EqualiseMargins();
	virtual void ReCalc();
	virtual void ReCalcByCellSize();
	virtual int ColumnAt(int xpos);
	virtual int RowAt(int ypos);
	protected:
	int hgutter,vgutter;
	int rows,columns;
	float celwidth,celheight;
	int rightpadding,bottompadding;
	bool absolutemode;				// Used to track whether we're recalcing in terms of rows/columns or cell size.
};

#endif
