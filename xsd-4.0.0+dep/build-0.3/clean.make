# file      : build/clean.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

#@@ Maybe create file aliases.make for standard aliases
#   plus check for interactivity? What if some targets
#   are not defined?
#

ifdef %interactive%

.PHONY: clean

clean: $(out_base)/.clean

endif
