ld_lib_type   := archive

ifeq ($(filter $(origin AR),undefined default),)
ld_lib_ar     := $(AR)
else
ld_lib_ar     := ar
endif

ifeq ($(filter $(origin RANLIB),undefined default),)
ld_lib_ranlib := $(RANLIB)
else
ld_lib_ranlib := ranlib
endif
