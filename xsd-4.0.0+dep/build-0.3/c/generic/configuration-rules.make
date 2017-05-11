# file      : build/c/generic/configuration-rules.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

$(dcf_root)/c/generic/configuration-dynamic.make: | $(dcf_root)/c/generic/.
	$(call message,,$(bld_root)/c/generic/configure $@)

ifndef %foreign%

$(dcf_root)/.disfigure::
	$(call message,rm $(dcf_root)/c/generic/configuration-dynamic.make,\
rm -f $(dcf_root)/c/generic/configuration-dynamic.make)

endif
