ifeq ($(filter $(origin CXX),undefined default),)
cxx_generic := $(CXX)
else
cxx_generic := g++
endif

r := $(shell $(cxx_generic) $(cxx_extra_options) $(cxx_ld_extra_options) \
-print-search-dirs 2>&1 | sed -e 's/libraries: =//p' -e d)

ifneq ($r,)
  ifeq ($(cxx_extra_lib_paths),)
    cxx_extra_lib_paths := $r
  else
    cxx_extra_lib_paths := $(cxx_extra_lib_paths):$r
  endif
endif
