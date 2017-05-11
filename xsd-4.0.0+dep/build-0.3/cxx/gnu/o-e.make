# file      : build/cxx/gnu/o-e.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

$(call include,$(bld_root)/cxx/gnu/configuration.make)

#@@ should it be lib%.so?
#
ifneq ($(cxx_extra_lib_paths),)
vpath %.so $(cxx_extra_lib_paths)
vpath %.a $(cxx_extra_lib_paths)
endif

ifneq ($(cxx_gnu_libraries),)
vpath %.so $(cxx_gnu_libraries)
vpath %.a $(cxx_gnu_libraries)
endif

$(out_base)/%: ld := $(cxx_gnu)
$(out_base)/%: ld_options := $(cxx_gnu_optimization_options) $(cxx_gnu_debugging_options) $(cxx_ld_extra_options)

$(out_base)/%: expand-l = $(if $(subst n,,$(cxx_rpath)),\
$(shell sed -e 's%^rpath:\(.*\)%-Wl,-rpath,\1%' $1),\
$(shell sed -e 's%^rpath:\(.*\)%%' $1))

ifeq ($(out_base),$(src_base))
$(out_base)/%: $(out_base)/%.o
else
$(out_base)/%: $(out_base)/%.o | $$(dir $$@).
endif
	$(call message,ld $@,$(ld) \
$(cxx_extra_options) $(ld_options) $(cxx_ld_extra_options) -o $@ \
$(foreach f,$^,$(if $(patsubst %.l,,$f),$f,$(call expand-l,$f))) $(cxx_extra_libs))

.PHONY: $(out_base)/%.o.clean

$(out_base)/%.o.clean:
	$(call message,rm $(basename $(basename $@)),rm -f $(basename $@) $(basename $(basename $@)))
