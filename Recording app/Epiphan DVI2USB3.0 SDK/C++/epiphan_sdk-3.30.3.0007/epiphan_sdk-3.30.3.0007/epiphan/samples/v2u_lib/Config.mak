# -*- Mode: makefile-gmake -*-
#
# $Id: Config.mak 25621 2014-03-09 20:23:27Z monich $
#
# Copyright (C) 2003-2014 Epiphan Systems Inc. All rights reserved.
#
# libv2u build configuration
#

ifndef V2ULIB_DIR
${error V2ULIB_DIR must point to the directory containing Config.mak}
endif # V2ULIB_DIR

# Make sure that opensource/config/Config.mak is included
ifndef COMMON_CONFIG_INCLUDED
${error You must include master Config.mak prior to including this file}
endif # COMMON_CONFIG_INCLUDED

V2ULIB_BASE = libv2u
V2ULIB_RELEASE_FILE = $(V2ULIB_BASE).a
V2ULIB_DEBUG_FILE = $(V2ULIB_BASE)d.a

#
# Directories
#

V2ULIB_RELEASE_BUILD_DIR = $(V2ULIB_DIR)/$(RELEASE_BUILD_DIR)
V2ULIB_DEBUG_BUILD_DIR = $(V2ULIB_DIR)/$(DEBUG_BUILD_DIR)

#
# Includes
#

V2ULIB_INCLUDES = -I$(V2ULIB_DIR)/include

#
# Targets
#

V2U_RELEASE_LIB = $(V2ULIB_RELEASE_BUILD_DIR)/$(V2ULIB_RELEASE_FILE)
V2U_DEBUG_LIB = $(V2ULIB_DEBUG_BUILD_DIR)/$(V2ULIB_DEBUG_FILE)

V2U_RELEASE_TARGET = v2u_release$(ARCH_TARGET_SUFFIX)
V2U_DEBUG_TARGET = v2u_debug$(ARCH_TARGET_SUFFIX)
