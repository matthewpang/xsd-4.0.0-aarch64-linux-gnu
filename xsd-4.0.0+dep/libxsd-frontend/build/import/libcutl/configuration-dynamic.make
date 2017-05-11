ifeq ($(EXTERNAL_LIBCUTL),y)
libcutl_installed := y
else
libcutl_installed := n
src_root := $(abspath $(src_root)/../libcutl)
scf_root := $(src_root)/build
out_root := $(src_root)
endif
