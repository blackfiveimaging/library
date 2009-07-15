# Makefile.am
# RL 28/11/04

SUBDIRS = po support imagesource imageutils miscwidgets stpui_widgets stp_support gp_cppsupport pixbufthumbnail profilemanager

localedir=$(datadir)/locale

AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -Wall -I./support/ -I./imagesource/ -I./imageutils -I./splashscreen \
	-I./miscwidgets -I./profilemanager -I./effects \
	$(LCMS_CFLAGS) $(GP_CFLAGS) $(GTK2_CFLAGS)

bin_PROGRAMS = profileuitest mkrgbprofile mklabprofile mkgreyprofile \
				composite imgsrctest \
				jpegcheck pvtest threadtest \
				gprintertest combotest \
				printqueuetest stpuitest mkthumbnail jpegscan

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

LDADD = \
	imageutils/libimageutils.la	\
	gp_cppsupport/libgp_cppsupport.la \
	miscwidgets/libmiscwidgets.la	\
	support/libsupport.la	\
	imagesource/libimagesource.la	\
	support/libsupport.la	\
	profilemanager/libprofilemanager.la	\
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


EXTRA_DIST = m4/ChangeLog  COPYING README TODO


ACLOCAL_AMFLAGS = -I m4
