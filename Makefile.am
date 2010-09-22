# Makefile.am

SUBDIRS = po argyllsupport support imagesource imageutils miscwidgets stpui_widgets stp_support gp_cppsupport pixbufthumbnail profilemanager

localedir=$(datadir)/locale

AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -Wall -I./support/ -I./imagesource/ -I./imageutils -I./splashscreen \
	-I./miscwidgets -I./profilemanager -I./effects \
	$(LCMS_CFLAGS) $(GP_CFLAGS) $(GTK2_CFLAGS)

check_PROGRAMS = dirtreetest jpegscan jpegcheck \
				mkrgbprofile mklabprofile mkgreyprofile \
				composite imgsrctest \
				combotest tabui \
				printqueuetest stpuitest debugtest errorqueuetest


bin_PROGRAMS = threadtest gprintertest mkthumbnail profileuitest pvtest configtest hsvtest

hsvtest_SOURCES = hsvtest.cpp
configtest_SOURCES = configtest.cpp
debugtest_SOURCES = debugtest.cpp
gprintertest_SOURCES = gprintertest.cpp
combotest_SOURCES = combotest.cpp
profileuitest_SOURCES = profileuitest.cpp
mkthumbnail_SOURCES = mkthumbnail.cpp
mkrgbprofile_SOURCES = mkrgbprofile.cpp
mkgreyprofile_SOURCES = mkgreyprofile.cpp
mklabprofile_SOURCES = mklabprofile.cpp
composite_SOURCES = composite.cpp
imgsrctest_SOURCES = imgsrctest.cpp
jpegcheck_SOURCES = jpegcheck.cpp
pvtest_SOURCES = pvtest.cpp
threadtest_SOURCES = threadtest.cpp
printqueuetest_SOURCES = printqueuetest.cpp
stpuitest_SOURCES = stpuitest.cpp
jpegscan_SOURCES = jpegscan.cpp
dirtreetest_SOURCES = dirtreetest.cpp
tabui_SOURCES = tabui.cpp
errorqueuetest_SOURCES = errorqueuetest.cpp

LDADD = \
	argyllsupport/libargyllsupport.la \
	imageutils/libimageutils.la	\
	gp_cppsupport/libgp_cppsupport.la \
	miscwidgets/libmiscwidgets.la	\
	imagesource/libimagesource.la	\
	profilemanager/libprofilemanager.la	\
	support/libsupport.la	\
	stpui_widgets/libstpui_widgets.la	\
	stp_support/libstp_support.la \
	pixbufthumbnail/libpixbufthumbnail.la	\
	$(LIBINTL) $(LIBM_LIBS) $(GETOPT_LIBS) $(JPEG_LIBS) $(PNM_LIBS) $(TIFF_LIBS) $(LCMS_LIBS) $(GP_LIBS) $(GTK2_LIBS)


imagesource/libimagesource.la:
	cd imagesource
	$(MAKE) libimagesource.la
	
support/libsupport.la:
	cd support
	$(MAKE) libsupport.la

profilemanager/libprofilemanager.la:
	cd profilemanager
	$(MAKE) libprofilemanager.la

stpui_widgets/libstpui_widgets.la:
	cd stpui_widgets
	$(MAKE) libstpui_widgets.la

stp_support/libstp_support.la:
	cd stp_support
	$(MAKE) libstp_support.la

gp_cppsupport/libgp_cppsupport.la:
	cd gp_cppsupport
	$(MAKE) libgp_cppsupport.la

pixbufthumbnail/libpixbufthumbnail.la:
	cd pixbufthumbnail
	$(MAKE) libpixbufthumbnail.la


EXTRA_DIST = COPYING README TODO


ACLOCAL_AMFLAGS = -I m4
