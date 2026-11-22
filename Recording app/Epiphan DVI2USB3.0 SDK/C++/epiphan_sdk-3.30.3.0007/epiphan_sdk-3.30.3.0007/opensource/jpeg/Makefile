# -*- Mode: makefile-gmake -*-
#
# $Id: Makefile 1617 2014-03-09 19:10:48Z monich $
#
# Copyright (C) 2001-2014 Epiphan Systems Inc. All rights reserved.
#
# Makefile for libjpeg
#

.PHONY: all clean 

#
# Include master config file
#

CONFIG_DIR = ../config
include $(CONFIG_DIR)/Config.mak

#
# Directories
#

SRC_DIR = .

#
# Compiler settings
#

DEFINES = $(BASE_DEFINES) -Dmain=_main

#
# Targets
#

DEBUG_LIB = $(JPEG_DEBUG_LIB)
RELEASE_LIB = $(JPEG_RELEASE_LIB)
RESULTS = $(DEBUG_LIB) $(RELEASE_LIB)

#
# Source files
#

SRC_C = jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c \
  jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c \
  jcphuff.c jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c \
  jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c \
  jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c \
  jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c jfdctfst.c \
  jfdctint.c jidctflt.c jidctfst.c jidctint.c jidctred.c jquant1.c \
  jquant2.c jutils.c jmemmgr.c jmemnobs.c

#
# Rules
#

all: debug release

clean:
	-rm -fr core core.* *~ $(SRC_DIR)/*~ $(RESULTS) $(BUILD_DIR)

#
# Common rules
#

include $(CONFIG_DIR)/Rules.mak

#
# Dependencies
#

include $(DEPENDS)
