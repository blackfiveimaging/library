AM_CXXFLAGS = -Wall -I../imagesource/ -I../pixbufthumbnail -I../profilemanager -I../support $(LCMS_CFLAGS) $(GTK2_CFLAGS) ${PNM_CFLAGS}

noinst_LTLIBRARIES = libmiscwidgets.la

libmiscwidgets_la_SOURCES =	\
	generaldialogs.cpp	\
	generaldialogs.h	\
	simplecombo.cpp	\
	simplecombo.h	\
	\
	pixbufview.cpp \
	pixbufview.h \
	\
	patheditor.cpp	\
	patheditor.h	\
	imageselector.cpp \
	imageselector.h \
	imageselector_noborder_image.h

libsupport_la_LDFLAGS = -static

