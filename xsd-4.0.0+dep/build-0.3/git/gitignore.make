# file      : build/git/gitignore.make
# copyright : Copyright (c) 2004-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

$(out_base)/.gitignore:
	@$(bld_root)/git/gitignore $(files) >$@

.PHONY: $(out_base)/.gitignore.clean
$(out_base)/.gitignore.clean:
	@rm -f $(basename $@)
