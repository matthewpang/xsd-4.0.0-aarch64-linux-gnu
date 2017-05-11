# file      : build/c/gnu/configuration-rules.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

$(dcf_root)/c/gnu/configuration-dynamic.make: | $(dcf_root)/c/gnu/.
	$(call message,,$(bld_root)/c/gnu/configure $@ $(c_optimize) \
"$(c_extra_options)" "$(c_ld_extra_options)")

ifndef %foreign%

$(dcf_root)/.disfigure::
	$(call message,rm $(dcf_root)/c/gnu/configuration-dynamic.make,\
rm -f $(dcf_root)/c/gnu/configuration-dynamic.make)

endif
