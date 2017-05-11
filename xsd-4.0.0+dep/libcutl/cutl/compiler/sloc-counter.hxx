// file      : cutl/compiler/sloc-counter.hxx
// copyright : Copyright (c) 2009-2013 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#ifndef CUTL_COMPILER_SLOC_COUNTER_HXX
#define CUTL_COMPILER_SLOC_COUNTER_HXX

#include <cstddef> // std::size_t

#include <cutl/compiler/code-stream.hxx>

namespace cutl
{
  namespace compiler
  {
    template <typename C>
    class sloc_counter: public code_stream<C>
    {
    public:
      sloc_counter (code_stream<C>& out);

      std::size_t
      count () const
      {
        return count_;
      }

    private:
      sloc_counter (sloc_counter const&);

      sloc_counter&
      operator= (sloc_counter const&);

    public:
      virtual void
      put (C);

      virtual void
      unbuffer ();

    private:
      void
      code (C);

      void
      c_comment (C);

      void
      cxx_comment (C);

      void
      char_literal (C);

      void
      string_literal (C);

    private:
      code_stream<C>& out_;
      std::size_t count_;

      C prev_; // Previous character or '\0'.
      bool code_counted_; // This code line has already been counted.

      enum construct
      {
        con_code,
        con_c_com,
        con_cxx_com,
        con_char_lit,
        con_string_lit
      } construct_;
    };
  }
}

#include <cutl/compiler/sloc-counter.txx>

#endif // CUTL_COMPILER_SLOC_COUNTER_HXX
