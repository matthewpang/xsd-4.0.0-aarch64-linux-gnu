# file      : build/cxx/cxx-d.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

$(call include,$(bld_root)/cxx/configuration.make)

ifdef cxx_id
$(call include-once,$(bld_root)/cxx/$(cxx_id)/cxx-d.make,$(out_base))
endif
