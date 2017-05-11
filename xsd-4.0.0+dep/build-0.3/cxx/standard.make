# file      : build/cxx/standard.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

# Set the cxx_standard variable to either c++98 or c++11.
#
$(call include,$(bld_root)/cxx/configuration.make) # cxx_id

cxx_standard :=

ifdef cxx_id
  $(call include,$(bld_root)/cxx/$(cxx_id)/configuration.make) # cxx_*_standard
  ifeq ($(cxx_id),gnu)
    ifdef cxx_gnu
      ifneq ($(filter $(cxx_gnu_standard),c++11 gnu++11),)
        cxx_standard := c++11
      else
        cxx_standard := c++98
      endif
    endif
  else ifeq ($(cxx_id),intel)
    ifdef cxx_intel
      ifneq ($(filter $(cxx_intel_standard),c++11 gnu++11),)
        cxx_standard := c++11
      else
        cxx_standard := c++98
      endif
    endif
  else ifeq ($(cxx_id),generic)
    cxx_standard := c++98
  else
    $(error unknown C++ compiler $(cxx_id))
  endif
  $(out_root)/%: cxx_standard := $(cxx_standard)
endif
