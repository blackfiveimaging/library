AM_CXXFLAGS = -Wall -I../imagesource/ -I../ -I../pixbufthumbnail -I../imagesource -I../profilemanager $(LCMS_CFLAGS) $(GTK2_CFLAGS) ${PNM_CFLAGS}

noinst_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES =	\
	binarysearch.h \
	\
	consumer.cpp	\
	consumer.h	\
	configdb.cpp	\
	configdb.h	\
	\
	layoutrectangle.cpp	\
	layoutrectangle.h	\
	pageextent.h	\
	signature.cpp	\
	signature.h	\
	circlemontage.cpp \
	circlemontage.h \
	\
	jobqueue.h \
	pathsupport.cpp \
	pathsupport.h \
	progress.h	\
	progresstext.h \
	progressthread.h \
	progressbar.cpp	\
	progressbar.h	\
	\
	rangeparser.cpp \
	rangeparser.h \
	\
	thread.cpp \
	thread.h \
	ptmutex.cpp \
	ptmutex.h \
	rwmutex.cpp \
	rwmutex.h \
	multex.cpp \
	multex.h \
	threadutil.h \
	threadevent.cpp \
	threadevent.h \
	\
	searchpath.cpp \
	searchpath.h \
	searchpathdbhandler.h \
	dirtreewalker.cpp \
	dirtreewalker.h \
	\
	tempfile.cpp	\
	tempfile.h	\
	\
	md5.cpp \
	md5.h \
	util.cpp \
	util.h

libsupport_la_LDFLAGS = -static

