AM_CXXFLAGS = -Wall -I../imagesource/ -I../ -I../pixbufthumbnail -I../imagesource -I../profilemanager $(LCMS_CFLAGS) $(GTK2_CFLAGS) ${PNM_CFLAGS}

noinst_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES =	\
	binarysearch.h \
	binaryblob.h \
	\
	consumer.cpp	\
	consumer.h	\
	configdb.cpp	\
	configdb.h	\
	\
	jobqueue.h \
	pathsupport.cpp \
	pathsupport.h \
	progress.h	\
	progresstext.h \
	progressthread.h \
	\
	rangeparser.cpp \
	rangeparser.h \
	\
	cyclicbuffer.h \
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
	refcount.cpp \
	refcount.h \
	refcountptr.cpp \
	refcountptr.h \
	breakhandler.cpp \
	breakhandler.h \
	\
	searchpath.cpp \
	searchpath.h \
	searchpathdbhandler.h \
	dirtreewalker.cpp \
	dirtreewalker.h \
	externalprog.h \
	hackstream.h \
	\
	tempfile.cpp	\
	tempfile.h	\
	\
	md5.cpp \
	md5.h \
	md5consumer.h \
	debug.cpp	\
	debug.h	\
	util.cpp \
	util.h \
	\
	rs232wrapper.h

libsupport_la_LDFLAGS = -static

