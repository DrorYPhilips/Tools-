# -*- Mode: makefile-gmake -*-
#
# $Id: Rules.mak 25621 2014-03-09 20:23:27Z monich $
#
# Copyright (C) 2003-2014 Epiphan Systems Inc. All rights reserved.
#
# libv2u build rules
#

v2u_clean:
	$(MAKE) -C $(V2ULIB_DIR) clean

v2u_debug:
	$(MAKE) -C $(V2ULIB_DIR) debug

v2u_release:
	$(MAKE) -C $(V2ULIB_DIR) release

v2u_debug_%:
	$(MAKE) -C $(V2ULIB_DIR) ARCH=$* debug

v2u_release_%:
	$(MAKE) -C $(V2ULIB_DIR) ARCH=$* release
