AM_CXXFLAGS = -Wall -I../imagesource/ -I../pixbufthumbnail -I../profilemanager -I../support $(LCMS_CFLAGS) $(GTK2_CFLAGS) ${PNM_CFLAGS}

noinst_LTLIBRARIES = libmiscwidgets.la

libmiscwidgets_la_SOURCES =	\
	generaldialogs.cpp	\
	generaldialogs.h	\
	errordialogqueue.cpp	\
	errordialogqueue.h	\
	simplecombo.cpp	\
	simplecombo.h	\
	\
	colorantselector.cpp	\
	colorantselector.h	\
	coloranttoggle.cpp	\
	coloranttoggle.h	\
	\
	pixbufview.cpp \
	pixbufview.h \
	\
	uitab.cpp \
	uitab.h \
	\
	patheditor.cpp	\
	patheditor.h	\
	imageselector.cpp \
	imageselector.h \
	imageselector_noborder_image.h

libmiscwidgets_la_LDFLAGS = -static

EXTRA_DIST = squiggledata.cpp

