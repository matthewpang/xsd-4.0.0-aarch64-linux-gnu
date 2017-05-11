// file      : xsd/cxx/tree/stream-extraction-source.cxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cxx/tree/stream-extraction-source.hxx>

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

namespace CXX
{
  namespace Tree
  {
    namespace
    {
      struct List: Traversal::List, Context
      {
        List (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (Type& l)
        {
          String name (ename (l));

          // If renamed name is empty then we do not need to generate
          // anything for this type.
          //
          if (renamed_type (l, name) && !name)
            return;

          SemanticGraph::Type& item_type (l.argumented ().type ());
          String base (L"::xsd::cxx::tree::list< " +
                       item_type_name (item_type) + L", " + char_type);

          if (item_type.is_a<SemanticGraph::Fundamental::Double> ())
            base += L", ::xsd::cxx::tree::schema_type::double_";
          else if (item_type.is_a<SemanticGraph::Fundamental::Decimal> ())
            base += L", ::xsd::cxx::tree::schema_type::decimal";

          base += L" >";

          size_t n (0);
          NarrowStrings const& st (options.generate_extraction ());
          for (NarrowStrings::const_iterator i (st.begin ());
               i != st.end (); ++i)
          {
            os << name << "::" << endl
               << name << " (" << istream_type << "< " <<
              i->c_str () << " >& s," << endl
               << flags_type << " f," << endl
               << container << "* c)" << endl
               << ": " << any_simple_type << " (s, f, c)," << endl
               << "  " << base <<  " (s, f, this)"
               << "{"
               << "}";

            // Register with type map.
            //
            if (polymorphic &&
                polymorphic_p (l) &&
                (!anonymous_p (l) || anonymous_substitutes_p (l)))
            {
              // Note that we are using the original type name.
              //
              String const& name (ename (l));

              os << "static" << endl
                 << "const ::xsd::cxx::tree::stream_extraction_initializer< " <<
                poly_plate << ", " << i->c_str () << ", " << char_type <<
                ", " << name << " >" << endl
                 << "_xsd_" << name << "_stream_extraction_init_" <<
                n++ << " (" << endl
                 << strlit (l.name ()) << "," << endl
                 << strlit (xml_ns_name (l)) << ");"
                 << endl;
            }
          }
        }

      private:
        String
        item_type_name (SemanticGraph::Type& t)
        {
          std::wostringstream o;

          MemberTypeName type (*this, o);
          type.dispatch (t);

          return o.str ();
        }
      };


      struct Union: Traversal::Union, Context
      {
        Union (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (Type& u)
        {
          String name (ename (u));

          // If renamed name is empty then we do not need to generate
          // anything for this type.
          //
          if (renamed_type (u, name) && !name)
            return;

          String const& base (xs_string_type);

          size_t n (0);
          NarrowStrings const& st (options.generate_extraction ());
          for (NarrowStrings::const_iterator i (st.begin ()); i != st.end ();
               ++i)
          {
            os << name << "::" << endl
               << name << " (" << istream_type << "< " <<
              i->c_str () << " >& s," << endl
               << flags_type << " f," << endl
               << container << "* c)" << endl
               << ": " << base << " (s, f, c)"
               << "{"
               << "}";

            // Register with type map.
            //
            if (polymorphic &&
                polymorphic_p (u) &&
                (!anonymous_p (u) || anonymous_substitutes_p (u)))
            {
              // Note that we are using the original type name.
              //
              String const& name (ename (u));

              os << "static" << endl
                 << "const ::xsd::cxx::tree::stream_extraction_initializer< " <<
                poly_plate << ", " << i->c_str () << ", " << char_type <<
                ", " << name << " >" << endl
                 << "_xsd_" << name << "_stream_extraction_init_" <<
                n++ << " (" << endl
                 << strlit (u.name ()) << "," << endl
                 << strlit (xml_ns_name (u)) << ");"
                 << endl;
            }
          }
        }
      };


      struct Enumeration: Traversal::Enumeration, Context
      {
        Enumeration (Context& c)
            : Context (c), base_ (c)
        {
          inherits_base_ >> base_;
        }

        virtual void
        traverse (Type& e)
        {
          String name (ename (e));

          // If renamed name is empty then we do not need to generate
          // anything for this type.
          //
          if (renamed_type (e, name) && !name)
            return;

          bool string_based (false);
          {
            IsStringBasedType t (string_based);
            t.dispatch (e);
          }

          bool enum_based (false);
          if (string_based)
          {
            SemanticGraph::Enumeration* base_enum (0);
            IsEnumBasedType t (base_enum);
            t.dispatch (e);

            enum_based = (base_enum != 0);
          }

          String value;
          if (string_based)
            value = evalue (e);

          size_t n (0);
          NarrowStrings const& st (options.generate_extraction ());
          for (NarrowStrings::const_iterator i (st.begin ()); i != st.end ();
               ++i)
          {
            os << name << "::" << endl
               << name << " (" << istream_type << "< " <<
              i->c_str () << " >& s," << endl
               << flags_type << " f," << endl
               << container << "* c)" << endl
               << ": ";

            inherits (e, inherits_base_);

            if (string_based && !enum_based)
            {
              // Use copy c-tor to pass the flags and container.
              //
              os << " (" << endl;
              inherits (e, inherits_base_);
              os << " (_xsd_" << name << "_literals_[s.read_uint ()]), f, c)";
            }
            else
              os << " (s, f, c)";

            os << "{";

            if (string_based)
              os << "_xsd_" << name << "_convert ();";

            os << "}";

            // Register with type map.
            //
            if (polymorphic &&
                polymorphic_p (e) &&
                (!anonymous_p (e) || anonymous_substitutes_p (e)))
            {
              // Note that we are using the original type name.
              //
              String const& name (ename (e));

              os << "static" << endl
                 << "const ::xsd::cxx::tree::stream_extraction_initializer< " <<
                poly_plate << ", " << i->c_str () << ", " << char_type <<
                ", " << name << " >" << endl
                 << "_xsd_" << name << "_stream_extraction_init_" <<
                n++ << " (" << endl
                 << strlit (e.name ()) << "," << endl
                 << strlit (xml_ns_name (e)) << ");"
                 << endl;
            }
          }
        }

      private:
        Traversal::Inherits inherits_base_;
        BaseTypeName base_;
      };

      struct CtorMember: Traversal::Member, Context
      {
        CtorMember (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (SemanticGraph::Member& m)
        {
          if (skip (m))
            return;

          os << "," << endl
             << "  " << emember (m) << " (this)";
        }
      };

      struct CtorAny: Traversal::Any,
                      Traversal::AnyAttribute,
                      Context
      {
        CtorAny (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (SemanticGraph::Any& a)
        {
          String const& member (emember (a));
          String const& dom_doc (
            edom_document (
              dynamic_cast<SemanticGraph::Complex&> (a.scope ())));

          os << "," << endl
             << "  " << member << " (this->" << dom_doc << " ())";
        }

        virtual void
        traverse (SemanticGraph::AnyAttribute& a)
        {
          String const& member (emember (a));
          String const& dom_doc (
            edom_document (
              dynamic_cast<SemanticGraph::Complex&> (a.scope ())));

          os << "," << endl
             << "  " << member << " (this->" << dom_doc << " ())";
        }
      };

      struct Element: Traversal::Element, Context
      {
        Element (Context& c, String const& stream_)
            : Context (c), stream (stream_)
        {
        }

        virtual void
        traverse (Type& e)
        {
          if (skip (e))
            return;

          String const& member (emember (e));

          SemanticGraph::Type& t (e.type ());
          String type (etype (e));
          String tr (etraits (e)); // traits type name

          bool fund (false);
          {
            IsFundamentalType traverser (fund);
            traverser.dispatch (t);
          }

          // Figure out if we need to generate polymorphic code. If this
          // elemen's type is anonymous then we don't need to do anything.
          // Note that if the type is anonymous then it can't be derived
          // from which makes it impossible to substitute or dynamically-
          // type with xsi:type.
          //
          bool poly (polymorphic && polymorphic_p (t) && !anonymous_p (t));

          char const* r (
            (poly || !fund) && std >= cxx_version::cxx11
            ? "::std::move (r)"
            : "r");

          if (max (e) != 1)
          {
            // sequence
            //
            String container (econtainer (e));

            os << "{"
               << "::std::size_t n;"
               << "::xsd::cxx::tree::istream_common::as_size< " <<
              "::std::size_t > as (n);"
               << "s >> as;"
               << "if (n > 0)"
               << "{"
               << container << "& c (this->" << member << ");"
               << "c.reserve (n);"
               << "while (n--)"
               << "{";

            if (poly)
            {
              os << "bool d;"
                 << auto_ptr << "< " << type << " > r;"
                 << "s >> d;"
                 << endl
                 << "if (!d)" << endl
                 << "r.reset (new " << type << " (s, f, this));"
                 << "else"
                 << "{"
                 << auto_ptr << "< ::xsd::cxx::tree::type > tmp (" << endl
                 << "::xsd::cxx::tree::stream_extraction_map_instance< " <<
                poly_plate << ", " << stream << ", " << char_type <<
                " > ().extract (" << endl
                 << "s, f, this));"
                 << "r.reset (dynamic_cast< " << type << "* > (tmp.get ()));"
                 << "if (r.get ())" << endl
                 << "tmp.release ();"
                 << "else" << endl
                 << "throw ::xsd::cxx::tree::not_derived< " << char_type <<
                " > ();"
                 << "}";
            }
            else if (fund)
            {
              os << type << " r;"
                 << "s >> r;";
            }
            else
            {
              os << auto_ptr << "< " << type << " > r (" << endl
                 << tr << "::create (s, f, this));";
            }

            os << "c.push_back (" << r << ");"
               << "}" // while
               << "}" // if
               << "}";
          }
          else if (min (e) == 0)
          {
            // optional
            //
            os << "{"
               << "bool p;"
               << "s >> p;"
               << "if (p)"
               << "{";

            if (poly)
            {
              os << "bool d;"
                 << auto_ptr << "< " << type << " > r;"
                 << "s >> d;"
                 << endl
                 << "if (!d)" << endl
                 << "r.reset (new " << type << " (s, f, this));"
                 << "else"
                 << "{"
                 << auto_ptr << "< ::xsd::cxx::tree::type > tmp (" << endl
                 << "::xsd::cxx::tree::stream_extraction_map_instance< " <<
                poly_plate << ", " << stream << ", " << char_type <<
                " > ().extract (" << endl
                 << "s, f, this));"
                 << "r.reset (dynamic_cast< " << type << "* > (tmp.get ()));"
                 << "if (r.get ())" << endl
                 << "tmp.release ();"
                 << "else" << endl
                 << "throw ::xsd::cxx::tree::not_derived< " << char_type <<
                " > ();"
                 << "}";
            }
            else if (fund)
            {
              os << type << " r;"
                 << "s >> r;";
            }
            else
            {
              os << auto_ptr << "< " << type << " > r (" << endl
                 << tr << "::create (s, f, this));";
            }

            os << "this->" << member << ".set (" << r << ");"
               << "}" // if (p)
               << "}";
          }
          else
          {
            // one
            //
            os << "{";

            if (poly)
            {
              os << "bool d;"
                 << auto_ptr << "< " << type << " > r;"
                 << "s >> d;"
                 << endl
                 << "if (!d)" << endl
                 << "r.reset (new " << type << " (s, f, this));"
                 << "else"
                 << "{"
                 << auto_ptr << "< ::xsd::cxx::tree::type > tmp (" << endl
                 << "::xsd::cxx::tree::stream_extraction_map_instance< " <<
                poly_plate << ", " << stream << ", " << char_type <<
                " > ().extract (" << endl
                 << "s, f, this));"
                 << "r.reset (dynamic_cast< " << type << "* > (tmp.get ()));"
                 << "if (r.get ())" << endl
                 << "tmp.release ();"
                 << "else" << endl
                 << "throw ::xsd::cxx::tree::not_derived< " << char_type <<
                " > ();"
                 << "}";
            }
            else if (fund)
            {
              os << type << " r;"
                 << "s >> r;";
            }
            else
            {
              os << auto_ptr << "< " << type << " > r (" << endl
                 << tr << "::create (s, f, this));";
            }

            os << "this->" << member << ".set (" << r << ");"
               << "}";
          }
        }

      private:
        String stream;
      };

      struct Attribute: Traversal::Attribute, Context
      {
        Attribute (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (Type& a)
        {
          String const& member (emember (a));
          String type (etype (a));
          String tr (etraits (a)); // traits type name

          bool fund (false);
          {
            IsFundamentalType traverser (fund);
            traverser.dispatch (a.type ());
          }

          if (a.optional_p () && !a.default_p ())
          {
            os << "{"
               << "bool p;"
               << "s >> p;"
               << "if (p)"
               << "{";

            if (fund)
            {
              os << type << " r;"
                 << "s >> r;"
                 << "this->" << member << ".set (r);";
            }
            else
            {
              os << "this->" << member << ".set (" << tr <<
                "::create (s, f, this));";
            }

            os << "}" // if (p)
               << "}";
          }
          else
          {
            os << "{";

            if (fund)
            {
              os << type << " r;"
                 << "s >> r;"
                 << "this->" << member << ".set (r);";
            }
            else
            {
              os << "this->" << member << ".set (" << tr <<
                "::create (s, f, this));";
            }

            os << "}";
          }
        }
      };

      struct Complex: Traversal::Complex, Context
      {
        Complex (Context& c)
            : Context (c), base_ (c), ctor_any_ (c), ctor_member_ (c)
        {
          inherits_ >> base_;
          names_ctor_member_ >> ctor_member_;

          if (options.generate_wildcard ())
            names_ctor_member_ >> ctor_any_;
        }

        virtual void
        traverse (Type& c)
        {
          String name (ename (c));

          // If renamed name is empty then we do not need to generate
          // anything for this type.
          //
          if (renamed_type (c, name) && !name)
            return;

          bool has_members (has<Traversal::Member> (c));

          bool facets (false);
          if (c.inherits_p ())
          {
            // See if we have any facets that we need to handle.
            //
            using SemanticGraph::Restricts;
            using SemanticGraph::Fundamental::Decimal;

            if (Restricts* r = dynamic_cast<Restricts*> (&c.inherits ()))
            {
              if (!r->facet_empty () &&
                  (r->facet_find ("fractionDigits") != r->facet_end () ||
                   r->facet_find ("totalDigits") != r->facet_end ()) &&
                  ultimate_base (c).is_a<Decimal> ())
                facets = true;
            }
          }

          size_t n (0);
          NarrowStrings const& st (options.generate_extraction ());
          for (NarrowStrings::const_iterator i (st.begin ()); i != st.end ();
               ++i)
          {

            os << name << "::" << endl
               << name << " (" << istream_type << "< " <<
              i->c_str () << " >& s," << endl
               << flags_type << " f," << endl
               << container << "* c)" << endl
               << ": ";

            if (c.inherits_p ())
              inherits (c, inherits_);
            else
              os << any_type;

            os << " (s, f, c)";

            if (edom_document_member_p (c))
            {
              os << "," << endl
                 << "  " << edom_document_member (c) << " (" <<
                "::xsd::cxx::xml::dom::create_document< " << char_type <<
                " > ())";
            }

            names (c, names_ctor_member_);

            os << "{";

            if (facets)
              os << "this->_facet_table (_xsd_" << name << "_facet_table);"
                 << endl;

            if (has_members)
              os << "this->" << unclash (name, "parse") << " (s, f);";

            os << "}";

            // Parse
            //
            if (has_members)
            {
              os << "void " << name << "::" << endl
                 << unclash (name, "parse") << " (" <<
                istream_type << "< " << i->c_str () << " >& s," << endl
                 << flags_type << " f)"
                 << "{"
                 << "XSD_UNUSED (f);"; // Can be unused.
              {
                Element element (*this, *i);
                Attribute attribute (*this);
                Traversal::Names names_;

                names_ >> element;
                names_ >> attribute;

                names (c, names_);
              }

              os << "}";
            }


            // Register with type map.
            //
            if (polymorphic &&
                polymorphic_p (c) &&
                (!anonymous_p (c) || anonymous_substitutes_p (c)))
            {
              // Note that we are using the original type name.
              //
              String const& name (ename (c));

              os << "static" << endl
                 << "const ::xsd::cxx::tree::stream_extraction_initializer< " <<
                poly_plate << ", " << i->c_str () << ", " << char_type <<
                ", " << name << " >" << endl
                 << "_xsd_" << name << "_stream_extraction_init_" <<
                n++ << " (" << endl
                 << strlit (c.name ()) << "," << endl
                 << strlit (xml_ns_name (c)) << ");"
                 << endl;
            }
          }
        }

      private:
        Traversal::Inherits inherits_;
        BaseTypeName base_;

        CtorAny ctor_any_;
        CtorMember ctor_member_;

        Traversal::Names names_ctor_member_;
      };
    }

    void
    generate_stream_extraction_source (Context& ctx)
    {
      if (ctx.polymorphic)
      {
        NarrowStrings const& st (ctx.options.generate_extraction ());

        ctx.os << "#include <xsd/cxx/tree/stream-extraction-map.hxx>" << endl
               << endl;

        bool import_maps (ctx.options.import_maps ());
        bool export_maps (ctx.options.export_maps ());

        if (import_maps || export_maps)
        {
          ctx.os << "#ifndef XSD_NO_EXPORT" << endl
                 << endl
                 << "namespace xsd"
                 << "{"
                 << "namespace cxx"
                 << "{"
                 << "namespace tree"
                 << "{";

          for (NarrowStrings::const_iterator i (st.begin ()); i != st.end ();
               ++i)
          {
            String stream (*i);

            ctx.os << "#ifdef _MSC_VER" << endl;

            if (export_maps)
              ctx.os << "template struct __declspec (dllexport) " <<
                "stream_extraction_plate< " << ctx.poly_plate << ", " <<
                stream << ", " << ctx.char_type << " >;";

            if (import_maps)
              ctx.os << "template struct __declspec (dllimport) " <<
                "stream_extraction_plate< " << ctx.poly_plate << ", " <<
                stream << ", " << ctx.char_type << " >;";

            ctx.os << "#elif defined(__GNUC__) && __GNUC__ >= 4" << endl
                   << "template struct __attribute__ ((visibility(\"default\"))) " <<
              "stream_extraction_plate< " << ctx.poly_plate << ", " <<
              stream << ", " << ctx.char_type << " >;"
                   << "#elif defined(XSD_MAP_VISIBILITY)" << endl
                   << "template struct XSD_MAP_VISIBILITY " <<
              "stream_extraction_plate< " << ctx.poly_plate << ", " <<
              stream << ", " << ctx.char_type << " >;"
                   << "#endif" << endl;
          }

          ctx.os << "}"  // tree
                 << "}"  // cxx
                 << "}"  // xsd
                 << "#endif // XSD_NO_EXPORT" << endl
                 << endl;

        }

        ctx.os << "namespace _xsd"
               << "{";

        size_t n (0);
        for (NarrowStrings::const_iterator i (st.begin ()); i != st.end ();
             ++i)
        {
          String stream (*i);

          ctx.os << "static" << endl
                 << "const ::xsd::cxx::tree::stream_extraction_plate< " <<
            ctx.poly_plate << ", " << stream << ", " << ctx.char_type <<
            " >" << endl
                 << "stream_extraction_plate_init_" << n++ << ";";
        }

        ctx.os << "}";
      }

      Traversal::Schema schema;

      Sources sources;
      Traversal::Names names_ns, names;

      Namespace ns (ctx);

      List list (ctx);
      Union union_ (ctx);
      Complex complex (ctx);
      Enumeration enumeration (ctx);

      schema >> sources >> schema;
      schema >> names_ns >> ns >> names;

      names >> list;
      names >> union_;
      names >> complex;
      names >> enumeration;

      schema.dispatch (ctx.schema_root);
    }
  }
}
