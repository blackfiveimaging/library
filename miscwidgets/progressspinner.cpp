/*
 * progressspinner.cpp - A subclass of Progress; this provides an animated "spinner"
 * to provide feedback that something is happening.
 *
 * Copyright (c) 2009 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include <gtk/gtk.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkstock.h>

#include "progressspinner.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;


ProgressSpinner::ProgressSpinner()
	: Progress(), Spinner(), frame(0)
{
	DoProgress(0,0);
}


ProgressSpinner::~ProgressSpinner()
{
}


bool ProgressSpinner::DoProgress(int i,int maxi)
{
	++frame;
	SetFrame(frame);
}

