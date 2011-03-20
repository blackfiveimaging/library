AM_CXXFLAGS = -Wall -I../support/ -I../imagesource/ -I../ -I../pixbufthumbnail -I../imagesource -I../profilemanager $(LCMS_CFLAGS) $(GTK2_CFLAGS) ${PNM_CFLAGS}

noinst_LTLIBRARIES = liblayoutsupport.la

liblayoutsupport_la_SOURCES =	\
	layoutrectangle.cpp	\
	layoutrectangle.h	\
	pageextent.h	\
	signature.cpp	\
	signature.h	\
	circlemontage.cpp \
	circlemontage.h

liblayoutsupport_la_LDFLAGS = -static

