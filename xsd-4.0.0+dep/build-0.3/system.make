# file      : build/system.make
# copyright : Copyright (c) 2006-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

$(call include-once,$(bld_root)/system/configuration-rules.make,$(dcf_root))

$(call -include,$(dcf_root)/system/configuration-dynamic.make)

ifndef system_configuration

.NOTPARALLEL:

endif
