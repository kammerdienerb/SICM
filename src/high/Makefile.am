#
# Copyright 2018        UT-Battelle, LLC
#                       All rights reserved.
#
# See COPYING in top-level directory.
# 
# Additional copyrights may follow
# 
# $HEADER$
#

bin_PROGRAMS = 
lib_LTLIBRARIES = libhigh.la

libhigh_la_SOURCES = high.c profile.c
libhigh_la_LIBADD = @JEMALLOC_LIBS@ -lnuma ../low/libsicm.la
libhigh_la_LDFLAGS = -version-info 0:0:0
libhigh_la_CFLAGS = @JEMALLOC_CPPFLAGS@

if HAVE_LLVM
lib_LTLIBRARIES += libcompass.la

libcompass_la_SOURCES = compass.cpp
libcompass_la_LIBADD = @LLVM_LIBS@
libcompass_la_LDFLAGS = -version-info 0:0:0
libcompass_la_CPPFLAGS = @LLVM_CPPFLAGS@
endif