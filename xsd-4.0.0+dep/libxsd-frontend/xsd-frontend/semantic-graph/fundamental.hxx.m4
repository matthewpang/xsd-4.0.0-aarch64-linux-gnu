divert(-1)

# file      : xsd-frontend/semantic-graph/fundamental.hxx.m4
# copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include(`fundamental.m4')

define(`fundamental_type', `fundamental_type_impl(`make_class_name(`$1')', `$1')')

define(`fundamental_type_impl', `

      //
      //
      class $1: public virtual Type
      {
      public:
        $1 (Path const& file,
            unsigned long line,
            unsigned long column);
      };')
divert(0)dnl
dnl
dnl
dnl
// file      : xsd-frontend/semantic-graph/fundamental.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

// Note, that this file is automatically generated!
//

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_FUNDAMENTAL_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_FUNDAMENTAL_HXX

#include <xsd-frontend/semantic-graph/elements.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    namespace Fundamental
    {
      //
      //
      class Type: public virtual SemanticGraph::Type
      {
      protected:
        Type ();
      };
dnl
dnl Integers.
dnl
fundamental_type(`byte')
fundamental_type(`unsigned byte')
fundamental_type(`short')
fundamental_type(`unsigned short')
fundamental_type(`int')
fundamental_type(`unsigned int')
fundamental_type(`long')
fundamental_type(`unsigned long')
fundamental_type(`integer')
fundamental_type(`non positive integer')
fundamental_type(`non negative integer')
fundamental_type(`positive integer')
fundamental_type(`negative integer')
dnl
dnl Boolean.
dnl
fundamental_type(`boolean')
dnl
dnl Floats.
dnl
fundamental_type(`float')
fundamental_type(`double')
fundamental_type(`decimal')
dnl
dnl Strings.
dnl
fundamental_type(`string')
fundamental_type(`normalized string')
fundamental_type(`token')
fundamental_type(`name')
fundamental_type(`name token')
fundamental_type(`name tokens')
fundamental_type(`NC name')
fundamental_type(`language')
dnl
dnl Qualified name.
dnl
fundamental_type(`q name')
dnl
dnl ID/IDREF.
dnl
fundamental_type(`id')


      //
      //
      class IdRef: public virtual Type,
                   public virtual Specialization
      {
      public:
        IdRef (Path const& file,
               unsigned long line,
               unsigned long column);
      };


      //
      //
      class IdRefs: public virtual Type,
                    public virtual Specialization
      {
      public:
        IdRefs (Path const& file,
                unsigned long line,
                unsigned long column);
      };
dnl
dnl URI.
dnl
fundamental_type(`any URI')
dnl
dnl Binary.
dnl
fundamental_type(`base 64 binary')
fundamental_type(`hex binary')
dnl
dnl Date/time.
dnl
fundamental_type(`date')
fundamental_type(`date time')
fundamental_type(`duration')
fundamental_type(`day')
fundamental_type(`month')
fundamental_type(`month day')
fundamental_type(`year')
fundamental_type(`year month')
fundamental_type(`time')
dnl
dnl Entity.
dnl
fundamental_type(`entity')
fundamental_type(`entities')
dnl
dnl Notation.
dnl
fundamental_type(`notation')
dnl
    }
  }
}

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_FUNDAMENTAL_HXX