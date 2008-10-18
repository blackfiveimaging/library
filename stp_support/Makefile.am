AM_CFLAGS = -Wall
AM_CXXFLAGS = -Wall

noinst_LTLIBRARIES = libstp_support.la

libstp_support_la_SOURCES = 		\
	printerqueues.c	\
	printerqueues.h	\
	stputil.c	\
	stputil.h


libstp_support_la_LDFLAGS = -static

EXTRA_DIST = printerqueues_unix.c printerqueues_win32.c
