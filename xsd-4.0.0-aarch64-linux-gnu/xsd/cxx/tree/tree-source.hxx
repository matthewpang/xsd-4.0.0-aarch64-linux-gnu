// file      : xsd/cxx/tree/tree-source.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef CXX_TREE_TREE_SOURCE_HXX
#define CXX_TREE_TREE_SOURCE_HXX

#include <cxx/tree/elements.hxx>

namespace CXX
{
  namespace Tree
  {
    void
    generate_tree_source (Context&, size_t first, size_t last);
  }
}

#endif  // CXX_TREE_TREE_SOURCE_HXX
