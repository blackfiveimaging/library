#include "config.h"
#ifdef HAVE_LCMS2
#include "lcmswrapper_lcms2.cpp"
#else
#include "lcmswrapper_lcms1.cpp"
#endif

