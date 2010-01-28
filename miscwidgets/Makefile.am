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
	progressbar.cpp	\
	progressbar.h	\
	\
	uitab.cpp \
	uitab.h \
	\
	spinner.cpp \
	spinner.h \
	progressspinner.h \
	patheditor.cpp	\
	patheditor.h	\
	imageselector.cpp \
	imageselector.h \
	imageselector_noborder_image.h \
	\
	pixbuf_from_imagedata.cpp \
	pixbuf_from_imagedata.h \
	\
	refcountui.h


BUILT_SOURCES = squiggledata.cpp dabdata.cpp spinner1.cpp spinner2.cpp spinner3.cpp \
	spinner4.cpp spinner5.cpp spinner6.cpp spinner7.cpp spinner8.cpp

squiggledata.cpp:
	gdk-pixbuf-csource --name=squiggle_data >squiggledata.cpp squiggle.tif

dabdata.cpp:
	gdk-pixbuf-csource --name=dab_data >dabdata.cpp dab.tif

spinner1.cpp:
	gdk-pixbuf-csource --name=spinner1_data >spinner1.cpp spinner1.png
spinner2.cpp:
	gdk-pixbuf-csource --name=spinner2_data >spinner2.cpp spinner2.png
spinner3.cpp:
	gdk-pixbuf-csource --name=spinner3_data >spinner3.cpp spinner3.png
spinner4.cpp:
	gdk-pixbuf-csource --name=spinner4_data >spinner4.cpp spinner4.png
spinner5.cpp:
	gdk-pixbuf-csource --name=spinner5_data >spinner5.cpp spinner5.png
spinner6.cpp:
	gdk-pixbuf-csource --name=spinner6_data >spinner6.cpp spinner6.png
spinner7.cpp:
	gdk-pixbuf-csource --name=spinner7_data >spinner7.cpp spinner7.png
spinner8.cpp:
	gdk-pixbuf-csource --name=spinner8_data >spinner8.cpp spinner8.png


libmiscwidgets_la_LDFLAGS = -static

EXTRA_DIST = squiggledata.cpp dabdata.cpp spinner1.cpp spinner2.cpp spinner3.cpp \
	spinner4.cpp spinner5.cpp spinner6.cpp spinner7.cpp spinner8.cpp

