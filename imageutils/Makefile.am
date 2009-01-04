AM_CXXFLAGS = -Wall -I../imagesource/ -I../pixbufthumbnail -I../profilemanager -I../support $(LCMS_CFLAGS) $(GTK2_CFLAGS) ${PNM_CFLAGS}

noinst_LTLIBRARIES = libimageutils.la

libimageutils_la_SOURCES =	\
	maskpixbuf.cpp	\
	maskpixbuf.h	\
	rotatepixbuf.cpp	\
	rotatepixbuf.h	\
	\
	jpegsave.cpp	\
	jpegsave.h	\
	tiffsave.cpp	\
	tiffsave.h

libimageutils_la_LDFLAGS = -static

