# file      : build/import/libboost/serialization/rules.make
# copyright : Copyright (c) 2005-2011 Boris Kolpackov
# license   : GNU GPL v2; see accompanying LICENSE file

$(dcf_root)/import/libboost/%: root := $(libboost_root)

$(dcf_root)/import/libboost/serialization/serialization.l: \
  | $(dcf_root)/import/libboost/serialization/.

ifeq ($(libboost_type),archive)

ifeq ($(libboost_system),y)
$(dcf_root)/import/libboost/serialization/serialization.l: \
  $(libboost_root)/stage/lib/libboost_serialization$(libboost_suffix).a \
  $(libboost_root)/stage/lib/libboost_system$(libboost_suffix).a
else
$(dcf_root)/import/libboost/serialization/serialization.l: \
  $(libboost_root)/stage/lib/libboost_serialization$(libboost_suffix).a
endif
	@echo $^ >$@

else

ifeq ($(libboost_system),y)
$(dcf_root)/import/libboost/serialization/serialization.l: \
  $(libboost_root)/stage/lib/libboost_serialization$(libboost_suffix).so \
  $(libboost_root)/stage/lib/libboost_system$(libboost_suffix).so
else
$(dcf_root)/import/libboost/serialization/serialization.l: \
  $(libboost_root)/stage/lib/libboost_serialization$(libboost_suffix).so
endif
	@echo $^ >$@
	@echo rpath:$(root)/stage/lib >>$@

endif


$(dcf_root)/import/libboost/serialization/serialization.l.cpp-options: \
  | $(dcf_root)/import/libboost/serialization/.
	@echo include: -I$(root) >$@

ifndef %foreign%

disfigure::
	$(call message,rm $(dcf_root)/import/libboost/serialization/serialization.l,\
rm -f $(dcf_root)/import/libboost/serialization/serialization.l)
	$(call message,,rm -f $(dcf_root)/import/libboost/serialization/serialization.l.cpp-options)

endif
