// file      : xsd/cxx/tree/name-processor.cxx
// copyright : Copyright (c) 2006-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

#include <cutl/re.hxx>

#include <cxx/tree/default-value.hxx>
#include <cxx/tree/name-processor.hxx>

using namespace std;

namespace CXX
{
  namespace Tree
  {
    namespace
    {
      //
      //
      typedef set<String> NameSet;

      class Context: public Tree::Context
      {
      public:
        struct Failed {};

        Context (Tree::options const& ops,
                 Counts const& counts,
                 bool generate_xml_schema,
                 SemanticGraph::Schema& root,
                 SemanticGraph::Path const& path,
                 StringLiteralMap const& map)
            : Tree::Context (std::wcerr,
                             root,
                             path,
                             ops,
                             counts,
                             generate_xml_schema,
                             &map,
                             0,
                             0,
                             0),
              global_type_names (global_type_names_),
              global_element_names (global_element_names_),
              detach (ops.generate_detach ()),
              type_regex (type_regex_),
              accessor_regex (accessor_regex_),
              one_accessor_regex (one_accessor_regex_),
              opt_accessor_regex (opt_accessor_regex_),
              seq_accessor_regex (seq_accessor_regex_),
              modifier_regex (modifier_regex_),
              one_modifier_regex (one_modifier_regex_),
              opt_modifier_regex (opt_modifier_regex_),
              seq_modifier_regex (seq_modifier_regex_),
              parser_regex (parser_regex_),
              serializer_regex (serializer_regex_),
              const_regex (const_regex_),
              enumerator_regex (enumerator_regex_),
              element_type_regex (element_type_regex_)
        {
          NarrowString tn (options.type_naming ());
          NarrowString fn (options.function_naming ());

          // Type name regex.
          //
          {
            // Predefined rules. The most frequently used come last: global
            // names, two components (<name>,type), three components
            // (<name>,const,iterator), and one component (value in enum).
            //
            if (tn == "knr")
            {
              type_regex.push_back ("/(?:[^ ]* )?([^,]+),([^,]+),([^,]+),([^,]+)/$1_$2_$3_$4/");
              type_regex.push_back ("/(?:[^ ]* )?([^,]+),([^,]+),([^,]+)/$1_$2_$3/");
              type_regex.push_back ("/(?:[^ ]* )?([^,]+),([^,]+)/$1_$2/");
              type_regex.push_back ("/(?:[^ ]* )?([^,]+)/$1/");

              /*
              type_regex.push_back ("/([^,]+)/$1/");
              type_regex.push_back ("/([^,]+),([^,]+),([^,]+),([^,]+)/$1_$2_$3_$4/");
              type_regex.push_back ("/([^,]+),([^,]+),([^,]+)/$1_$2_$3/");
              type_regex.push_back ("/([^,]+),([^,]+)/$1_$2/");
              type_regex.push_back ("/[^ ]* (.+)/$1/");
              */
            }
            else
            {
              // Upper camel case or Java.
              //
              type_regex.push_back ("/(?:[^ ]* )?([^,]+),([^,]+),([^,]+),([^,]+)/\\u$1\\u$2\\u$3\\u$4/");
              type_regex.push_back ("/(?:[^ ]* )?([^,]+),([^,]+),([^,]+)/\\u$1\\u$2\\u$3/");
              type_regex.push_back ("/(?:[^ ]* )?([^,]+),([^,]+)/\\u$1\\u$2/");
              type_regex.push_back ("/(?:[^ ]* )?([^,]+)/\\u$1/");

              /*
              type_regex.push_back ("/([^,]+)/\\u$1/");
              type_regex.push_back ("/([^,]+),([^,]+),([^,]+),([^,]+)/\\u$1\\u$2\\u$3\\u$4/");
              type_regex.push_back ("/([^,]+),([^,]+),([^,]+)/\\u$1\\u$2\\u$3/");
              type_regex.push_back ("/([^,]+),([^,]+)/\\u$1\\u$2/");
              type_regex.push_back ("/[^ ]* (.+)/\\u$1/");
              */

            }

            compile_regex (options.type_regex (), type_regex, "type");
          }

          // Accessor name regex.
          //
          {
            // Predefined rules. The most frequently used come last: one
            // component, three components (<name>,default,value) and two
            // component (dom,document).
            //
            if (fn == "knr")
            {
              accessor_regex.push_back ("/([^,]+),([^,]+)/$1_$2/");
              accessor_regex.push_back ("/([^,]+),([^,]+),([^,]+)/$1_$2_$3/");
              accessor_regex.push_back ("/([^,]+)/$1/");
            }
            else if (fn == "lcc")
            {
              accessor_regex.push_back ("/([^,]+),([^,]+)/\\l$1\\u$2/");
              accessor_regex.push_back ("/([^,]+),([^,]+),([^,]+)/\\l$1\\u$2\\u$3/");
              accessor_regex.push_back ("/([^,]+)/\\l$1/");
            }
            else
            {
              // Java: add get.
              //
              accessor_regex.push_back ("/([^,]+),([^,]+)/get\\u$1\\u$2/");
              accessor_regex.push_back ("/([^,]+),([^,]+),([^,]+)/get\\u$1\\u$2\\u$3/");
              accessor_regex.push_back ("/([^,]+)/get\\u$1/");
            }

            compile_regex (options.accessor_regex (),
                           accessor_regex,
                           "accessor");

            compile_regex (options.one_accessor_regex (),
                           one_accessor_regex,
                           "one accessor");

            compile_regex (options.opt_accessor_regex (),
                           opt_accessor_regex,
                           "optional accessor");

            compile_regex (options.seq_accessor_regex (),
                           seq_accessor_regex,
                           "sequence accessor");
          }

          // Modifier name regex.
          //
          {
            if (fn == "knr")
            {
              // any,attribute
              //
              modifier_regex.push_back ("/([^,]+),([^,]+)/$1_$2/");
            }
            else if (fn == "lcc")
            {
              modifier_regex.push_back ("/([^,]+),([^,]+)/\\l$1\\u$2/");
              modifier_regex.push_back ("/([^,]+)/\\l$1/");
            }
            else
            {
              // Java: add set.
              //
              modifier_regex.push_back ("/([^,]+),([^,]+)/set\\u$1\\u$2/");
              modifier_regex.push_back ("/([^,]+)/set\\u$1/");
              modifier_regex.push_back ("/detach,([^,]+)/detach\\u$1/");
            }

            compile_regex (options.modifier_regex (),
                           modifier_regex,
                           "modifier");

            compile_regex (options.one_modifier_regex (),
                           one_modifier_regex,
                           "one modifier");

            compile_regex (options.opt_modifier_regex (),
                           opt_modifier_regex,
                           "optional modifier");

            compile_regex (options.seq_modifier_regex (),
                           seq_modifier_regex,
                           "sequence modifier");
          }

          // Parser name regex.
          //
          {
            if (fn == "lcc")
            {
              parser_regex.push_back ("/(.+)/\\l$1/");
            }
            else if (fn == "java")
            {
              // Java: add parse.
              //
              parser_regex.push_back ("/(.+)/parse\\u$1/");
            }

            compile_regex (options.parser_regex (), parser_regex, "parser");
          }

          // Serializer name regex.
          //
          {
            if (fn == "lcc")
            {
              serializer_regex.push_back ("/(.+)/\\l$1/");
            }
            else if (fn == "java")
            {
              // Java: add serialize.
              //
              serializer_regex.push_back ("/(.+)/serialize\\u$1/");
            }

            compile_regex (options.serializer_regex (),
                           serializer_regex,
                           "serializer");
          }

          // Const regex.
          //
          {
            if (fn == "knr")
            {
              const_regex.push_back ("/([^,]+),([^,]+),([^,]+)/$1_$2_$3/");
              const_regex.push_back ("/([^,]+),([^,]+)/$1_$2/");
            }
            else if (fn == "lcc")
            {
              const_regex.push_back ("/([^,]+),([^,]+),([^,]+)/\\l$1_\\u$2_\\u$3/");
              const_regex.push_back ("/([^,]+),([^,]+)/\\l$1\\u$2/");
            }
            else
            {
              // Java: all uppercase.
              //
              const_regex.push_back ("/([^,]+),([^,]+),([^,]+)/\\U$1_$2_$3/");
              const_regex.push_back ("/([^,]+),([^,]+)/\\U$1_$2/");
            }

            compile_regex (options.const_regex (), const_regex, "const");
          }

          // Enumerator name regex.
          //
          {
            // By default map an empty enumerator to the 'empty' word.
            //
            enumerator_regex.push_back ("/^$/empty/");

            compile_regex (options.enumerator_regex (),
                           enumerator_regex,
                           "enumerator");
          }

          // Element type regex.
          //
          compile_regex (options.element_type_regex (),
                         element_type_regex,
                         "element_type");
        }

      protected:
        Context (Context& c)
            : Tree::Context (c),
              global_type_names (c.global_type_names),
              global_element_names (c.global_element_names),
              detach (c.detach),
              type_regex (c.type_regex),
              accessor_regex (c.accessor_regex),
              one_accessor_regex (c.one_accessor_regex),
              opt_accessor_regex (c.opt_accessor_regex),
              seq_accessor_regex (c.seq_accessor_regex),
              modifier_regex (c.modifier_regex),
              one_modifier_regex (c.one_modifier_regex),
              opt_modifier_regex (c.opt_modifier_regex),
              seq_modifier_regex (c.seq_modifier_regex),
              parser_regex (c.parser_regex),
              serializer_regex (c.serializer_regex),
              const_regex (c.const_regex),
              enumerator_regex (c.enumerator_regex),
              element_type_regex (c.element_type_regex)
        {
        }

      public:
        typedef cutl::re::wregexsub Regex;
        typedef cutl::re::wformat RegexFormat;

        struct RegexVector: vector<Regex>
        {
          void
          push_back (String const& r)
          {
            vector<Regex>::push_back (Regex (r));
          }
        };

        String
        process_regex (String const& name,
                       RegexVector const& rv,
                       String const& id)
        {
          bool trace (options.name_regex_trace ());

          if (trace)
            os << id << " name: '" << name << "'" << endl;

          for (RegexVector::const_reverse_iterator i (rv.rbegin ());
               i != rv.rend (); ++i)
          {
            if (trace)
              os << "try: '" << i->regex () << "' : ";

            if (i->match (name))
            {
              String r (i->replace (name));

              if (trace)
                os << "'" << r << "' : +" << endl;

              return r;
            }

            if (trace)
              os << '-' << endl;
          }

          return name;
        }

        String
        process_regex (String const& name,
                       RegexVector const& primary,
                       RegexVector const& backup,
                       String const& id)
        {
          bool trace (options.name_regex_trace ());

          if (trace)
            os << id << " name: '" << name << "'" << endl;

          for (RegexVector::const_reverse_iterator i (primary.rbegin ());
               i != primary.rend (); ++i)
          {
            if (trace)
              os << "try: '" << i->regex () << "' : ";

            if (i->match (name))
            {
              String r (i->replace (name));

              if (trace)
                os << "'" << r << "' : +" << endl;

              return r;
            }

            if (trace)
              os << '-' << endl;
          }

          for (RegexVector::const_reverse_iterator i (backup.rbegin ());
               i != backup.rend (); ++i)
          {
            if (trace)
              os << "try: '" << i->regex () << "' : ";

            if (i->match (name))
            {
              String r (i->replace (name));

              if (trace)
                os << "'" << r << "' : +" << endl;

              return r;
            }

            if (trace)
              os << '-' << endl;
          }

          return name;
        }

        String
        process_regex (String const& ns,
                       String const& name,
                       RegexVector const& rv,
                       String const& id)
        {
          String s (ns + L' ' + name);
          bool trace (options.name_regex_trace ());

          if (trace)
            os << id << " name: '" << s << "'" << endl;

          for (RegexVector::const_reverse_iterator i (rv.rbegin ());
               i != rv.rend (); ++i)
          {
            if (trace)
              os << "try: '" << i->regex () << "' : ";

            if (i->match (s))
            {
              String r (i->replace (s));

              if (trace)
                os << "'" << r << "' : +" << endl;

              return r;
            }

            if (trace)
              os << '-' << endl;
          }

          return name;
        }

        String
        process_regex (String const& ns,
                       String const& name,
                       RegexVector const& primary,
                       RegexVector const& backup,
                       String const& id)
        {
          String s (ns + L' ' + name);
          bool trace (options.name_regex_trace ());

          if (trace)
            os << id << " name: '" << s << "'" << endl;

          for (RegexVector::const_reverse_iterator i (primary.rbegin ());
               i != primary.rend (); ++i)
          {
            if (trace)
              os << "try: '" << i->regex () << "' : ";

            if (i->match (s))
            {
              String r (i->replace (s));

              if (trace)
                os << "'" << r << "' : +" << endl;

              return r;
            }

            if (trace)
              os << '-' << endl;
          }

          for (RegexVector::const_reverse_iterator i (backup.rbegin ());
               i != backup.rend (); ++i)
          {
            if (trace)
              os << "try: '" << i->regex () << "' : ";

            if (i->match (s))
            {
              String r (i->replace (s));

              if (trace)
                os << "'" << r << "' : +" << endl;

              return r;
            }

            if (trace)
              os << '-' << endl;
          }

          return name;
        }

      public:
        String
        find_name (String const& base_name,
                   NameSet& set,
                   bool insert = true)
        {
          String name (base_name);

          for (size_t i (1); set.find (name) != set.end (); ++i)
          {
            std::wostringstream os;
            os << i;
            name = base_name + os.str ();
          }

          if (insert)
            set.insert (name);

          return name;
        }

      private:
        void
        compile_regex (NarrowStrings const& sv,
                       RegexVector& rv,
                       String const& id)
        {
          for (NarrowStrings::const_iterator i (sv.begin ()); i != sv.end ();
               ++i)
          {
            try
            {
              rv.push_back (*i);
            }
            catch (RegexFormat const& e)
            {
              os << "error: invalid " << id << " name regex: '" <<
                e.regex () << "': " << e.description ().c_str () << endl;

              throw Failed ();
            }
          }
        }

      private:
        map<String, NameSet> global_type_names_;
        map<String, NameSet> global_element_names_;

        RegexVector type_regex_;
        RegexVector accessor_regex_;
        RegexVector one_accessor_regex_;
        RegexVector opt_accessor_regex_;
        RegexVector seq_accessor_regex_;
        RegexVector modifier_regex_;
        RegexVector one_modifier_regex_;
        RegexVector opt_modifier_regex_;
        RegexVector seq_modifier_regex_;
        RegexVector parser_regex_;
        RegexVector serializer_regex_;
        RegexVector const_regex_;
        RegexVector enumerator_regex_;
        RegexVector element_type_regex_;

      public:
        map<String, NameSet>& global_type_names;
        map<String, NameSet>& global_element_names;

        bool detach;

        RegexVector& type_regex;
        RegexVector& accessor_regex;
        RegexVector& one_accessor_regex;
        RegexVector& opt_accessor_regex;
        RegexVector& seq_accessor_regex;
        RegexVector& modifier_regex;
        RegexVector& one_modifier_regex;
        RegexVector& opt_modifier_regex;
        RegexVector& seq_modifier_regex;
        RegexVector& parser_regex;
        RegexVector& serializer_regex;
        RegexVector& const_regex;
        RegexVector& enumerator_regex;
        RegexVector& element_type_regex;
      };

      //
      //
      struct Enumerator: Traversal::Enumerator, Context
      {
        Enumerator (Context& c, NameSet& set)
            : Context (c), set_ (set)
        {
        }

        virtual void
        traverse (Type& e)
        {
          // Process the name with enumerator name regex.
          //
          String name (
            process_regex (e.name (), enumerator_regex, L"enumerator"));

          // Escape and unclash.
          //
          name = find_name (escape (name), set_);
          e.context ().set ("name", name);
        }

      private:
        NameSet& set_;
      };

      //
      //
      struct Enumeration: Traversal::Enumeration, Context
      {
        Enumeration (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (Type& e)
        {
          // Use processed name.
          //
          String name (e.context ().get<String> ("name"));

          // If renamed name is empty then we are not generating
          // anything for this type and name processing is not
          // required.
          //
          if (renamed_type (e, name) && !name)
            return;

          NameSet enum_set;
          enum_set.insert (name);

          Enumerator enumerator (*this, enum_set);
          Traversal::Names names (enumerator);

          Traversal::Enumeration::names (e, names);

          // Assign name to the value type. First process the name
          // with type name regex.
          //
          String value_name (
            escape (process_regex ("value", type_regex, L"type")));
          e.context ().set ("value", find_name (value_name, enum_set));
        }
      };

      //
      //
      struct PrimaryMember: Traversal::Member, Context
      {
        PrimaryMember (Context& c, NameSet& name_set, NameSet& stem_set)
            : Context (c), name_set_ (name_set), stem_set_ (stem_set)
        {
        }

        virtual void
        traverse (Type& m)
        {
          if (Tree::Context::skip (m))
            return;

          String stem (find_name (m.name (), stem_set_));

          m.context ().set ("stem", stem);
          m.context ().set ("name",
                            find_name (escape (stem), name_set_, false));
        }

      private:
        NameSet& name_set_;
        NameSet& stem_set_;
      };

      //
      //
      struct DerivedMember: Traversal::Member, Context
      {
        DerivedMember (Context& c, NameSet& name_set)
            : Context (c), name_set_ (name_set)
        {
        }

        virtual void
        traverse (Type& m)
        {
          if (Tree::Context::skip (m))
            return;

          SemanticGraph::Complex& c (
            dynamic_cast<SemanticGraph::Complex&> (m.scope ()));

          size_t max (Tree::Context::max (m));
          size_t min (Tree::Context::min (m));

          String const& s (m.context ().get<String> ("stem"));
          String const& b (m.context ().get<String> ("name"));

          bool def_attr (m.default_p () &&
                         m.is_a<SemanticGraph::Attribute> ());

          // Accessors/modifiers. Note that we postpone inserting
          // the names into the name_set to avoid over-escaping.
          //
          String an, mn;

          if (max != 1)
          {
            an = find_name (
              escape (process_regex (s,
                                     seq_accessor_regex,
                                     accessor_regex,
                                     L"sequence accessor")),
              name_set_,
              false);

            mn = find_name (
              escape (process_regex (s,
                                     seq_modifier_regex,
                                     modifier_regex,
                                     L"sequence modifier")),
              name_set_,
              false);
          }
          else if (min == 0 && !def_attr)
          {
            an = find_name (
              escape (process_regex (s,
                                     opt_accessor_regex,
                                     accessor_regex,
                                     L"optional accessor")),
              name_set_,
              false);

            mn = find_name (
              escape (process_regex (s,
                                     opt_modifier_regex,
                                     modifier_regex,
                                     L"optional modifier")),
              name_set_,
              false);
          }
          else
          {
            an = find_name (
              escape (process_regex (s,
                                     one_accessor_regex,
                                     accessor_regex,
                                     L"one accessor")),
              name_set_,
              false);

            mn = find_name (
              escape (process_regex (s,
                                     one_modifier_regex,
                                     modifier_regex,
                                     L"one modifier")),
              name_set_,
              false);
          }

          m.context ().set ("aname", an);
          m.context ().set ("mname", mn);

          name_set_.insert (b);

          if (an != b)
            name_set_.insert (an);

          if (mn != b && mn != an)
            name_set_.insert (mn);

          // Detach.
          //
          if (detach && max == 1 && (min == 1 || def_attr))
          {
            String dn (find_name (
                         escape (process_regex (L"detach," + s,
                                                one_modifier_regex,
                                                modifier_regex,
                                                L"one modifier")),
                         name_set_));

            m.context ().set ("dname", dn);
          }

          // Types.
          //
          m.context ().set (
            "type",
            find_name (
              escape (process_regex (s + L",type", type_regex, L"type")),
              name_set_));

          m.context ().set (
            "traits",
            find_name (
              escape (process_regex (s + L",traits", type_regex, L"type")),
              name_set_));

          if (max != 1)
          {
            m.context ().set (
              "container",
              find_name (
                escape (process_regex (s + L",sequence", type_regex, L"type")),
                name_set_));

            m.context ().set (
              "iterator",
              find_name (
                escape (process_regex (s + L",iterator", type_regex, L"type")),
                name_set_));

            m.context ().set (
              "const-iterator",
              find_name (
                escape (
                  process_regex (s + L",const,iterator", type_regex, L"type")),
                name_set_));
          }
          else if (min == 0 && !def_attr)
          {
            m.context ().set (
              "container",
              find_name (
                escape (process_regex (s + L",optional", type_regex, L"type")),
                name_set_));
          }

          // Data member.
          //
          m.context ().set ("member", find_name (b + L"_", name_set_));

          // Default value.
          //
          if (m.default_p ())
          {
            bool simple (true);

            if (m.is_a<SemanticGraph::Element> ())
            {
              IsSimpleType test (simple);
              test.dispatch (m.type ());
            }

            if (simple)
            {
              String an (
                escape (
                  process_regex (
                    s + L",default,value", accessor_regex, L"accessor")));

              m.context ().set ("default-value", find_name (an, name_set_));

              bool lit (false);
              {
                IsLiteralValue test (lit);
                test.dispatch (m.type ());
              }

              if (!lit)
              {
                m.context ().set (
                  "default-value-member",
                  find_name (b + L"_default_value_", name_set_));
              }
            }
          }

          // Element id.
          //
          if (m.is_a<SemanticGraph::Element> () && ordered_p (c))
          {
            String id (
              escape (
                process_regex (
                  s + L",id", const_regex, L"const")));

            m.context ().set ("ordered-id-name", find_name (id, name_set_));
          }
        }

      private:
        NameSet& name_set_;
      };


      //
      //
      struct Any: Traversal::Any, Traversal::AnyAttribute, Context
      {
        Any (Context& c,
             NameSet& name_set,
             NameSet& stem_set,
             bool& has_wildcard)
            : Context (c),
              name_set_ (name_set),
              stem_set_ (stem_set),
              has_wildcard_ (has_wildcard)
        {
        }

        virtual void
        traverse (SemanticGraph::Any& a)
        {
          SemanticGraph::Complex& c (
            dynamic_cast<SemanticGraph::Complex&> (a.scope ()));

          size_t max (Tree::Context::max (a));
          size_t min (Tree::Context::min (a));

          String s (find_name (L"any", stem_set_));

          String b (find_name (escape (s), name_set_, false));
          a.context ().set ("name", b);

          // Accessors/modifiers. Note that we postpone inserting the
          // names into the name_set to avoid over-escaping.
          //
          String an, mn;

          if (max != 1)
          {
            an = find_name (
              escape (process_regex (s,
                                     seq_accessor_regex,
                                     accessor_regex,
                                     L"sequence accessor")),
              name_set_,
              false);

            mn = find_name (
              escape (process_regex (s,
                                     seq_modifier_regex,
                                     modifier_regex,
                                     L"sequence modifier")),
              name_set_,
              false);
          }
          else if (min == 0)
          {
            an = find_name (
              escape (process_regex (s,
                                     opt_accessor_regex,
                                     accessor_regex,
                                     L"optional accessor")),
              name_set_,
              false);

            mn = find_name (
              escape (process_regex (s,
                                     opt_modifier_regex,
                                     modifier_regex,
                                     L"optional modifier")),
              name_set_,
              false);
          }
          else
          {
            an = find_name (
              escape (process_regex (s,
                                     one_accessor_regex,
                                     accessor_regex,
                                     L"one accessor")),
              name_set_,
              false);

            mn = find_name (
              escape (process_regex (s,
                                     one_modifier_regex,
                                     modifier_regex,
                                     L"one modifier")),
              name_set_,
              false);
          }

          a.context ().set ("aname", an);
          a.context ().set ("mname", mn);

          name_set_.insert (b);

          if (an != b)
            name_set_.insert (an);

          if (mn != b && mn != an)
            name_set_.insert (mn);

          // Types
          //
          if (max != 1)
          {
            a.context ().set (
              "container",
              find_name (
                escape (process_regex (s + L",sequence", type_regex, L"type")),
                name_set_));

            a.context ().set (
              "iterator",
              find_name (
                escape (process_regex (s + L",iterator", type_regex, L"type")),
                name_set_));

            a.context ().set (
              "const-iterator",
              find_name (
                escape (
                  process_regex (s + L",const,iterator", type_regex, L"type")),
                name_set_));
          }
          else if (min == 0)
          {
            a.context ().set (
              "container",
              find_name (
                escape (process_regex (s + L",optional", type_regex, L"type")),
                name_set_));
          }

          // Data member.
          //
          a.context ().set ("member", find_name (b + L"_", name_set_));

          // Wildcard id.
          //
          if (ordered_p (c))
          {
            String id (
              escape (
                process_regex (
                  s + L",id", const_regex, L"const")));

            a.context ().set ("ordered-id-name", find_name (id, name_set_));
          }

          if (!has_wildcard_)
            has_wildcard_ = true;
        }

        virtual void
        traverse (SemanticGraph::AnyAttribute& a)
        {
          String s (find_name (L"any,attribute", stem_set_));

          String b (find_name (escape (s), name_set_, false));
          a.context ().set ("name", b);

          // Accessors/modifiers. Note that we postpone inserting the
          // names into the name_set to avoid over-escaping.
          //
          String an (
            find_name (
              escape (process_regex (s, accessor_regex, L"accessor")),
              name_set_,
              false));

          String mn (
            find_name (
              escape (process_regex (s, modifier_regex, L"modifier")),
              name_set_,
              false));

          a.context ().set ("aname", an);
          a.context ().set ("mname", mn);

          name_set_.insert (b);

          if (an != b)
            name_set_.insert (an);

          if (mn != b && mn != an)
            name_set_.insert (mn);

          // Types
          //
          a.context ().set (
            "container",
            find_name (
              escape (process_regex (s + L",set", type_regex, L"type")),
              name_set_));

          a.context ().set (
            "iterator",
            find_name (
              escape (process_regex (s + L",iterator", type_regex, L"type")),
              name_set_));

          a.context ().set (
            "const-iterator",
            find_name (
              escape (
                process_regex (s + L",const,iterator", type_regex, L"type")),
              name_set_));

          // Data member.
          //
          a.context ().set ("member", find_name (b + L"_", name_set_));

          if (!has_wildcard_)
            has_wildcard_ = true;
        }

      private:
        NameSet& name_set_;
        NameSet& stem_set_;
        bool& has_wildcard_;
      };

      //
      //
      struct Complex: Traversal::Complex, Context
      {
        Complex (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (Type& c)
        {
          SemanticGraph::Context& ctx (c.context ());

          // We leave this set around to allow other mappings to use
          // this information.
          //
          ctx.set ("cxx-tree-name-processor-stem-set", NameSet ());
          ctx.set ("cxx-tree-name-processor-member-set", NameSet ());

          // Use processed name.
          //
          String name (ctx.get<String> ("name"));

          // If renamed name is empty then we are not generating
          // anything for this type and name processing is not
          // required.
          //
          if (renamed_type (c, name) && !name)
            return;

          NameSet& stem_set (
            ctx.get<NameSet> ("cxx-tree-name-processor-stem-set"));

          NameSet& member_set (
            ctx.get<NameSet> ("cxx-tree-name-processor-member-set"));

          stem_set.insert (c.name ());
          member_set.insert (name);

          // Add our base's stems and members to the initial list.
          //
          if (c.inherits_p ())
          {
            // @@ What if this types name is the same as one of base's
            //    members?
            //
            SemanticGraph::Type& base (c.inherits ().base ());

            if (base.is_a<SemanticGraph::Complex> () &&
                !base.is_a<SemanticGraph::Enumeration> ())
            {
              if (!base.context ().count (
                    "cxx-tree-name-processor-member-set"))
              {
                dispatch (base);
              }

              NameSet const& base_stem_set (
                base.context ().get<NameSet> (
                  "cxx-tree-name-processor-stem-set"));

              stem_set.insert (base_stem_set.begin (), base_stem_set.end ());

              NameSet const& base_member_set (
                base.context ().get<NameSet> (
                  "cxx-tree-name-processor-member-set"));

              member_set.insert (base_member_set.begin (),
                                 base_member_set.end ());
            }
          }

          // First assign the "primary" names.
          //
          {
            PrimaryMember member (*this, member_set, stem_set);
            Traversal::Names names (member);

            Complex::names (c, names);
          }

          // Derived names for members.
          //
          {
            DerivedMember member (*this, member_set);
            Traversal::Names names (member);

            Complex::names (c, names);
          }

          // Names for the mixed content.
          //
          if (mixed_p (c))
          {
            // Check if we already have the mixed content down inheritance
            // hierarchy.
            //
            using SemanticGraph::Complex;

            for (Complex* p (&c); p->inherits_p ();)
            {
              if (Complex* b = dynamic_cast<Complex*> (
                    &p->inherits ().base ()))
              {
                if (mixed_p (*b))
                {
                  SemanticGraph::Context& bctx (b->context ());
                  ctx.set ("mixed-type", bctx.get<String> ("mixed-type"));
                  ctx.set ("mixed-const-iterator",
                           bctx.get<String> ("mixed-const-iterator"));
                  ctx.set ("mixed-ordered-id-name",
                           bctx.get<String> ("mixed-ordered-id-name"));
                  ctx.set ("mixed-aname", bctx.get<String> ("mixed-aname"));
                  ctx.set ("mixed-member", bctx.get<String> ("mixed-member"));
                  ctx.set ("mixed-in-base", true);
                  break;
                }

                p = b;
              }
              else
                break;
            }

            // If not, set up the names.
            //
            if (!ctx.count ("mixed-in-base"))
            {
              String s (find_name (L"text,content", stem_set));
              String n (find_name (escape (s), member_set, false));

              String an (find_name (
                           escape (process_regex (s,
                                                  seq_accessor_regex,
                                                  accessor_regex,
                                                  L"sequence accessor")),
                           member_set,
                           false));

              String mn (find_name (
                           escape (process_regex (s,
                                                  seq_modifier_regex,
                                                  modifier_regex,
                                                  L"sequence modifier")),
                           member_set,
                           false));

              ctx.set ("mixed-aname", an);
              ctx.set ("mixed-mname", mn);

              member_set.insert (name);

              if (an != n)
                member_set.insert (an);

              if (mn != n && mn != an)
                member_set.insert (mn);

              // Types.
              //
              ctx.set (
                "mixed-type",
                find_name (
                  escape (process_regex (s + L",type", type_regex, L"type")),
                  member_set));

              ctx.set (
                "mixed-container",
                find_name (
                  escape (process_regex (s + L",sequence", type_regex, L"type")),
                  member_set));

              ctx.set (
                "mixed-iterator",
                find_name (
                  escape (process_regex (s + L",iterator", type_regex, L"type")),
                  member_set));

              ctx.set (
                "mixed-const-iterator",
                find_name (
                  escape (
                    process_regex (s + L",const,iterator", type_regex, L"type")),
                  member_set));

              // Text content id.
              //
              ctx.set (
                "mixed-ordered-id-name",
                find_name (
                  escape (
                    process_regex (s + L",id", const_regex, L"const")),
                  member_set));

              // Data member.
              //
              ctx.set ("mixed-member", find_name (n + L"_", member_set));
            }
          }

          // Names for wildcards.
          //
          if (options.generate_wildcard ())
          {
            bool has_wildcard (false);
            Any any (*this, member_set, stem_set, has_wildcard);
            Traversal::Names names (any);
            Complex::names (c, names);

            // Assign names for dom_document.
            //
            if (has_wildcard)
            {
              // Check if we already have dom_document down inheritance
              // hierarchy.
              //
              for (SemanticGraph::Complex* p (&c); p->inherits_p ();)
              {
                if (SemanticGraph::Complex* base =
                    dynamic_cast<SemanticGraph::Complex*> (
                      &p->inherits ().base ()))
                {
                  if (base->context ().count ("dom-document"))
                  {
                    c.context ().set (
                      "dom-document",
                      base->context ().get<String> ("dom-document"));
                    break;
                  }

                  p = base;
                }
                else
                  break;
              }

              // If not, set up the names.
              //
              if (!c.context ().count ("dom-document"))
              {
                String stem (find_name (L"dom,document", stem_set));

                String an (
                  escape (
                    process_regex (stem, accessor_regex, L"accessor")));

                c.context ().set ("dom-document", find_name (an, member_set));

                c.context ().set (
                  "dom-document-member",
                  find_name (escape (stem + L"_"), member_set));
              }
            }
          }

          // Names for the order container.
          //
          if (ordered_p (c))
          {
            // Check if we already have the order container down
            // inheritance hierarchy.
            //
            using SemanticGraph::Complex;

            for (Complex* p (&c); p->inherits_p ();)
            {
              if (Complex* b = dynamic_cast<Complex*> (
                    &p->inherits ().base ()))
              {
                if (ordered_p (*b))
                {
                  SemanticGraph::Context& bctx (b->context ());
                  ctx.set ("order-type", bctx.get<String> ("order-type"));
                  ctx.set ("order-const-iterator",
                           bctx.get<String> ("order-const-iterator"));
                  ctx.set ("order-aname", bctx.get<String> ("order-aname"));
                  ctx.set ("order-member", bctx.get<String> ("order-member"));
                  ctx.set ("order-in-base", true);
                  break;
                }

                p = b;
              }
              else
                break;
            }

            // If not, set up the names.
            //
            if (!ctx.count ("order-in-base"))
            {
              String s (find_name (L"content,order", stem_set));
              String n (find_name (escape (s), member_set, false));

              String an (find_name (
                           escape (process_regex (s,
                                                  seq_accessor_regex,
                                                  accessor_regex,
                                                  L"sequence accessor")),
                           member_set,
                           false));

              String mn (find_name (
                           escape (process_regex (s,
                                                  seq_modifier_regex,
                                                  modifier_regex,
                                                  L"sequence modifier")),
                           member_set,
                           false));

              ctx.set ("order-aname", an);
              ctx.set ("order-mname", mn);

              member_set.insert (name);

              if (an != n)
                member_set.insert (an);

              if (mn != n && mn != an)
                member_set.insert (mn);

              // Types.
              //
              ctx.set (
                "order-type",
                find_name (
                  escape (process_regex (s + L",type", type_regex, L"type")),
                  member_set));

              ctx.set (
                "order-container",
                find_name (
                  escape (process_regex (s + L",sequence", type_regex, L"type")),
                  member_set));

              ctx.set (
                "order-iterator",
                find_name (
                  escape (process_regex (s + L",iterator", type_regex, L"type")),
                  member_set));

              ctx.set (
                "order-const-iterator",
                find_name (
                  escape (
                    process_regex (s + L",const,iterator", type_regex, L"type")),
                  member_set));

              // Data member.
              //
              ctx.set ("order-member", find_name (n + L"_", member_set));
            }
          }
        }
      };


      //
      //
      struct GlobalType: Traversal::Type, Context
      {
        GlobalType (Context& c, NameSet& set)
            : Context (c), set_ (set)
        {
        }

        virtual void
        traverse (SemanticGraph::Type& t)
        {
          // Process the name with type name regex.
          //
          String name (process_regex (
                         namespace_ (t).name (),
                         t.name (),
                         type_regex,
                         L"type"));

          // Escape and unclash.
          //
          name = find_name (escape (name), set_);
          t.context ().set ("name", name);

          // Also add renamed name if any.
          //
          if (renamed_type (t, name) && name)
            set_.insert (name);
        }

      private:
        NameSet& set_;
      };


      //
      //
      struct GlobalElement: Traversal::Element,
                            GlobalElementBase,
                            Context
      {
        GlobalElement (Context& c,
                       NameSet const& type_set,
                       NameSet& element_set)
            : GlobalElementBase (c),
              Context (c),
              type_set_ (type_set),
              element_set_ (element_set)
        {
        }

        virtual void
        traverse (Type& e)
        {
          // First we need to figure out if we need to process this
          // global element.
          //
          if (!generate_p (e))
            return;

          if (options.generate_element_type ())
          {
            SemanticGraph::Context& ec (e.context ());

            String name;

            if (doc_root_p (e))
            {
              name = find_name (
                escape (
                  process_regex (
                    namespace_ (e).name (),
                    e.name (),
                    element_type_regex,
                    type_regex,
                    L"element type")));

              // Assign inner names.
              //
              NameSet set;
              set.insert (name);

              ec.set (
                "type",
                Context::find_name (
                  escape (process_regex (L"value,type", type_regex, L"type")),
                  set));

              ec.set (
                "traits",
                Context::find_name (
                  escape (process_regex (L"value,traits", type_regex, L"type")),
                  set));

              String an (Context::find_name (
                escape (process_regex ("value",
                                       one_accessor_regex,
                                       accessor_regex,
                                       L"one accessor")),
                set,
                false));

              String mn (Context::find_name (
                escape (process_regex ("value",
                                       one_modifier_regex,
                                       modifier_regex,
                                       L"one modifier")),
                set,
                false));

              ec.set ("aname", an);
              ec.set ("mname", mn);

              set.insert (an);

              if (an != mn)
                set.insert (mn);

              // Detach.
              //
              if (detach)
              {
                String dn (Context::find_name (
                             escape (process_regex (L"detach,value",
                                                    one_modifier_regex,
                                                    modifier_regex,
                                                    L"one modifier")),
                             set));

                ec.set ("dname", dn);
              }

              // Assign name() and namespace_() names.
              //
              ec.set (
                "element-name",
                Context::find_name (
                  escape (
                    process_regex ("name", accessor_regex, L"modifier")),
                  set));

              ec.set (
                "element-ns",
                Context::find_name (
                  escape (
                    process_regex ("namespace", accessor_regex, L"modifier")),
                  set));

              // Data members.
              //
              ec.set ("member", Context::find_name ("value_", set));
              ec.set ("element-name-member",
                      Context::find_name ("name_", set));
              ec.set ("element-ns-member",
                      Context::find_name ("namespace__", set));
            }
            else
              name = find_name (escape (e.name ()));

            ec.set ("name", name);
            element_set_.insert (name);
          }
          else
          {
            // Make sure the name is unique among global elements and
            // does not collide with a global type name.
            //
            String base (find_name (escape (e.name ())));
            e.context ().set ("name", base);

            String n (e.name ());

            // Assign the parsing function name.
            //
            String p;

            if (!options.suppress_parsing () && doc_root_p (e))
            {
              p = find_name (
                escape (
                  process_regex (n, parser_regex, L"parsing function")));

              e.context ().set ("parser", p);
            }

            // Assign the serialization function name.
            //
            String s;

            if (options.generate_serialization () && doc_root_p (e))
            {
              s = find_name (
                escape (
                  process_regex (
                    n, serializer_regex, L"serialization function")));

              e.context ().set ("serializer", s);
            }

            // Add the names to the set only after processing parsing and
            // serialization function names so that we do not over-escape
            // them.
            //
            element_set_.insert (base);

            if (p && p != base)
              element_set_.insert (p);

            if (s && s != base && s != p)
              element_set_.insert (s);
          }
        }

      private:
        String
        find_name (String const& name)
        {
          String r (name);

          // If we are conflicting with a type name let's first try to
          // simply append an underscore and only resort to ugly names
          // like name1, etc., if this fails.
          //
          if (type_set_.find (r) != type_set_.end ())
            r += L"_";

          for (size_t i (1);
               element_set_.find (r) != element_set_.end () ||
                 type_set_.find (r) != type_set_.end (); ++i)
          {
            std::wostringstream os;
            os << i;
            r = name + os.str ();
          }

          return r;
        }

      private:
        NameSet const& type_set_;
        NameSet& element_set_;
      };

      struct NamespacePassOne: Traversal::Namespace, Context
      {
        NamespacePassOne (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (Type& ns)
        {
          NameSet& type_set (global_type_names[ns.name ()]);

          GlobalType type (*this, type_set);
          Traversal::Names names (type);

          Traversal::Namespace::names (ns, names);
          Traversal::Namespace::names (ns);
        }
      };


      struct NamespacePassThree: Traversal::Namespace, Context
      {
        NamespacePassThree (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (Type& ns)
        {
          String const& name (ns.name ());

          NameSet const& type_set (global_type_names[name]);
          NameSet& element_set (global_element_names[name]);

          GlobalElement element (*this, type_set, element_set);
          Traversal::Names names (element);

          Traversal::Namespace::names (ns, names);
        }
      };


      struct FundamentalNamespace: Traversal::Namespace,

                                   Traversal::AnyType,
                                   Traversal::AnySimpleType,

                                   Traversal::Fundamental::Byte,
                                   Traversal::Fundamental::UnsignedByte,
                                   Traversal::Fundamental::Short,
                                   Traversal::Fundamental::UnsignedShort,
                                   Traversal::Fundamental::Int,
                                   Traversal::Fundamental::UnsignedInt,
                                   Traversal::Fundamental::Long,
                                   Traversal::Fundamental::UnsignedLong,
                                   Traversal::Fundamental::Integer,
                                   Traversal::Fundamental::NonPositiveInteger,
                                   Traversal::Fundamental::NonNegativeInteger,
                                   Traversal::Fundamental::PositiveInteger,
                                   Traversal::Fundamental::NegativeInteger,

                                   Traversal::Fundamental::Boolean,

                                   Traversal::Fundamental::Float,
                                   Traversal::Fundamental::Double,
                                   Traversal::Fundamental::Decimal,

                                   Traversal::Fundamental::String,
                                   Traversal::Fundamental::NormalizedString,
                                   Traversal::Fundamental::Token,
                                   Traversal::Fundamental::Name,
                                   Traversal::Fundamental::NameToken,
                                   Traversal::Fundamental::NameTokens,
                                   Traversal::Fundamental::NCName,
                                   Traversal::Fundamental::Language,

                                   Traversal::Fundamental::Id,
                                   Traversal::Fundamental::IdRef,
                                   Traversal::Fundamental::IdRefs,

                                   Traversal::Fundamental::AnyURI,

                                   Traversal::Fundamental::QName,

                                   Traversal::Fundamental::Base64Binary,
                                   Traversal::Fundamental::HexBinary,

                                   Traversal::Fundamental::Date,
                                   Traversal::Fundamental::DateTime,
                                   Traversal::Fundamental::Duration,
                                   Traversal::Fundamental::Day,
                                   Traversal::Fundamental::Month,
                                   Traversal::Fundamental::MonthDay,
                                   Traversal::Fundamental::Year,
                                   Traversal::Fundamental::YearMonth,
                                   Traversal::Fundamental::Time,

                                   Traversal::Fundamental::Entity,
                                   Traversal::Fundamental::Entities,

                                   Context
      {
        FundamentalNamespace (Context& c)
            : Context (c)
        {
          *this >> names_ >> *this;
        }

        void
        process_name (SemanticGraph::Type& t, String const& name)
        {
          String r (
            process_regex (
              namespace_ (t).name (), name, type_regex, L"type"));

          t.context ().set ("name", escape (r));
        }

        void
        process_name (SemanticGraph::Namespace& n,
                      String const& name,
                      char const* key)
        {
          String r (process_regex (name, type_regex, L"type"));
          n.context ().set (key, escape (r));
        }

        // anyType and anySimpleType
        //
        virtual void
        traverse (SemanticGraph::AnyType& t)
        {
          process_name (t, "type");
        }

        virtual void
        traverse (SemanticGraph::AnySimpleType& t)
        {
          process_name (t, "simple,type");
        }

        // Integrals.
        //
        virtual void
        traverse (SemanticGraph::Fundamental::Byte& t)
        {
          process_name (t, "byte");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::UnsignedByte& t)
        {
          process_name (t, "unsigned,byte");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Short& t)
        {
          process_name (t, "short");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::UnsignedShort& t)
        {
          process_name (t, "unsigned,short");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Int& t)
        {
          process_name (t, "int");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::UnsignedInt& t)
        {
          process_name (t, "unsigned,int");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Long& t)
        {
          process_name (t, "long");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::UnsignedLong& t)
        {
          process_name (t, "unsigned,long");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Integer& t)
        {
          process_name (t, "integer");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::NonPositiveInteger& t)
        {
          process_name (t, "non,positive,integer");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::NonNegativeInteger& t)
        {
          process_name (t, "non,negative,integer");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::PositiveInteger& t)
        {
          process_name (t, "positive,integer");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::NegativeInteger& t)
        {
          process_name (t, "negative,integer");
        }

        // Boolean.
        //
        virtual void
        traverse (SemanticGraph::Fundamental::Boolean& t)
        {
          process_name (t, "boolean");
        }

        // Floats.
        //
        virtual void
        traverse (SemanticGraph::Fundamental::Float& t)
        {
          process_name (t, "float");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Double& t)
        {
          process_name (t, "double");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Decimal& t)
        {
          process_name (t, "decimal");
        }

        // Strings.
        //
        virtual void
        traverse (SemanticGraph::Fundamental::String& t)
        {
          process_name (t, "string");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::NormalizedString& t)
        {
          process_name (t, "normalized,string");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Token& t)
        {
          process_name (t, "token");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::NameToken& t)
        {
          process_name (t, "nmtoken");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::NameTokens& t)
        {
          process_name (t, "nmtokens");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Name& t)
        {
          process_name (t, "name");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::NCName& t)
        {
          process_name (t, "ncname");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Language& t)
        {
          process_name (t, "language");
        }

        // ID/IDREF.
        //
        virtual void
        traverse (SemanticGraph::Fundamental::Id& t)
        {
          process_name (t, "id");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::IdRef& t)
        {
          process_name (t, "idref");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::IdRefs& t)
        {
          process_name (t, "idrefs");
        }


        // URI.
        //
        virtual void
        traverse (SemanticGraph::Fundamental::AnyURI& t)
        {
          process_name (t, "uri");
        }

        // Qualified name.
        //
        virtual void
        traverse (SemanticGraph::Fundamental::QName& t)
        {
          process_name (t, "qname");
        }

        // Binary.
        //
        virtual void
        traverse (SemanticGraph::Fundamental::Base64Binary& t)
        {
          process_name (t, "base64,binary");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::HexBinary& t)
        {
          process_name (t, "hex,binary");
        }


        // Date/time.
        //
        virtual void
        traverse (SemanticGraph::Fundamental::Date& t)
        {
          process_name (t, "date");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::DateTime& t)
        {
          process_name (t, "date,time");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Duration& t)
        {
          process_name (t, "duration");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Day& t)
        {
          process_name (t, "gday");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Month& t)
        {
          process_name (t, "gmonth");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::MonthDay& t)
        {
          process_name (t, "gmonth,day");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Year& t)
        {
          process_name (t, "gyear");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::YearMonth& t)
        {
          process_name (t, "gyear,month");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Time& t)
        {
          process_name (t, "time");
        }

        // Entity.
        //
        virtual void
        traverse (SemanticGraph::Fundamental::Entity& t)
        {
          process_name (t, "entity");
        }

        virtual void
        traverse (SemanticGraph::Fundamental::Entities& t)
        {
          process_name (t, "entities");
        }

        virtual void
        post (SemanticGraph::Namespace& n)
        {
          // Assign names to extra stuff in the XML Schema namespace.
          //
          process_name (n, "container", "container");
          process_name (n, "buffer", "buffer");
          process_name (n, "time,zone", "time-zone");

          process_name (n, "content,order", "content-order");

          if (options.generate_element_type ())
            process_name (n, "element,type", "element-type");

          if (options.generate_element_map ())
            process_name (n, "element,map", "element-map");

          if (options.generate_serialization ())
          {
            process_name (n, "namespace,info", "namespace-info");
            process_name (n, "namespace,infomap", "namespace-infomap");
            process_name (n, "list,stream", "list-stream");
            process_name (n, "as,double", "as-double");
            process_name (n, "as,decimal", "as-decimal");
            process_name (n, "facet", "facet");
          }

          if (!options.generate_insertion ().empty ())
          {
            process_name (n, "ostream", "ostream");
          }

          if (!options.generate_extraction ().empty ())
          {
            process_name (n, "istream", "istream");
          }

          process_name (n, "flags", "flags");
          process_name (n, "properties", "properties");

          NarrowString fn (options.function_naming ());

          if (fn == "knr")
            n.context ().set ("tree-node-key", String ("tree_node_key"));
          else
            n.context ().set ("tree-node-key", String ("treeNodeKey"));

          process_name (n, "exception", "exception");
          process_name (n, "parsing", "parsing");
          process_name (n, "expected,element", "expected-element");
          process_name (n, "unexpected,element", "unexpected-element");
          process_name (n, "expected,attribute", "expected-attribute");
          process_name (n, "unexpected,enumerator", "unexpected-enumerator");
          process_name (n, "expected,text,content", "expected-text-content");
          process_name (n, "no,type,info", "no-type-info");
          process_name (n, "no,element,info", "no-element-info");
          process_name (n, "not,derived", "not-derived");
          process_name (n, "duplicate,id", "duplicate-id");
          process_name (n, "serialization", "serialization");
          process_name (n, "no,namespace,mapping", "no-namespace-mapping");
          process_name (n, "no,prefix,mapping", "no-prefix-mapping");
          process_name (n, "xsi,already,in,use", "xsi-already-in-use");
          process_name (n, "bounds", "bounds");

          process_name (n, "severity", "severity");
          process_name (n, "error", "error");
          process_name (n, "diagnostics", "diagnostics");

          if (!options.suppress_parsing () ||
              options.generate_serialization ())
          {
            process_name (n, "error,handler", "error-handler");
          }

          Namespace::post (n);
        }

      private:
        Traversal::Names names_;
      };


      // Go into sourced/included/imported schemas while making sure
      // we don't process the same stuff more than once.
      //
      struct UsesPassOne: Traversal::Uses
      {
        virtual void
        traverse (Type& u)
        {
          SemanticGraph::Schema& s (u.schema ());

          if (!s.context ().count ("cxx-tree-name-processor-pass-1"))
          {
            s.context ().set ("cxx-tree-name-processor-pass-1", true);
            Traversal::Uses::traverse (u);
          }
        }
      };

      struct UsesPassThree: Traversal::Uses
      {
        virtual void
        traverse (Type& u)
        {
          SemanticGraph::Schema& s (u.schema ());

          if (!s.context ().count ("cxx-tree-name-processor-pass-3"))
          {
            s.context ().set ("cxx-tree-name-processor-pass-3", true);
            Traversal::Uses::traverse (u);
          }
        }
      };

      // Go into implied schemas while making sure we don't process
      // the same stuff more than once.
      //
      struct Implies: Traversal::Implies
      {
        virtual void
        traverse (SemanticGraph::Implies& i)
        {
          SemanticGraph::Schema& s (i.schema ());

          if (!s.context ().count ("cxx-tree-name-processor-seen"))
          {
            s.context ().set ("cxx-tree-name-processor-seen", true);
            Traversal::Implies::traverse (i);
          }
        }
      };

      bool
      process_impl (options const& ops,
                    SemanticGraph::Schema& tu,
                    SemanticGraph::Path const& file,
                    StringLiteralMap const& map)
      {
        try
        {
          Counts counts;
          Context ctx (ops, counts, false, tu, file, map);

          if (tu.names_begin ()->named ().name () ==
              L"http://www.w3.org/2001/XMLSchema")
          {
            // XML Schema namespace.
            //
            Traversal::Schema xs_schema;
            Traversal::Names xs_schema_names;
            FundamentalNamespace xs_ns (ctx);

            xs_schema >> xs_schema_names >> xs_ns;

            xs_schema.dispatch (tu);
          }
          else
          {

            // Pass one - assign names to global types. This pass cannot
            // be combined with pass two because of possible recursive
            // schema inclusions. Also note that we check first if this
            // schema has already been processed which may happen in the
            // file-per-type compilation mode.
            //
            if (!tu.context ().count ("cxx-tree-name-processor-pass-1"))
            {
              Traversal::Schema schema;
              Traversal::Schema xs_schema;
              UsesPassOne uses;
              Implies implies;

              schema >> uses >> schema;
              schema >> implies >> xs_schema;

              Traversal::Names schema_names;
              Traversal::Names xs_schema_names;
              NamespacePassOne ns (ctx);
              FundamentalNamespace xs_ns (ctx);

              schema >> schema_names >> ns;
              xs_schema >> xs_schema_names >> xs_ns;

              // Some twisted schemas do recusive self-inclusion.
              //
              tu.context ().set ("cxx-tree-name-processor-pass-1", true);

              schema.dispatch (tu);
            }

            // Pass two - assign names inside complex types. Here
            // we don't need to go into included/imported schemas.
            //
            {
              Traversal::Schema schema;
              Sources sources;

              schema >> sources >> schema;

              Traversal::Names schema_names;
              Traversal::Namespace ns;
              Traversal::Names ns_names;

              schema >> schema_names >> ns >> ns_names;

              Complex complex (ctx);
              Traversal::Enumeration enumeration; // Avoid fallback on complex.

              ns_names >> complex;
              ns_names >> enumeration;

              schema.dispatch (tu);
            }

            // Pass three - assign names to global elements as well as
            // inside enums. Also note that we check first if this schema
            // has already been processed which may happen in the file-per-
            // type compilation mode.
            //
            if (!tu.context ().count ("cxx-tree-name-processor-pass-3"))
            {
              Traversal::Schema schema;
              UsesPassThree uses;

              schema >> uses >> schema;

              Traversal::Names schema_names;
              NamespacePassThree ns (ctx);
              Traversal::Namespace ns_enum;

              schema >> schema_names;

              schema_names >> ns;
              schema_names >> ns_enum;

              Traversal::Names ns_names;
              Enumeration enumeration (ctx);

              ns_enum >> ns_names >> enumeration;

              // Some twisted schemas do recusive self-inclusion.
              //
              tu.context ().set ("cxx-tree-name-processor-pass-3", true);

              schema.dispatch (tu);
            }
          }
        }
        catch (Context::Failed const&)
        {
          // Diagnostics has already been issued.
          //
          return false;
        }

        return true;
      }
    }

    bool NameProcessor::
    process (options const& ops,
             SemanticGraph::Schema& tu,
             SemanticGraph::Path const& file,
             StringLiteralMap const& map)
    {
      return process_impl (ops, tu, file, map);
    }
  }
}
