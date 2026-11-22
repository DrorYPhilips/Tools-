# -*- Mode: makefile-gmake -*-
#
# $Id: Rules.mak 1717 2014-05-24 13:03:44Z monich $
#
# Copyright (C) 2001-2014 Epiphan Systems Inc. All rights reserved.
#
# Build rules
#
# Environment: 
#
# GNU make on Unix. Not guaranteed to work with non-GNU make utilities
# and especially on non-Unix platforms
#

#
# Input:
#
# SRC_C      list of the C source files (no directory names)
# SRC_CPP    list of the C++ source files (no directory names)
# SRC_OBJC   list of the Objective C source files (no directory names)
# SRC_OBJCPP list of the Objective C/C++ source files (no directory names)
# SRC_DIR    directory where source files are located
# 

SRC_FILES = \
$(SRC_C:%=$(SRC_DIR)/%) \
$(SRC_C1:%=$(SRC_DIR1)/%) \
$(SRC_C2:%=$(SRC_DIR2)/%) \
$(SRC_C3:%=$(SRC_DIR3)/%) \
$(SRC_C4:%=$(SRC_DIR4)/%) \
$(SRC_C5:%=$(SRC_DIR5)/%) \
$(SRC_C6:%=$(SRC_DIR6)/%) \
$(SRC_CPP:%=$(SRC_DIR)/%) \
$(SRC_CPP1:%=$(SRC_DIR1)/%) \
$(SRC_CPP2:%=$(SRC_DIR2)/%) \
$(SRC_OBJC:%=$(SRC_DIR)/%) \
$(SRC_OBJCPP:%=$(SRC_DIR)/%)

DEBUG_OBJS = \
$(SRC_C:%.c=$(DEBUG_BUILD_DIR)/%.o) \
$(SRC_C1:%.c=$(DEBUG_BUILD_DIR)/%.o) \
$(SRC_C2:%.c=$(DEBUG_BUILD_DIR)/%.o) \
$(SRC_C3:%.c=$(DEBUG_BUILD_DIR)/%.o) \
$(SRC_C4:%.c=$(DEBUG_BUILD_DIR)/%.o) \
$(SRC_C5:%.c=$(DEBUG_BUILD_DIR)/%.o) \
$(SRC_C6:%.c=$(DEBUG_BUILD_DIR)/%.o) \
$(SRC_CPP:%.cpp=$(DEBUG_BUILD_DIR)/%.o) \
$(SRC_CPP1:%.cpp=$(DEBUG_BUILD_DIR)/%.o) \
$(SRC_CPP2:%.cpp=$(DEBUG_BUILD_DIR)/%.o) \
$(SRC_OBJC:%.m=$(DEBUG_BUILD_DIR)/%.o) \
$(SRC_OBJCPP:%.mm=$(DEBUG_BUILD_DIR)/%.o)

RELEASE_OBJS = \
$(SRC_C:%.c=$(RELEASE_BUILD_DIR)/%.o) \
$(SRC_C1:%.c=$(RELEASE_BUILD_DIR)/%.o) \
$(SRC_C2:%.c=$(RELEASE_BUILD_DIR)/%.o) \
$(SRC_C3:%.c=$(RELEASE_BUILD_DIR)/%.o) \
$(SRC_C4:%.c=$(RELEASE_BUILD_DIR)/%.o) \
$(SRC_C5:%.c=$(RELEASE_BUILD_DIR)/%.o) \
$(SRC_C6:%.c=$(RELEASE_BUILD_DIR)/%.o) \
$(SRC_CPP:%.cpp=$(RELEASE_BUILD_DIR)/%.o) \
$(SRC_CPP1:%.cpp=$(RELEASE_BUILD_DIR)/%.o) \
$(SRC_CPP2:%.cpp=$(RELEASE_BUILD_DIR)/%.o) \
$(SRC_OBJC:%.m=$(RELEASE_BUILD_DIR)/%.o) \
$(SRC_OBJCPP:%.mm=$(RELEASE_BUILD_DIR)/%.o)

#
# Create build directories
#

$(DEBUG_BUILD_DIR):
	@mkdir -p $@

$(RELEASE_BUILD_DIR):
	@mkdir -p $@

$(DEBUG_OBJS):   | $(DEBUG_BUILD_DIR)
$(RELEASE_OBJS): | $(RELEASE_BUILD_DIR)

#
# These targets are used for building Mac OS X universal binaries
#

debug_%:
	$(call RUN,$(MAKE) -f $(MAKEFILE) V="$(V)" ARCH=$* debug,)

release_%:
	$(call RUN,$(MAKE) -f $(MAKEFILE) V="$(V)" ARCH=$* release,)

# ============================================================================
# Build static library
#
# The problem with ar on Mac OS X is that it can't update fat archives,
# even though it can create them. So we need to delete the .a file in
# between builds.
# ============================================================================

ifdef DEBUG_LIB

debug: $(DEBUG_PREBUILD_TARGET) $(DEBUG_LIB)

$(DEBUG_LIB): $(DEBUG_OBJS)
ifeq ($(OS),darwin)
	$(call RUN,rm -f $@,)
endif # darwin
	$(call RUN,$(AR) $(ARFLAGS) $@ $^,"  AR    $@")
	@ranlib $@

endif # DEBUG_LIB

ifdef RELEASE_LIB

release: $(RELEASE_PREBUILD_TARGET) $(RELEASE_LIB)

$(RELEASE_LIB): $(RELEASE_OBJS)
ifeq ($(OS),darwin)
	$(call RUN,rm -f $@,)
endif # darwin
	$(call RUN,$(AR) $(ARFLAGS) $@ $^,"  AR    $@")
	@ranlib $@

endif # RELEASE_LIB

# ============================================================================
# Build an application
# ============================================================================

ifdef DEBUG_APP

debug: $(DEBUG_PREBUILD_TARGET) $(DEBUG_APP)

$(DEBUG_APP): $(DEBUG_OBJS) $(DEBUG_DEPENDENCIES)
	$(call RUN,$(LD) $(LDFLAGS) -o $@ $(DEBUG_OBJS) $(DEBUG_LIBS) $(SYSTEM_LIBS),"  LINK  $@")

endif # DEBUG_APP

ifdef RELEASE_APP

release: $(RELEASE_PREBUILD_TARGET) $(RELEASE_APP)

$(RELEASE_APP): $(RELEASE_OBJS) $(RELEASE_DEPENDENCIES)
	$(call RUN,$(LD) $(LDFLAGS) -o $@ $(RELEASE_OBJS) $(RELEASE_LIBS) $(SYSTEM_LIBS),"  LINK  $@")
	@$(STRIP) $@

endif # RELEASE_APP

# ============================================================================
# Build a shared library
# ============================================================================

ifdef DEBUG_SO

debug: $(DEBUG_PREBUILD_TARGET) $(DEBUG_SO)

$(DEBUG_SO): $(DEBUG_OBJS) $(DEBUG_DEPENDENCIES)
	$(call RUN,$(LD) $(LD_SHARED_OPT) $(LDFLAGS) -o $@ $(DEBUG_OBJS) $(DEBUG_LIBS) $(SYSTEM_LIBS),"  LINK  $@")

endif # DEBUG_SO

ifdef RELEASE_SO

release: $(RELEASE_PREBUILD_TARGET) $(RELEASE_SO)

$(RELEASE_SO): $(RELEASE_OBJS) $(RELEASE_DEPENDENCIES)
	$(call RUN,$(LD) $(LD_SHARED_OPT) $(LDFLAGS) -o $@ $(RELEASE_OBJS) $(RELEASE_LIBS) $(SYSTEM_LIBS),"  LINK  $@")

endif # RELEASE_SO

# ============================================================================
#
# Components
#
# ============================================================================

.PHONY: slib slib_clean slib_debug slib_release

slib: slib_debug slib_release

slib_clean:
	$(call RUN,$(MAKE) -C $(SLIB_DIR) V="$(V)" clean,)

slib_debug:
	$(call RUN,$(MAKE) -C $(SLIB_DIR) V="$(V)" debug,)

slib_release:
	$(call RUN,$(MAKE) -C $(SLIB_DIR) V="$(V)" release,)

slib_debug_%:
	$(call RUN,$(MAKE) -C $(SLIB_DIR) V="$(V)" ARCH=$* debug,)

slib_release_%:
	$(call RUN,$(MAKE) -C $(SLIB_DIR) V="$(V)" ARCH=$* release,)

.PHONY: jpeg jpeg_clean jpeg_debug jpeg_release

jpeg: jpeg_debug jpeg_release

ifdef JPEG_DIR

jpeg_clean:
	$(call RUN,$(MAKE) -C $(JPEG_DIR) V="$(V)" clean,)

jpeg_debug:
	$(call RUN,$(MAKE) -C $(JPEG_DIR) V="$(V)" debug,)

jpeg_release:
	$(call RUN,$(MAKE) -C $(JPEG_DIR) V="$(V)" release,)

jpeg_debug_%:
	$(call RUN,$(MAKE) -C $(JPEG_DIR) V="$(V)" ARCH=$* debug,)

jpeg_release_%:
	$(call RUN,$(MAKE) -C $(JPEG_DIR) V="$(V)" ARCH=$* release,)

else  # JPEG_DIR
jpeg_clean:

jpeg_debug:

jpeg_release:

endif # JPEG_DIR

.PHONY: png png_clean png_debug png_release

png: png_debug png_release

ifdef PNG_DIR

png_clean:
	$(call RUN,$(MAKE) -C $(PNG_DIR) V="$(V)" clean,)

png_debug:
	$(call RUN,$(MAKE) -C $(PNG_DIR) V="$(V)" debug,)

png_release:
	$(call RUN,$(MAKE) -C $(PNG_DIR) V="$(V)" release,)

png_debug_%:
	$(call RUN,$(MAKE) -C $(PNG_DIR) V="$(V)" ARCH=$* debug,)

png_release_%:
	$(call RUN,$(MAKE) -C $(PNG_DIR) V="$(V)" ARCH=$* release,)

else  # PNG_DIR
png_clean:

png_debug:

png_release:

endif # PNG_DIR

.PHONY: curl_clean curl_debug curl_release

ifdef CURL_DIR
curl_clean:
	$(call RUN,$(MAKE) -C $(CURL_LIB_DIR) V="$(V)" clean,)

curl_debug:
	$(call RUN,$(MAKE) -C $(CURL_LIB_DIR) V="$(V)" debug,)

curl_release:
	$(call RUN,$(MAKE) -C $(CURL_LIB_DIR) V="$(V)" release,)

else  # CURL_DIR
curl_clean:

curl_debug:

curl_release:

endif # CURL_DIR

.PHONY: expat_clean expat_debug expat_release

ifdef EXPAT_DIR
expat_clean:
	$(call RUN,$(MAKE) -C $(EXPAT_DIR) V="$(V)" clean,)

expat_debug:
	$(call RUN,$(MAKE) -C $(EXPAT_DIR) V="$(V)" debug,)

expat_release:
	$(call RUN,$(MAKE) -C $(EXPAT_DIR) V="$(V)" release,)

expat_debug_%:
	$(call RUN,$(MAKE) -C $(EXPAT_DIR) V="$(V)" ARCH=$* debug,)

expat_release_%:
	$(call RUN,$(MAKE) -C $(EXPAT_DIR) V="$(V)" ARCH=$* release,)

else  # EXPAT_DIR
expat_clean:

expat_debug:

expat_release:

endif # EXPAT_DIR

ifdef ZLIB_DIR
zlib_clean:
	$(call RUN,$(MAKE) -C $(ZLIB_DIR) V="$(V)" clean,)

zlib_debug:
	$(call RUN,$(MAKE) -C $(ZLIB_DIR) V="$(V)" debug,)

zlib_release:
	$(call RUN,$(MAKE) -C $(ZLIB_DIR) V="$(V)" release,)
else  # ZLIB_DIR
zlib_clean:

zlib_debug:

zlib_release:

endif # ZLIB_DIR

#
# Pattern Rules
#

CPPFLAGS = $(BASE_CPPFLAGS) $(INCLUDES) $(DEFINES)
DEBUG_OPTS = -c $(DEBUG_FLAGS) $(CFLAGS) $(CPPFLAGS) $(DEBUG_DEFINES) 
RELEASE_OPTS = -c $(RELEASE_FLAGS) $(CFLAGS) $(CPPFLAGS) $(RELEASE_DEFINES)

ifndef CPLUSPLUSFLAGS
CPLUSPLUSFLAGS = $(CFLAGS)
endif

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(call RUN,$(CC) $(DEBUG_OPTS) $< -o $@,"  CC    $@")

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR1)/%.c
	$(call RUN,$(CC) $(DEBUG_OPTS) $< -o $@,"  CC    $@")

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR2)/%.c
	$(call RUN,$(CC) $(DEBUG_OPTS) $< -o $@,"  CC    $@")

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR3)/%.c
	$(call RUN,$(CC) $(DEBUG_OPTS) $< -o $@,"  CC    $@")

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR4)/%.c
	$(call RUN,$(CC) $(DEBUG_OPTS) $< -o $@,"  CC    $@")

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR5)/%.c
	$(call RUN,$(CC) $(DEBUG_OPTS) $< -o $@,"  CC    $@")

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR6)/%.c
	$(call RUN,$(CC) $(DEBUG_OPTS) $< -o $@,"  CC    $@")

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(call RUN,$(CC) $(CPLUSPLUSFLAGS) $(DEBUG_OPTS) $< -o $@,"  CC    $@")

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR1)/%.cpp
	$(call RUN,$(CC) $(CPLUSPLUSFLAGS) $(DEBUG_OPTS) $< -o $@,"  CC    $@")

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR2)/%.cpp
	$(call RUN,$(CC) $(CPLUSPLUSFLAGS) $(DEBUG_OPTS) $< -o $@,"  CC    $@")

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR)/%.m
	$(call RUN,$(CC) $(DEBUG_OPTS) $< -o $@,"  CC    $@")

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR)/%.mm
	$(call RUN,$(CC) $(DEBUG_OPTS) $< -o $@,"  CC    $@")

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(call RUN,$(CC) $(RELEASE_OPTS) $< -o $@,"  CC    $@")

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR1)/%.c
	$(call RUN,$(CC) $(RELEASE_OPTS) $< -o $@,"  CC    $@")

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR2)/%.c
	$(call RUN,$(CC) $(RELEASE_OPTS) $< -o $@,"  CC    $@")

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR3)/%.c
	$(call RUN,$(CC) $(RELEASE_OPTS) $< -o $@,"  CC    $@")

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR4)/%.c
	$(call RUN,$(CC) $(RELEASE_OPTS) $< -o $@,"  CC    $@")

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR5)/%.c
	$(call RUN,$(CC) $(RELEASE_OPTS) $< -o $@,"  CC    $@")

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR6)/%.c
	$(call RUN,$(CC) $(RELEASE_OPTS) $< -o $@,"  CC    $@")

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(call RUN,$(CC) $(CPLUSPLUSFLAGS) $(RELEASE_OPTS) $< -o $@,"  CC    $@")

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR1)/%.cpp
	$(call RUN,$(CC) $(CPLUSPLUSFLAGS) $(RELEASE_OPTS) $< -o $@,"  CC    $@")

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR2)/%.cpp
	$(call RUN,$(CC) $(CPLUSPLUSFLAGS) $(RELEASE_OPTS) $< -o $@,"  CC    $@")

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR)/%.m
	$(call RUN,$(CC) $(RELEASE_OPTS) $< -o $@,"  CC    $@")

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR)/%.mm
	$(call RUN,$(CC) $(RELEASE_OPTS) $< -o $@,"  CC    $@")

#
# Cancel some implicit rules
#

%: %.o

#
# Special target (dependency rules)
#
# Unfortunately -M options are not allowed with multiple -arch flags
# on Mac OS X. This restriction certainly makes sense but complicates
# dependency generation for us.
#

ifndef DEPENDS
DEPENDS = Makefile.dep
endif # DEPENDS

ifndef DEPENDS_DEPS
DEPENDS_DEPS = $(SRC_FILES)
endif # DEPENDS_DEPS

#
# sed script for filtering the dependency file. Makes the dependency file
# much smaller, but I'm not sure whether that makes build any faster
#

DEPENDS_SED += \
-e 's/'`echo $(SLIB_DIR)|sed 's/[\/\.]/\\\\&/g'`'/$$(SLIB_DIR)/g'

ifdef JPEG_DIR
DEPENDS_SED += \
-e 's/'`echo $(JPEG_DIR)|sed 's/[\/\.]/\\\\&/g'`'/$$(JPEG_DIR)/g'
endif

ifdef PNG_DIR
DEPENDS_SED += \
-e 's/'`echo $(PNG_DIR)|sed 's/[\/\.]/\\\\&/g'`'/$$(PNG_DIR)/g'
endif

ifdef CURL_DIR
DEPENDS_SED += \
-e 's/'`echo $(CURL_DIR)|sed 's/[\/\.]/\\\\&/g'`'/$$(CURL_DIR)/g'
endif

ifdef EXPAT_DIR
DEPENDS_SED += \
-e 's/'`echo $(EXPAT_DIR)|sed 's/[\/\.]/\\\\&/g'`'/$$(EXPAT_DIR)/g'
endif

ifdef ZLIB_DIR
DEPENDS_SED += \
-e 's/'`echo $(ZLIB_DIR)|sed 's/[\/\.]/\\\\&/g'`'/$$(ZLIB_DIR)/g'
endif

depends nothing:

nodepend:
	-rm -fr $(DEPENDS)

veryclean: clean nodepend

$(DEPENDS): $(DEPENDS_DEPS)
	@echo "$(MAKE): Updating dependencies"
	@echo "# -*- Mode: makefile -*-" > $@
	@echo "# This file is generated automatically." >> $@
	@echo "# Run 'make veryclean' and  'make' to update it." >> $@
	@$(CC) -MM $(CPPFLAGS) $(DEBUG_DEFINES) $(SRC_FILES) | sed -e "s/^.*:/\\$$\(DEBUG_BUILD_DIR)\\/&/g" $(DEPENDS_SED) >> $@
	@$(CC) -MM $(CPPFLAGS) $(RELEASE_DEFINES) $(SRC_FILES) | sed -e "s/^.*:/\\$$\(RELEASE_BUILD_DIR)\\/&/g" $(DEPENDS_SED) >> $@
