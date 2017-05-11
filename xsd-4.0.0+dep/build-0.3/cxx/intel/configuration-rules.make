# file      : build/cxx/intel/configuration-rules.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

$(dcf_root)/cxx/intel/configuration-dynamic.make: | $(dcf_root)/cxx/intel/.
	$(call message,,$(bld_root)/cxx/intel/configure $@ $(cxx_optimize) \
"$(cxx_extra_options)" "$(cxx_ld_extra_options)")

ifndef %foreign%

$(dcf_root)/.disfigure::
	$(call message,rm $(dcf_root)/cxx/intel/configuration-dynamic.make,\
rm -f $(dcf_root)/cxx/intel/configuration-dynamic.make)

endif
