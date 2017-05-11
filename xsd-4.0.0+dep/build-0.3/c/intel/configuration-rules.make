# file      : build/c/intel/configuration-rules.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

$(dcf_root)/c/intel/configuration-dynamic.make: | $(dcf_root)/c/intel/.
	$(call message,,$(bld_root)/c/intel/configure $@ $(c_optimize) \
"$(c_extra_options)" "$(c_ld_extra_options)")

ifndef %foreign%

$(dcf_root)/.disfigure::
	$(call message,rm $(dcf_root)/c/intel/configuration-dynamic.make,\
rm -f $(dcf_root)/c/intel/configuration-dynamic.make)

endif
