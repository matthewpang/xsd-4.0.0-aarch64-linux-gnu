cxx_id               := generic
cxx_pp_extra_options := $(CPPFLAGS)
cxx_extra_options    := $(CXXFLAGS)
cxx_ld_extra_options := $(LDFLAGS)
cxx_extra_libs       := $(LIBS)

r := $(shell echo $(LDFLAGS) | sed -e 's/-L *\([^ ]*\)/-L\1/g')
r := $(patsubst -L%,%,$(filter -L%,$(r)))
r := $(shell echo $(r) | sed -e 's/ /:/g')

cxx_extra_lib_paths  := $(r)
