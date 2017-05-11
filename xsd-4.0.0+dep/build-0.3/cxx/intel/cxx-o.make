# file      : build/cxx/intel/cxx-o.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

$(call include-once,$(bld_root)/c/cpp-options.make,$(out_base))
$(call include,$(bld_root)/cxx/intel/configuration.make)

$(out_base)/%.o: cxx := $(cxx_intel)
$(out_base)/%.o: cxx_options := $(cxx_intel_optimization_options) $(cxx_intel_debugging_options)

#@@ wrong prefix
#
$(out_base)/%.o: expand-cpp-options-impl = \
$(if $1,$(shell sed -e 's%include: \(.*\)%\1%' -e t -e d $1))

$(out_base)/%.o: expand-cpp-options = \
$(call expand-cpp-options-impl,$(filter %.cpp-options,$1))

ifeq ($(out_base),$(src_base))
$(out_base)/%.o: $(src_base)/%.$(cxx_s_suffix)
else
$(out_base)/%.o: $(src_base)/%.$(cxx_s_suffix) | $$(dir $$@).
endif
	$(call message,c++ $<,$(cxx) \
$(cpp_options) $(call expand-cpp-options,$^) $(cxx_pp_extra_options) \
$(cxx_options) $(cxx_pic_options) $(cxx_extra_options) -o $@ -c $<)

ifneq ($(out_base),$(src_base))

$(out_base)/%.o: $(out_base)/%.$(cxx_s_suffix) | $$(dir $$@).
	$(call message,c++ $<,$(cxx) \
$(cpp_options) $(call expand-cpp-options,$^) $(cxx_pp_extra_options) \
$(cxx_options) $(cxx_pic_options) $(cxx_extra_options) -o $@ -c $<)

endif

.PHONY: $(out_base)/%.o.$(cxx_s_suffix).clean

$(out_base)/%.o.$(cxx_s_suffix).clean:
	$(call message,rm $$1,rm -f $$1,$(basename $(basename $@)))
