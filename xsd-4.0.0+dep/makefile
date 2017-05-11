# file      : makefile
# copyright : Copyright (c) 2009-2014 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))/build-0.3/bootstrap.make

default  := $(out_base)/

.PHONY: $(default) test install dist dist-win clean

# Build.
#
$(default): $(out_base)/xsd/xsd/

# Test.
#
test: $(out_base)/xsd/.test

# Install.
#
install: $(out_base)/xsd/.install

# Dist.
#
dist: $(out_base)/xsd/.dist
dist-win: $(out_base)/xsd/.dist-win

# Clean.
#
clean: $(out_base)/libxsd-frontend/.clean      \
       $(out_base)/xsd/.clean

ifneq ($(EXTERNAL_LIBCUTL),y)
src_root := $(src_base)/libcutl
scf_root := $(src_root)/build
out_root := $(src_root)
$(call import,$(src_base)/libcutl/makefile)

clean: $(out_base)/libcutl/.clean
endif

src_root := $(src_base)/libxsd-frontend
scf_root := $(src_root)/build
out_root := $(src_root)
$(call import,$(src_base)/libxsd-frontend/makefile)

src_root := $(src_base)/xsd
scf_root := $(src_root)/build
out_root := $(src_root)

ifneq ($(filter $(MAKECMDGOALS),clean test install dist dist-win),)
$(call import,$(src_base)/xsd/makefile)
else
$(call import,$(src_base)/xsd/xsd/makefile)
endif
