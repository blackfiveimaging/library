/*
 * imagesource_pnm.h - ImageSource loader for PNM files.
 *
 * Supports high bit depths,
 * RGB data.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_PNM_H
#define IMAGESOURCE_PNM_H

#include "imagesource.h"
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined HAVE_LIBPNM || defined HAVE_LIBNETPBM

extern "C"
{
	namespace pnm {
		#ifdef HAVE_NETPBM_PAM_H
		#include <netpbm/pam.h>
		#else
		#include <pam.h>
		#endif
	}
}

using namespace std;

class ImageSource_PNM : public ImageSource
{
	public:
	ImageSource_PNM(const char *filename);
	~ImageSource_PNM();
	ISDataType *GetRow(int row);
	private:
	FILE *file;
	struct pnm::pam header;
	pnm::tuple *tuplerow;
};

#endif // HAVE_LIB...
#endif // IMAGESOURCE_PNM_H

