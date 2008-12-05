AM_CXXFLAGS = -Wall -I../support/ -I../profilemanager/ $(LCMS_CFLAGS) $(GTK2_CFLAGS) ${PNM_CFLAGS}

noinst_LTLIBRARIES = libimagesource.la

libimagesource_la_SOURCES =	\
	convkernel.h	\
	convkernel_gaussian.h	\
	convkernel_gaussian_1D.h	\
	convkernel_unsharpmask.h	\
	imagesource.cpp		\
	imagesource.h		\
	imagesource_types.h	\
	imagesource_bilinear.cpp	\
	imagesource_bilinear.h	\
	imagesource_bmp.cpp	\
	imagesource_bmp.h	\
	imagesource_cms.cpp	\
	imagesource_cms.h	\
	imagesource_convolution.cpp	\
	imagesource_convolution.h	\
	imagesource_crop.cpp	\
	imagesource_crop.h	\
	imagesource_dither.cpp	\
	imagesource_dither.h	\
	imagesource_desaturate.cpp	\
	imagesource_desaturate.h	\
	imagesource_downsample.cpp	\
	imagesource_downsample.h	\
	imagesource_flatten.cpp	\
	imagesource_flatten.h	\
	imagesource_gamma.cpp	\
	imagesource_gamma.h	\
	imagesource_gaussianblur.cpp \
	imagesource_gaussianblur.h \
	imagesource_gdkpixbuf.cpp 	\
	imagesource_gdkpixbuf.h	\
	imagesource_greyscale.cpp	\
	imagesource_greyscale.h	\
	imagesource_gs.cpp	\
	imagesource_gs.h	\
	imagesource_jpeg.cpp	\
	imagesource_jpeg.h	\
	imagesource_lanczossinc.cpp	\
	imagesource_lanczossinc.h	\
	imagesource_mask.cpp	\
	imagesource_mask.h	\
	imagesource_montage.cpp	\
	imagesource_montage.h	\
	imagesource_devicen_preview.cpp \
	imagesource_devicen_preview.h \
	imagesource_devicen_remap.cpp \
	imagesource_devicen_remap.h \
	imagesource_promote.cpp \
	imagesource_promote.h \
	imagesource_pnm.cpp \
	imagesource_pnm.h \
	imagesource_rotate.cpp	\
	imagesource_rotate.h	\
	imagesource_scale.cpp	\
	imagesource_scale.h	\
	imagesource_scaledensity.cpp	\
	imagesource_scaledensity.h	\
	imagesource_segmentmask.cpp \
	imagesource_segmentmask.h \
	imagesource_solid.cpp	\
	imagesource_solid.h	\
	imagesource_tiff.cpp	\
	imagesource_tiff.h	\
	imagesource_unsharpmask.cpp	\
	imagesource_unsharpmask.h	\
	imagesource_util.cpp	\
	imagesource_util.h	\
	devicencolorant.cpp \
	devicencolorant.h \
	pixbuf_from_imagesource.cpp	\
	pixbuf_from_imagesource.h	\
	iccjpeg.cpp	\
	iccjpeg.h

libimagesource_la_LDFLAGS = -static

