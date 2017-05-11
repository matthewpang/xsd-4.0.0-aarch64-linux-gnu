# file      : build/c/configuration-static.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

c_h_suffix := h
c_s_suffix := c

# Get user-supplied static configuration if any.
#
ifneq ($(bld_root),$(scf_root))
$(call -include,$(scf_root)/c/configuration-static.make)
endif
