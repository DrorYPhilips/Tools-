# -*- Mode: makefile-gmake -*-
#
# $Id: Makefile 1618 2014-03-09 19:11:14Z monich $
#
# Copyright (C) 2001-2014 Epiphan Systems Inc. All rights reserved.
#
# Makefile for libpng
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
# Targets
#

DEBUG_LIB = $(PNG_DEBUG_LIB)
RELEASE_LIB = $(PNG_RELEASE_LIB)
RESULTS = $(DEBUG_LIB) $(RELEASE_LIB)

#
# Source files
#

SRC_C = png.c pngerror.c pnggccrd.c pngget.c pngmem.c pngpread.c pngread.c \
  pngrio.c pngrtran.c pngrutil.c pngset.c pngtest.c pngtrans.c pngvcrd.c \
  pngwio.c pngwrite.c pngwtran.c pngwutil.c

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
