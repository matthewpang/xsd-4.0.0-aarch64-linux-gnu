# file      : build/cxx/o-l.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

$(call include,$(bld_root)/ld/configuration-lib.make)
$(call include,$(bld_root)/cxx/configuration.make)

ifdef cxx_id
$(call include-once,$(bld_root)/cxx/$(cxx_id)/o-l.make,$(out_base))
endif
