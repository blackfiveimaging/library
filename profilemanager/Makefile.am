
localedir=$(datadir)/locale

AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -Wall -I../imagesource/ -I../support/ $(LCMS_CFLAGS) $(GTK2_CFLAGS)

noinst_LTLIBRARIES = libprofilemanager.la

libprofilemanager_la_SOURCES =	\
	lcmswrapper.cpp	\
	lcmswrapper.h	\
	profilemanager.cpp	\
	profilemanager.h	\
	profileselector.cpp	\
	profileselector.h	\
	intentselector.cpp	\
	intentselector.h	\
	ciesupport.cpp \
	ciesupport.h \
	naivetransforms.h

libprofilemanager_la_LDFLAGS = -static

