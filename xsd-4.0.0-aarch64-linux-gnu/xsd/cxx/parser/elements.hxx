// file      : xsd/cxx/parser/elements.hxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef CXX_PARSER_ELEMENTS_HXX
#define CXX_PARSER_ELEMENTS_HXX

#include <sstream>

#include <cutl/re.hxx>

#include <types.hxx>

#include <cxx/elements.hxx>

#include <cxx/parser/options.hxx>

namespace CXX
{
  namespace Parser
  {
    struct Content
    {
      enum Value
      {
        simple,
        complex,
        mixed
      };
    };

    //
    //
    class Context: public CXX::Context
    {
    public:
      typedef cutl::re::regexsub Regex;

      typedef Parser::options options_type;

    public:
      Context (std::wostream&,
               SemanticGraph::Schema&,
               SemanticGraph::Path const& path,
               options_type const&,
               StringLiteralMap const*,
               Regex const* hxx_expr,
               Regex const* ixx_expr,
               Regex const* hxx_impl_expr);

    protected:
      Context (Context& c);
      Context (Context& c, std::wostream& o);

    public:
      bool
      restriction_p (SemanticGraph::Complex& c) const
      {
        if (c.inherits_p () &&
            c.inherits ().is_a<SemanticGraph::Restricts> ())
        {
          // Restriction of anyType is a special case.
          //
          return !c.inherits ().base ().is_a<SemanticGraph::AnyType> ();
        }

        return false;
      }

    public:
      static Content::Value
      content (SemanticGraph::Complex&);

    public:
      static bool
      anonymous (SemanticGraph::Type&);

    public:
      static String const&
      ret_type (SemanticGraph::Type&);

      static String const&
      arg_type (SemanticGraph::Type&);

      static String const&
      post_name (SemanticGraph::Type&);

    public:
      static String const&
      eparser (SemanticGraph::Member&);

      static String const&
      emember (SemanticGraph::Member&);

      static String const&
      emember_map (SemanticGraph::Member&);

    public:
      static String const&
      eimpl (SemanticGraph::Type&);

    public:
      options_type const& options;
      String& xml_parser;
      String& simple_base;
      String& complex_base;
      String& list_base;
      String& cout_inst;
      String& cerr_inst;
      String& parser_map;
      String& std_string_type;
      bool& validation;
      bool& polymorphic;

      Regex const* hxx_expr;
      Regex const* ixx_expr;
      Regex const* hxx_impl_expr;

    private:
      String xml_parser_;
      String simple_base_;
      String complex_base_;
      String list_base_;
      String cout_inst_;
      String cerr_inst_;
      String parser_map_;
      String std_string_type_;
      bool validation_;
      bool polymorphic_;
    };

    //
    //
    struct RequiredAttributeTest: Traversal::Attribute
    {
      RequiredAttributeTest (bool& result)
          : result_ (result)
      {
      }

      virtual void
      traverse (Type& a)
      {
        if (!result_ && !a.optional_p ())
          result_ = true;
      }

    private:
      bool& result_;
    };

    //
    //
    struct ParserParamDecl: Traversal::Complex,
                            Traversal::List,
                            Traversal::Member,
                            Context
    {
      ParserParamDecl (Context& c, bool name_arg)
          : Context (c), first_ (true), name_arg_ (name_arg)
      {
        inherits_ >> *this;
        names_ >> *this;
      }

      virtual void
      traverse (SemanticGraph::Complex& c)
      {
        inherits (c, inherits_);

        if (!restriction_p (c))
          names (c, names_);
      }

      virtual void
      traverse (SemanticGraph::List& l)
      {
        if (!first_)
          os << "," << endl;
        else
          first_ = false;

        os << fq_name (l.argumented ().type ()) << "&";

        if (name_arg_)
          os << " " << ename (l) << "_item";
        else
          os << " /* " << comment (l.name ()) << " item */";
      }

      virtual void
      traverse (SemanticGraph::Member& m)
      {
        if (skip (m)) return;

        if (!first_)
          os << "," << endl;
        else
          first_ = false;

        os << fq_name (m.type ()) << "&";

        if (name_arg_)
          os << " " << ename (m);
        else
          os << " /* " << comment (m.name ()) << " */";
      }

    private:
      Traversal::Inherits inherits_;
      Traversal::Names names_;

      bool first_;
      bool name_arg_;
    };


    //
    //
    struct TypeForward: Traversal::Type, Context
    {
      TypeForward (Context& c, char const* name_key)
          : Context (c), name_key_ (name_key)
      {
      }

      virtual void
      traverse (SemanticGraph::Type& t);

    private:
      char const* name_key_;
    };

    struct Includes: Traversal::Imports,
                     Traversal::Includes
    {
      enum Type
      {
        header,
        source,
        impl_header
      };

      Includes (Context& c, Type t)
          : ctx_ (c),
            type_ (t),
            namespace_ (c),
            type_forward_ (c, t == header ? "name" : "impl")
      {
        schema_ >> schema_names_ >> namespace_ >> names_ >> type_forward_;
      }

      virtual void
      traverse (SemanticGraph::Imports& i)
      {
        traverse_ (i);
      }

      virtual void
      traverse (SemanticGraph::Includes& i)
      {
        traverse_ (i);
      }

    private:
      void
      traverse_ (SemanticGraph::Uses&);

    private:
      Context& ctx_;
      Type type_;

      Traversal::Schema schema_;
      Traversal::Names schema_names_;
      Namespace namespace_;
      Traversal::Names names_;
      TypeForward type_forward_;
    };

    // Find root element for the test driver.
    //
    struct RootElement: Traversal::Element
    {
      typedef Parser::options options_type;

      RootElement (options_type const& options,
                   SemanticGraph::Element*& element)
          : options_ (options), element_ (element)
      {
      }

      virtual void
      traverse (Type& e)
      {
        if (options_.root_element_first ())
        {
          if (element_ == 0)
            element_ = &e;
        }
        else if (String name = options_.root_element ())
        {
          if (e.name () == name)
            element_ = &e;
        }
        else
          element_ = &e; // Cover root-element-last and no option.
      }

    private:
      options_type const& options_;
      SemanticGraph::Element*& element_;
    };
  }
}

#endif  // CXX_PARSER_ELEMENTS_HXX
