# file      : build/dir.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

.PRECIOUS: %/.

%/. :
	$(call message,,mkdir -p $*)
