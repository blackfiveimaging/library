AM_CXXFLAGS = -Wall -I../imagesource/ -I../ -I../pixbufthumbnail -I../imagesource -I../profilemanager $(LCMS_CFLAGS) $(GTK2_CFLAGS) ${PNM_CFLAGS}

noinst_LTLIBRARIES = libargyllsupport.la

libargyllsupport_la_SOURCES =	\
	argyllbg.h \
	blackgenselector.cpp \
	blackgenselector.h \
	viewingcondselector.cpp \
	viewingcondselector.h

libargyllsupport_la_LDFLAGS = -static

