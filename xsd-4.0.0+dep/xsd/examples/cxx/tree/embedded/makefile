# file      : examples/cxx/tree/embedded/makefile
# copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))../../../../build/bootstrap.make

xsd := library.xsd
cxx := driver.cxx grammar-input-stream.cxx

obj := $(addprefix $(out_base)/,$(cxx:.cxx=.o) $(xsd:.xsd=.o) \
$(xsd:.xsd=-schema.o))
obj2 := $(out_base)/xsdbin.o
dep := $(obj:.o=.o.d) $(obj2:.o=.o.d)

driver   := $(out_base)/driver
xsdbin   := $(out_base)/xsdbin
install  := $(out_base)/.install
dist     := $(out_base)/.dist
dist-win := $(out_base)/.dist-win
clean    := $(out_base)/.clean


# Import.
#
$(call import,\
  $(scf_root)/import/libxerces-c/stub.make,\
  l: xerces_c.l,cpp-options: xerces_c.l.cpp-options)


# Build.
#
$(driver): $(obj) $(xerces_c.l)
$(xsdbin): $(obj2) $(xerces_c.l)

$(obj) $(dep): cpp_options := -I$(out_base) -I$(src_base) -I$(src_root)/libxsd
$(obj) $(obj2) $(dep): $(xerces_c.l.cpp-options)

genf := $(xsd:.xsd=.hxx) $(xsd:.xsd=.ixx) $(xsd:.xsd=.cxx)
gen  := $(addprefix $(out_base)/,$(genf))

$(gen): xsd := $(out_root)/xsd/xsd
$(gen): $(out_root)/xsd/xsd

genf += $(xsd:.xsd=-schema.hxx) $(xsd:.xsd=-schema.cxx)

.PRECIOUS: $(out_base)/%-schema.hxx $(out_base)/%-schema.cxx
$(out_base)/%: xsdbin := $(xsdbin)
$(out_base)/%-schema.hxx $(out_base)/%-schema.cxx: $(src_base)/%.xsd $(xsdbin)
	$(call message,xsdbin $<,$(xsdbin) --output-dir $(out_base) $<)

$(call include-dep,$(dep),$(obj),$(gen))

# Convenience alias for default target.
#
$(out_base)/: $(driver)


# Install & Dist.
#
dist-common := $(out_base)/.dist-common

$(install) $(dist) $(dist-win) $(dist-common): path := $(subst $(src_root)/,,$(src_base))

$(install):
	$(call install-data,$(src_base)/README,$(install_doc_dir)/xsd/$(path)/README)
	$(call install-data,$(src_base)/xsdbin.cxx,$(install_doc_dir)/xsd/$(path)/xsdbin.cxx)
	$(call install-data,$(src_base)/driver.cxx,$(install_doc_dir)/xsd/$(path)/driver.cxx)
	$(call install-data,$(src_base)/grammar-input-stream.hxx,$(install_doc_dir)/xsd/$(path)/grammar-input-stream.hxx)
	$(call install-data,$(src_base)/grammar-input-stream.cxx,$(install_doc_dir)/xsd/$(path)/grammar-input-stream.cxx)
	$(call install-data,$(src_base)/library.xsd,$(install_doc_dir)/xsd/$(path)/library.xsd)
	$(call install-data,$(src_base)/library.xml,$(install_doc_dir)/xsd/$(path)/library.xml)

$(dist-common):
	$(call install-data,$(src_base)/xsdbin.cxx,$(dist_prefix)/$(path)/xsdbin.cxx)
	$(call install-data,$(src_base)/driver.cxx,$(dist_prefix)/$(path)/driver.cxx)
	$(call install-data,$(src_base)/grammar-input-stream.hxx,$(dist_prefix)/$(path)/grammar-input-stream.hxx)
	$(call install-data,$(src_base)/grammar-input-stream.cxx,$(dist_prefix)/$(path)/grammar-input-stream.cxx)
	$(call install-data,$(src_base)/library.xsd,$(dist_prefix)/$(path)/library.xsd)
	$(call install-data,$(src_base)/library.xml,$(dist_prefix)/$(path)/library.xml)

$(dist): $(dist-common)
	$(call install-data,$(src_base)/README,$(dist_prefix)/$(path)/README)

$(dist-win): |$(out_root)/.dist-pre
$(dist-win): $(dist-common)
	$(call install-data,$(src_base)/README,$(dist_prefix)/$(path)/README.txt)
	$(call message,,todos $(dist_prefix)/$(path)/README.txt)
        # Note: solution files are not automatically generated.

# Clean.
#
$(clean): files := $(out_base)/$(xsd:.xsd=-schema.?xx)
$(clean): $(driver).o.clean $(xsdbin).o.clean          \
  $(addsuffix .cxx.clean,$(obj))                       \
  $(addsuffix .cxx.clean,$(obj2))                      \
  $(addsuffix .cxx.clean,$(dep))                       \
  $(addprefix $(out_base)/,$(xsd:.xsd=.cxx.xsd.clean))
	$(call message,rm '$(out_base)/*-schema.?xx',rm -f $(files))

# Generated .gitignore.
#
ifeq ($(out_base),$(src_base))
$(gen): | $(out_base)/.gitignore
$(driver): | $(out_base)/.gitignore

$(out_base)/.gitignore: files := driver xsdbin $(genf)
$(clean): $(out_base)/.gitignore.clean

$(call include,$(bld_root)/git/gitignore.make)
endif

# How to.
#
$(call include,$(bld_root)/cxx/o-e.make)
$(call include,$(bld_root)/cxx/cxx-o.make)
$(call include,$(bld_root)/cxx/cxx-d.make)
$(call include,$(scf_root)/xsd/tree/xsd-cxx.make)

$(call include,$(bld_root)/install.make)

# Dependencies.
#
$(call import,$(src_root)/xsd/makefile)
