# -*- Mode: makefile-gmake -*-
#
# $Id: Makefile 25621 2014-03-09 20:23:27Z monich $
#
# Copyright (C) 2007-2014 Epiphan Systems Inc. All rights reserved.
#
# Unix makefile for the v2u_edid sample
#

.PHONY: all clean 

#
# Name of this makefile
#

MAKEFILE = Makefile

#
# Include master config file
#

CONFIG_DIR = ../../../opensource/config
include $(CONFIG_DIR)/Config.mak

#
# Include libv2u config file
#

V2ULIB_REL_DIR = ../v2u_lib
V2ULIB_DIR = ${shell (if [ -d "$(V2ULIB_REL_DIR)" ] ; then cd "$(V2ULIB_REL_DIR)"; pwd; else echo "V2ULIB_DIR"; fi)}
include $(V2ULIB_DIR)/Config.mak

#
# Output directories
#

DEBUG_DEPENDENCIES = v2u_debug
RELEASE_DEPENDENCIES = v2u_release

DEBUG_LIBS = $(V2U_DEBUG_LIB)
RELEASE_LIBS = $(V2U_RELEASE_LIB)

#
# Targets
#

DEBUG_APP = $(DEBUG_BUILD_DIR)/v2u_edid
RELEASE_APP = $(RELEASE_BUILD_DIR)/v2u_edid
RESULTS = $(DEBUG_APP) $(RELEASE_APP)

#
# Rules
#

all: debug release

clean:
	-rm -fr core core.* *~ $(SRC_DIR)/*~ $(BUILD_DIR)

#
# Source files
#

SRC_DIR = .
SRC_C = v2u_edid.c

#
# Compiler settings
#

INCLUDES = -I../../include $(V2ULIB_INCLUDES)

#
# Common rules
#

include $(V2ULIB_DIR)/Rules.mak
include $(CONFIG_DIR)/Rules.mak

#
# Dependencies
#

include $(DEPENDS)
