# file      : build/bootstrap.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

# First time bootstrap.make is read
#
ifndef %makefile%

# Configure make.
#

# Forces make to delete targets whos rebuild commands failed but
# updated the target.
#
.DELETE_ON_ERROR:

# Remove all built-in rules.
#
.SUFFIXES:

ifeq ($(filter -r,$(MAKEFLAGS)),)
MAKEFLAGS += -r
endif


# Disable implicit phony misfeature.
#
#ifeq ($(filter --no-implicit-phony,$(MAKEFLAGS)),)
#MAKEFLAGS += --no-implicit-phony
#endif

# Enable second expansion.
#
.SECONDEXPANSION:

# Force target. Used to force rule executions.
#
.PHONY: FORCE


# Export variables that could be useful for configuration scripts.
#
export bld_root
export MAKE

%replica%     :=
%foreign%     :=
%interactive% := t

# List of realpath'ed include-once files.
#
%included_files% :=

#$(warning init %included_files%: $(%included_files%))

# @@ maybe I should set makefile to bootstrap and restore it at the end.
#    This way I will be able to use %makefile% below (instead of _self) and
#    also it will be consistent.
#

# By convention, the makefile name is the second or third last word from
# the end. If it is the third, then the second one should be end with
# bootstrap.make
#
%makefile%          := $(subst $(lastword $(MAKEFILE_LIST)),,$(MAKEFILE_LIST))
ifneq ($(filter %bootstrap.make,$(lastword $(%makefile%))),)
%makefile%          := $(subst $(lastword $(%makefile%)),,$(%makefile%))
endif
%makefile%          := $(abspath $(lastword $(%makefile%)))

%makefile_realpath% := $(realpath $(%makefile%))


#
# include facility
#

# Execute makefile search algorithm. Return abspath of the makefile if found
# and empty string otherwise.
#
%frame_exclude% += include-find-makefile
define include-find-makefile
$(if $(wildcard $1),\
$(abspath $1),\
$(if $(filter-out /%,$1),\
$(call include-search-dirs,$1,$(.INCLUDE_DIRS))))
endef

# Search for $1 in the list of directoris $2
#
%frame_exclude% += include-search-dirs
define include-search-dirs
$(if $(strip $2),\
$(if $(wildcard $(firstword $2)/$1),\
$(abspath $(firstword $2)/$1),\
$(call include-search-dirs,$1,$(wordlist 2,$(words $2),$2))))
endef

# Test if $1 is a 'foreign' makefile, i.e., does not reside in
# src_root, out_root, or bld_root. Use shadows for src and out
# roots to catch cases where these were reset before inclusion.
# This happens in import stubs.
#
%frame_exclude% += include-test-foreign
define include-test-foreign
$(if $(%foreign%),t,$(if $(filter $(bld_root)/% $(src_root_shadow)/% $(out_root_shadow)/%,$1),,t))
endef

%frame_exclude% += include-body
define include-body
%interactive%       :=
%makefile%          := $(call include-find-makefile,$2)
%makefile_realpath% := $$(realpath $$(%makefile%))
%foreign%           := $$(call include-test-foreign,$$(%makefile%))

$1 $$(if $$(%makefile%),$$(%makefile%),$2)

%makefile_realpath% := $3
%makefile%          := $4
%interactive%       := $5
%foreign%           := $6
endef

%frame_exclude% += include
define include
$(eval $(foreach f,$1,\
  $(call include-body,\
    include,$f,$(%makefile_realpath%),$(%makefile%),$(%interactive%),$(%foreign%))))
endef

%frame_exclude% += -include
define -include
$(eval $(foreach f,$1,\
  $(call include-body,\
    -include,$f,$(%makefile_realpath%),$(%makefile%),$(%interactive%),$(%foreign%))))
endef


#
# include-once
#

# This one simply delegates to the include directive to get the
# error message.
#
# $1 : include/-include
# $2 : file
#
%frame_exclude% += include-once-failed-body
define include-once-failed-body
$1 $2
endef

#
# $1 : include/-include
# $2 : file
#
%frame_exclude% += include-once-file-body
define include-once-file-body
%interactive%       :=
%makefile%          := $2
%makefile_realpath% := $(realpath $2)
%foreign%           := $$(call include-test-foreign,$$(%makefile%))

%included_files%    += $$(%makefile_realpath%)

$1 $2

%makefile_realpath% := $3
%makefile%          := $4
%interactive%       := $5
%foreign%           := $6
endef


%frame_exclude% += include-once-file-filter
define include-once-file-filter
$(if $(filter $(realpath $2),$(%included_files%)),,\
  $(call include-once-file-body,\
    $1,$2,$(%makefile_realpath%),$(%makefile%),$(%interactive%),$(%foreign%)))
endef

#
# $1 : include/-include
# $2 : file
# $6 : value
#

%frame_exclude% += include-once-value-body
define include-once-value-body
%interactive%       :=
%makefile%          := $2
%makefile_realpath% := $(realpath $2)
%foreign%           := $$(call include-test-foreign,$$(%makefile%))

%include_once_$$(%makefile_realpath%)% += $7

$1 $2

%makefile_realpath% := $3
%makefile%          := $4
%interactive%       := $5
%foreign%           := $6
endef


%frame_exclude% += include-once-value-filter
define include-once-value-filter
$(if $(filter $3,$(value %include_once_$(realpath $2)%)),,\
  $(call include-once-value-body,\
    $1,$2,$(%makefile_realpath%),$(%makefile%),$(%interactive%),$(%foreign%),$3))
endef


# $1 - include/-include
# $2 - abspath or empty if not found
# $3 - orginal path
# $4 - value [optional]
#
%frame_exclude% += include-once-filter
define include-once-filter
$(if $2,\
$(if $4,\
$(call include-once-value-filter,$1,$2,$4),\
$(call include-once-file-filter,$1,$2)),\
$(call include-once-failed-body,$1,$3))
endef

%frame_exclude% += include-once
define include-once
$(eval $(call include-once-filter,\
include,$(call include-find-makefile,$1),$1,$(strip $2)))
endef

%frame_exclude% += -include-once
define -include-once
$(eval $(call include-once-filter,\
-include,$(call include-find-makefile,$1),$1,$(strip $2)))
endef


#$(warning include facility is up)

endif # %makefile%

#
#
bld_root := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))


# initialize {src,out}_{root,base}
#
%frame_exclude% += find-bootstrap-base
define find-bootstrap-base
$(if $(findstring $(abspath $(lastword $1)),$(%makefile%)),$2,\
$(call find-bootstrap-base,\
$(wordlist 1,$(words $(wordlist 2,$(words $1),$1)),$1),\
$(lastword $1)))
endef

src_root := $(abspath $(dir $(call find-bootstrap-base,$(MAKEFILE_LIST)))..)
src_base := $(abspath $(dir $(%makefile%)))

ifneq ($(src_root),$(src_base))

# In recursive make invocation, out_root may come from the environment
# in which case we need to override it.
#
ifneq ($(origin out_root),file)
  out_root := $(abspath $(patsubst \
    %$(subst $(src_root)/,,$(src_base)),%,$(CURDIR)))
endif

out_base := $(out_root)/$(subst $(src_root)/,,$(src_base))

else

# In recursive make invocation, out_root may come from the environment
# in which case we need to override it.
#
ifneq ($(origin out_root),file)
  out_root := $(abspath $(CURDIR))
endif

out_base := $(out_root)

endif

scf_root := $(src_root)/build
dcf_root := $(out_root)/build

src_root_shadow := $(src_root)
out_root_shadow := $(out_root)

$(out_root)/%: export out_root := $(out_root)
$(out_root)/%: export src_root := $(src_root)
$(out_root)/%: export scf_root := $(scf_root)
$(out_root)/%: export dcf_root := $(dcf_root)
$(out_root)/%: export project_name := $(project_name)

# @@ Why do I need out_root here?
#
$(out_base)/%: out_root := $(out_root)
$(out_base)/%: export out_base := $(out_base)
$(out_base)/%: export src_base := $(src_base)

ifdef build_debug

$(warning "bld root: $(bld_root)")
$(warning "scf root: $(scf_root)")
$(warning "dcf root: $(dcf_root)")
$(warning "src root: $(src_root)")
$(warning "src base: $(src_base)")
$(warning "out root: $(out_root)")
$(warning "out base: $(out_base)")

endif

#$(warning paths are set)

# frame facility
#
$(call include-once,$(bld_root)/frame.make)

# import facility
#
$(call include-once,$(bld_root)/import.make)

# Load some common facilities.
#
$(call include-once,$(bld_root)/literals.make)
$(call include-once,$(bld_root)/message.make)
$(call include-once,$(bld_root)/dir.make)


# Static configuration.
#
$(call include,$(bld_root)/configuration-static.make)

ifneq ($(bld_root),$(scf_root))
$(call -include,$(scf_root)/configuration-static.make)
endif

# `disfigure' target.
#
.PHONY: disfigure
.PHONY: $(dcf_root)/.disfigure $(out_root)/.disfigure

disfigure:: $(build_absolute_clean_target)
disfigure:: $(dcf_root)/.disfigure
disfigure:: $(out_root)/.disfigure

ifneq ($(out_root),$(out_base))
.PHONY: $(out_base)/.disfigure
$(out_root)/.disfigure:: $(out_base)/.disfigure
endif

# Clean up generated makefiles.
#
ifndef %foreign%
ifneq ($(out_root),$(src_root))
$(out_base)/.disfigure::
	$(call message, rm $$1,rm -f $$1,$(out_base)/makefile)
endif
endif

# Dynamic configuration (interactive check is to make sure we only do
# this once).
#
ifdef %interactive%
ifneq ($(out_root),$(src_root))

$(call -include,$(dcf_root)/bootstrap-dynamic.make)

$(dcf_root)/bootstrap-dynamic.make: | $(dcf_root)/.
	$(call message,,echo "src_root := $(src_root)" >$@)

$(dcf_root)/.disfigure::
	$(call message, rm $$1,rm -f $$1,$(dcf_root)/bootstrap-dynamic.make)
endif
endif

ifeq ($(.DEFAULT_GOAL),disfigure)
.DEFAULT_GOAL :=
endif
