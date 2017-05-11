# file      : build/meta/vc12sln-functions.make
# copyright : Copyright (c) 2009-2012 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

# Process VC++ solution template and write output to $(dist_prefix)/<path>.
# Where path is computed as difference between src_base and src_root.
#
# Arguments:
#
# $1 - template path, if doesn't start with /, assume relative to src_base
# $2 - output name (optional)
# $3 - project suffix (-vc12.vcxproj by default)
# $4-9 - optional pairs of additional varibales and values $4=$5, $6=$7, etc
#
$(out_base)/%: meta-vc12sln = \
$(call meta-vc12sln-body,$(if $(filter /%,$1),$1,$(src_base)/$1),$(if \
$2,$2,$(notdir $1)),$(subst \
$(src_root),,$(src_base)),$(if $3,$3,-vc12.vcxproj),$4,$5,$6,$7,$8,$9)

# $1 - template
# $2 - output name
# $3 - difference between src_base and src_root with leading '\'
# $4 - project suffix
# $4-10 - additional varibales
#
$(out_base)/%: meta-vc12sln-body = \
$(call message,meta $(dist_prefix)$3/$2,$(if $5,$5='$6'; export $5; )$(if \
$7,$7='$8'; export $7; )$(if $9,$9='$(10)'; export $9; )$(bld_root)/meta/vcsln \
-v 12 -b $(dist_prefix)$3 -e '$4' -o $(dist_prefix)$3/$2 $1)
