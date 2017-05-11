// file      : xsd/cxx/elements.cxx
// copyright : Copyright (c) 2005-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cxx/elements.hxx>

#include <cctype>    // std::toupper
#include <memory>
#include <sstream>
#include <fstream>
#include <cassert>
#include <iostream>

using std::wcerr;
using std::endl;

namespace CXX
{
  //
  //
  wchar_t
  upcase (wchar_t c)
  {
    return std::toupper (c);
  }

  namespace
  {
    wchar_t const* keywords[] = {
      L"NULL",
      L"and",
      L"asm",
      L"auto",
      L"bitand",
      L"bitor",
      L"bool",
      L"break",
      L"case",
      L"catch",
      L"char",
      L"class",
      L"compl",
      L"const",
      L"const_cast",
      L"continue",
      L"default",
      L"delete",
      L"do",
      L"double",
      L"dynamic_cast",
      L"else",
      L"end_eq",
      L"enum",
      L"explicit",
      L"export",
      L"extern",
      L"false",
      L"float",
      L"for",
      L"friend",
      L"goto",
      L"if",
      L"inline",
      L"int",
      L"long",
      L"mutable",
      L"namespace",
      L"new",
      L"not",
      L"not_eq",
      L"operator",
      L"or",
      L"or_eq",
      L"private",
      L"protected",
      L"public",
      L"register",
      L"reinterpret_cast",
      L"return",
      L"short",
      L"signed",
      L"sizeof",
      L"static",
      L"static_cast",
      L"struct",
      L"switch",
      L"template",
      L"this",
      L"throw",
      L"true",
      L"try",
      L"typedef",
      L"typeid",
      L"typename",
      L"union",
      L"unsigned",
      L"using",
      L"virtual",
      L"void",
      L"volatile",
      L"wchar_t",
      L"while",
      L"xor",
      L"xor_eq"
    };
  }

  // Context
  //

  Context::
  Context (std::wostream& o,
           SemanticGraph::Schema& root,
           SemanticGraph::Path const& path,
           options_type const& ops,
           StringLiteralMap const* string_literal_map_)
      : os (o),
        schema_root (root),
        schema_path (schema_path_),
        options (ops),
        std (ops.std ()),
        char_type (char_type_),
        char_encoding (char_encoding_),
        L (L_),
        string_type (string_type_),
        auto_ptr (auto_ptr_),
        string_literal_map (string_literal_map_),
        type_exp (type_exp_),
        inst_exp (inst_exp_),
        inl (inl_),
        ns_mapping_cache (ns_mapping_cache_),
        schema_path_ (path),
        xs_ns_ (0),
        char_type_ (ops.char_type ()),
        char_encoding_ (ops.char_encoding ()),
        L_ (char_type == L"wchar_t" ? L"L" : L""),
        inl_ (ops.generate_inline () ? L"inline\n" : L""),
        cxx_id_expr_ (L"^(::)?([a-zA-Z_]\\w*)(::[a-zA-Z_]\\w*)*$"),
        cxx_id_expr (cxx_id_expr_),
        urn_mapping_ (L"#^urn.*:([a-zA-Z_].*)$#$1#"),
        urn_mapping (urn_mapping_),
        nsr_mapping (nsr_mapping_),
        nsm_mapping (nsm_mapping_),
        include_mapping (include_mapping_),
        reserved_name_map (reserved_name_map_),
        keyword_set (keyword_set_)
  {
    // Export symbol.
    //
    {
      String es (ops.export_symbol ());
      type_exp_ = es ? es + L" " : es;
      inst_exp_ = es ? es + L"\n" : es;
    }

    // Resolve and cache XML Schema namespace.
    //
    {
      SemanticGraph::Nameable* n;

      if (schema_root.names_begin ()->name () ==
          L"http://www.w3.org/2001/XMLSchema")
      {
        // schema_root is the XML Schema itself.
        //
        n = &schema_root.names_begin ()->named ();
      }
      else
      {
        // Otherwise, the first used schema is implied XML Schema.
        //
        SemanticGraph::Uses& u = *schema_root.uses_begin ();
        assert (u.is_a<SemanticGraph::Implies> ());
        n = &u.schema ().names_begin ()->named ();
      }

      xs_ns_ = dynamic_cast<SemanticGraph::Namespace*> (n);
    }

    // String type.
    //
    if (char_type == L"char")
      string_type_ = L"::std::string";
    else if (char_type == L"wchar_t")
      string_type_ = L"::std::wstring";
    else
      string_type_ = L"::std::basic_string< " + char_type + L" >";

    // Automatic pointer type.
    //
    auto_ptr_ = std >= cxx_version::cxx11
      ? "::std::unique_ptr"
      : "::std::auto_ptr";

    // Default encoding.
    //
    if (!char_encoding)
    {
      if (char_type == L"char")
        char_encoding = L"utf8";
      else
        char_encoding = L"auto";
    }

    // Default mapping.
    //
    nsr_mapping_.push_back (
      Regex (L"#^.* (.*?/)??" L"(([a-zA-Z_]\\w*)(/[a-zA-Z_]\\w*)*)/?$#$2#"));
    nsr_mapping_.push_back (
      Regex (L"#^.* http://www\\.w3\\.org/2001/XMLSchema$#xml_schema#"));

    // Custom regex mapping.
    //
    for (NarrowStrings::const_iterator i (ops.namespace_regex ().begin ()),
           e (ops.namespace_regex ().end ()); i != e; ++i)
    {
      nsr_mapping_.push_back (Regex (String (*i)));
    }

    // Custom direct mapping.
    //
    for (NarrowStrings::const_iterator i (ops.namespace_map ().begin ()),
           e (ops.namespace_map ().end ()); i != e; ++i)
    {
      String s (*i);

      // Split the string in two parts at the last '='.
      //
      size_t pos (s.rfind ('='));

      if (pos == String::npos)
        throw InvalidNamespaceMapping (s, "delimiter ('=') not found");

      // Empty xml_ns designates the no-namespace case.
      //
      String xml_ns (s, 0, pos);
      String cxx_ns (s, pos + 1);

      if (!cxx_ns.empty () && !cxx_id_expr.match (cxx_ns))
        throw InvalidNamespaceMapping (s, "invalid C++ identifier");

      nsm_mapping_[xml_ns] = cxx_ns;
    }

    // Include path regex
    //
    for (NarrowStrings::const_iterator i (ops.include_regex ().begin ()),
           e (ops.include_regex ().end ()); i != e; ++i)
    {
      include_mapping_.push_back (Regex (String (*i)));
    }

    // Reserved names.
    //
    for (NarrowStrings::const_iterator i (ops.reserved_name ().begin ()),
           e (ops.reserved_name ().end ()); i != e; ++i)
    {
      String s (*i);

      // Split the string in two parts at '='.
      //
      size_t pos (s.find ('='));

      if (pos == String::npos)
        reserved_name_map_[s] = L"";
      else
        reserved_name_map_[String (s, 0, pos)] = String (s, pos + 1);
    }

    // Populate the keyword set.
    //
    for (size_t i (0); i < sizeof (keywords) / sizeof (char*); ++i)
      keyword_set_.insert (keywords[i]);
  }

  String Context::
  ns_name (SemanticGraph::Namespace& ns)
  {
    using SemanticGraph::Schema;
    using SemanticGraph::Includes;
    using SemanticGraph::Imports;
    using SemanticGraph::Implies;
    using SemanticGraph::Sources;

    String tmp;
    MapMapping::const_iterator i (nsm_mapping.find (ns.name ()));

    if (i != nsm_mapping.end ())
    {
      tmp = i->second;
    }
    else
    {
      SemanticGraph::Path path;
      Schema& schema (dynamic_cast<Schema&> (ns.scope ()));

      if (schema.used_p ())
      {
        // Here we need to detect a special multi-schema compilation
        // case where the root schemas are imported into a special
        // schema that doesn't have a namespace.
        //
        SemanticGraph::Uses& u (*schema.used_begin ());
        SemanticGraph::Schema& s (u.user ());

        if (s.names_begin () != s.names_end ())
          path = u.path ();
      }
      else
        path = schema_path;

      String pair;

      if (!path.empty ())
      {
        path.normalize ();

        // Try to use the portable representation of the path. If that
        // fails, fall back to the native representation.
        //
        try
        {
          pair = path.posix_string ();
        }
        catch (SemanticGraph::InvalidPath const&)
        {
          pair = path.string ();
        }
      }

      pair += L' ' + ns.name ();

      // Check cache first
      //
      MappingCache::const_iterator i (ns_mapping_cache.find (pair));

      if (i != ns_mapping_cache.end ())
      {
        tmp = i->second;
      }
      else
      {
        bool trace (options.namespace_regex_trace ());

        if (trace)
          wcerr << "namespace: '" << pair << "'" << endl;

        bool found (false);
        Regex colon (L"#/#::#");

        for (RegexMapping::const_reverse_iterator e (nsr_mapping.rbegin ());
             e != nsr_mapping.rend (); ++e)
        {
          if (trace)
            wcerr << "try: '" << e->regex () << "' : ";

          if (e->match (pair))
          {
            tmp = e->replace (pair);
            tmp = colon.replace (tmp); // replace `/' with `::'

            // Check the result.
            //
            found = cxx_id_expr.match (tmp);

            if (trace)
              wcerr << "'" << tmp << "' : ";
          }

          if (trace)
            wcerr << (found ? '+' : '-') << endl;

          if (found)
            break;
        }

        if (!found)
        {
          String const& n (ns.name ());

          // Check if the name is valid by itself.
          //
          if (n.empty ())
          {
            // Empty name denotes a no-namespace case.
            //
            tmp = n;
          }
          else
          {
            tmp = colon.replace (n); // replace `/' with `::'

            if (!cxx_id_expr.match (tmp))
            {
              // See if this is a urn-style namespace.
              //
              if (urn_mapping.match (n))
              {
                Regex filter (L"#[.:-]#_#");
                tmp = urn_mapping.replace (n);
                tmp = filter.replace (tmp);

                if (!cxx_id_expr.match (tmp))
                  throw NoNamespaceMapping (
                    ns.file (), ns.line (), ns.column (), ns.name ());
              }
              else
                throw NoNamespaceMapping (
                  ns.file (), ns.line (), ns.column (), ns.name ());
            }
          }
        }

        // Add the mapping to the cache.
        //
        ns_mapping_cache[pair] = tmp;
      }
    }


    // Parse resulting namespace string and id() each name.
    //
    String r;
    String::size_type b (0), e;

    do
    {
      e = tmp.find (L"::", b);

      String name (tmp, b, e == tmp.npos ? e : e - b);

      if (!name.empty ())
        r += L"::" + escape (name);

      b = e;

      if (b == tmp.npos)
        break;

      b += 2;

    } while (true);

    return r;
  }

  SemanticGraph::Namespace& Context::
  xs_ns ()
  {
    return *xs_ns_;
  }

  String Context::
  xs_ns_name ()
  {
    return ns_name (*xs_ns_);
  }

  SemanticGraph::Namespace& Context::
  namespace_ (SemanticGraph::Nameable& n)
  {
    // The basic idea goes like this: go up Names edges until you
    // reach Namespace. There are, however, anonymous types which
    // need special handling. In the case of an anonymous type we
    // will go up the first Belongs edge (because the first edge
    // is where the type was defined.
    //

    if (n.named_p ())
    {
      SemanticGraph::Scope& s (n.scope ());

      SemanticGraph::Namespace* ns (
        dynamic_cast<SemanticGraph::Namespace*> (&n));

      return ns ? *ns : namespace_ (s);
    }
    else
    {
      SemanticGraph::Type& t (dynamic_cast<SemanticGraph::Type&> (n));

      SemanticGraph::Belongs& b (*t.classifies_begin ());

      return namespace_ (b.instance ());
    }
  }

  String Context::
  xml_ns_name (SemanticGraph::Nameable& n)
  {
    return namespace_ (n).name ();
  }

  String Context::
  fq_name (SemanticGraph::Nameable& n, char const* name_key)
  {
    using namespace SemanticGraph;

    String r;

    if (dynamic_cast<Schema*> (&n))
    {
      return L""; // Map to global namespace.
    }
    else if (SemanticGraph::Namespace* ns =
             dynamic_cast<SemanticGraph::Namespace*> (&n))
    {
      r = ns_name (*ns);
    }
    else
    {
      r = fq_name (n.scope ());
      r += L"::";
      r += n.context ().get<String> (name_key);
    }

    return r;
  }

  SemanticGraph::Type& Context::
  ultimate_base (SemanticGraph::Complex& c)
  {
    using namespace SemanticGraph;

    Type* b (&c.inherits ().base ());

    while (true)
    {
      Complex* cb (dynamic_cast<Complex*> (b));

      if (cb != 0 && cb->inherits_p ())
      {
        b = &cb->inherits ().base ();
        continue;
      }

      break;
    }

    return *b;
  }

  String Context::
  escape (String const& name)
  {
    String r;
    size_t n (name.size ());

    // In most common cases we will have that many chars.
    //
    r.reserve (n);

    for (size_t i (0); i < n; ++i)
    {
      bool first (i == 0);

      unsigned int u (unicode_char (name, i)); // May advance i.

      if (first)
      {
        if (!((u >= 'a' && u <= 'z') ||
              (u >= 'A' && u <= 'Z') ||
              u == '_'))
          r = (u >= '0' && u <= '9') ? L"cxx_" : L"cxx";
      }

      if (!((u >= 'a' && u <= 'z') ||
            (u >= 'A' && u <= 'Z') ||
            (u >= '0' && u <= '9') ||
            u == '_'))
        r.push_back ('_');
      else
        r.push_back (static_cast<wchar_t> (u));
    }

    if (r.empty ())
      r = L"cxx";

    // Custom reserved words.
    //
    ReservedNameMap::const_iterator i (reserved_name_map.find (r));

    if (i != reserved_name_map.end ())
    {
      if (i->second)
        return i->second;
      else
        r += L'_';
    }

    // Keywords
    //
    if (keyword_set.find (r) != keyword_set.end ())
    {
      r += L'_';

      // Re-run custom words.
      //
      i = reserved_name_map.find (r);

      if (i != reserved_name_map.end ())
      {
        if (i->second)
          return i->second;
        else
          r += L'_';
      }
    }

    return r;
  }

  // String escaping.
  //

  String
  charlit (unsigned int u)
  {
    String r ("\\x");
    bool lead (true);

    for (int i (7); i >= 0; --i)
    {
      unsigned int x ((u >> (i * 4)) & 0x0F);

      if (lead)
      {
        if (x == 0)
          continue;

        lead = false;
      }

      r += x < 10 ? ('0' + x) : ('A' + x - 10);
    }

    return r;
  }

  String
  strlit_ascii (String const& str)
  {
    String r;
    size_t n (str.size ());

    // In most common cases we will have that many chars.
    //
    r.reserve (n + 2);

    r += '"';

    bool escape (false);

    for (size_t i (0); i < n; ++i)
    {
      unsigned int u (Context::unicode_char (str, i)); // May advance i.

      // [128 - ]     - unrepresentable
      // 127          - \x7F
      // [32  - 126]  - as is
      // [0   - 31]   - \X or \xXX
      //

      if (u < 32 || u == 127)
      {
        switch (u)
        {
        case L'\n':
          {
            r += L"\\n";
            break;
          }
        case L'\t':
          {
            r += L"\\t";
            break;
          }
        case L'\v':
          {
            r += L"\\v";
            break;
          }
        case L'\b':
          {
            r += L"\\b";
            break;
          }
        case L'\r':
          {
            r += L"\\r";
            break;
          }
        case L'\f':
          {
            r += L"\\f";
            break;
          }
        case L'\a':
          {
            r += L"\\a";
            break;
          }
        default:
          {
            r += charlit (u);
            escape = true;
            break;
          }
        }
      }
      else if (u < 127)
      {
        if (escape)
        {
          // Close and open the string so there are no clashes.
          //
          r += '"';
          r += '"';

          escape = false;
        }

        switch (u)
        {
        case L'"':
          {
            r += L"\\\"";
            break;
          }
        case L'\\':
          {
            r += L"\\\\";
            break;
          }
        default:
          {
            r += static_cast<wchar_t> (u);
            break;
          }
        }
      }
      else
      {
        // Unrepresentable character.
        //
        throw UnrepresentableCharacter (str, i + 1);
      }
    }

    r += '"';

    return r;
  }

  const unsigned int utf8_first_char_mask[5] =
  {
    0x00, 0x00, 0xC0, 0xE0, 0xF0
  };

  String
  strlit_utf8 (String const& str)
  {
    String r;
    size_t n (str.size ());

    // In most common cases we will have that many chars.
    //
    r.reserve (n + 2);

    r += '"';

    bool escape (false);

    for (size_t i (0); i < n; ++i)
    {
      unsigned int u (Context::unicode_char (str, i)); // May advance i.

      // [128 - ]     - UTF-8
      // 127          - \x7F
      // [32  - 126]  - as is
      // [0   - 31]   - \X or \xXX
      //

      if (u < 32 || u == 127)
      {
        switch (u)
        {
        case L'\n':
          {
            r += L"\\n";
            break;
          }
        case L'\t':
          {
            r += L"\\t";
            break;
          }
        case L'\v':
          {
            r += L"\\v";
            break;
          }
        case L'\b':
          {
            r += L"\\b";
            break;
          }
        case L'\r':
          {
            r += L"\\r";
            break;
          }
        case L'\f':
          {
            r += L"\\f";
            break;
          }
        case L'\a':
          {
            r += L"\\a";
            break;
          }
        default:
          {
            r += charlit (u);
            escape = true;
            break;
          }
        }
      }
      else if (u < 127)
      {
        if (escape)
        {
          // Close and open the string so there are no clashes.
          //
          r += '"';
          r += '"';

          escape = false;
        }

        switch (u)
        {
        case L'"':
          {
            r += L"\\\"";
            break;
          }
        case L'\\':
          {
            r += L"\\\\";
            break;
          }
        default:
          {
            r += static_cast<wchar_t> (u);
            break;
          }
        }
      }
      else
      {
        unsigned int count (0);
        unsigned int tmp[4];

        if (u < 0x800)
          count = 2;
        else if (u < 0x10000)
          count = 3;
        else if (u < 0x110000)
          count = 4;

        switch (count)
        {
        case 4:
          {
            tmp[3] = (u | 0x80UL) & 0xBFUL;
            u >>= 6;
          }
        case 3:
          {
            tmp[2] = (u | 0x80UL) & 0xBFUL;
            u >>= 6;
          }
        case 2:
          {
            tmp[1] = (u | 0x80UL) & 0xBFUL;
            u >>= 6;
          }
        case 1:
          {
            tmp[0] = u | utf8_first_char_mask[count];
            break;
          }
        default:
          assert (false);
        }

        for (unsigned int j (0); j < count; ++j)
          r += charlit (tmp[j]);

        escape = true;
      }
    }

    r += '"';

    return r;
  }

  String
  strlit_iso8859_1 (String const& str)
  {
    String r;
    size_t n (str.size ());

    // In most common cases we will have that many chars.
    //
    r.reserve (n + 2);

    r += '"';

    bool escape (false);

    for (size_t i (0); i < n; ++i)
    {
      unsigned int u (Context::unicode_char (str, i)); // May advance i.

      // [256 -    ]  - unrepresentable
      // [127 - 255]  - \xXX
      // [32  - 126]  - as is
      // [0   - 31]   - \X or \xXX
      //

      if (u < 32)
      {
        switch (u)
        {
        case L'\n':
          {
            r += L"\\n";
            break;
          }
        case L'\t':
          {
            r += L"\\t";
            break;
          }
        case L'\v':
          {
            r += L"\\v";
            break;
          }
        case L'\b':
          {
            r += L"\\b";
            break;
          }
        case L'\r':
          {
            r += L"\\r";
            break;
          }
        case L'\f':
          {
            r += L"\\f";
            break;
          }
        case L'\a':
          {
            r += L"\\a";
            break;
          }
        default:
          {
            r += charlit (u);
            escape = true;
            break;
          }
        }
      }
      else if (u < 127)
      {
        if (escape)
        {
          // Close and open the string so there are no clashes.
          //
          r += '"';
          r += '"';

          escape = false;
        }

        switch (u)
        {
        case L'"':
          {
            r += L"\\\"";
            break;
          }
        case L'\\':
          {
            r += L"\\\\";
            break;
          }
        default:
          {
            r += static_cast<wchar_t> (u);
            break;
          }
        }
      }
      else if (u < 256)
      {
        r += charlit (u);
        escape = true;
      }
      else
      {
        // Unrepresentable character.
        //
        throw UnrepresentableCharacter (str, i + 1);
      }
    }

    r += '"';

    return r;
  }

  String
  strlit_utf32 (String const& str)
  {
    String r;
    size_t n (str.size ());

    // In most common cases we will have that many chars.
    //
    r.reserve (n + 3);

    r += L"L\"";

    bool escape (false);

    for (size_t i (0); i < n; ++i)
    {
      unsigned int u (Context::unicode_char (str, i)); // May advance i.

      // [128 - ]     - \xUUUUUUUU
      // 127          - \x7F
      // [32  - 126]  - as is
      // [0   - 31]   - \X or \xXX
      //

      if (u < 32 || u == 127)
      {
        switch (u)
        {
        case L'\n':
          {
            r += L"\\n";
            break;
          }
        case L'\t':
          {
            r += L"\\t";
            break;
          }
        case L'\v':
          {
            r += L"\\v";
            break;
          }
        case L'\b':
          {
            r += L"\\b";
            break;
          }
        case L'\r':
          {
            r += L"\\r";
            break;
          }
        case L'\f':
          {
            r += L"\\f";
            break;
          }
        case L'\a':
          {
            r += L"\\a";
            break;
          }
        default:
          {
            r += charlit (u);
            escape = true;
            break;
          }
        }
      }
      else if (u < 127)
      {
        if (escape)
        {
          // Close and open the string so there are no clashes. C++11
          // requires a space between " and L.
          //
          r += L"\" L\"";
          escape = false;
        }

        switch (u)
        {
        case L'"':
          {
            r += L"\\\"";
            break;
          }
        case L'\\':
          {
            r += L"\\\\";
            break;
          }
        default:
          {
            r += static_cast<wchar_t> (u);
            break;
          }
        }
      }
      else
      {
        r += charlit (u);
        escape = true;
      }
    }

    r += '"';

    return r;
  }

  String Context::
  strlit (String const& str)
  {
    // First see if we have a custom mapping.
    //
    assert (string_literal_map != 0);
    StringLiteralMap::const_iterator i (string_literal_map->find (str));

    if (i != string_literal_map->end ())
      return i->second;

    if (char_type == L"char")
    {
      if (char_encoding == L"utf8")
        return strlit_utf8 (str);
      else if (char_encoding == L"iso8859-1")
        return strlit_iso8859_1 (str);
      else
      {
        // For LCP, custom, and other unknown encodings, use ASCII.
        //
        return strlit_ascii (str);
      }
    }
    else
      return strlit_utf32 (str);
  }

  String Context::
  comment (String const& str)
  {
    String r;

    wchar_t const* s (str.c_str ());
    size_t size (str.size ());

    // In most common cases we will have that many chars.
    //
    r.reserve (size);

    for (wchar_t const* p (s); p < s + size; ++p)
    {
      unsigned int u (unicode_char (p)); // May advance p.

      // We are going to treat \v, \f and \n as unrepresentable
      // here even though they can be present in C++ source code.
      //
      if (u > 127 || (u < 32 && u != '\t'))
        r += L'?';
      else
        r += static_cast<wchar_t> (u);
    }

    return r;
  }

  String Context::
  process_include_path (String const& name) const
  {
    String path (String (options.include_prefix ()) + name);
    bool trace (options.include_regex_trace ());

    if (trace)
      wcerr << "include: '" << path << "'" << endl;

    String r;
    bool found (false);

    for (RegexMapping::const_reverse_iterator e (include_mapping.rbegin ());
         e != include_mapping.rend (); ++e)
    {
      if (trace)
        wcerr << "try: '" << e->regex () << "' : ";

      if (e->match (path))
      {
        r = e->replace (path);
        found = true;

        if (trace)
          wcerr << "'" << r << "' : ";
      }

      if (trace)
        wcerr << (found ? '+' : '-') << endl;

      if (found)
        break;
    }

    if (!found)
      r = path;

    if (!r.empty () && r[0] != L'"' && r[0] != L'<')
    {
      wchar_t op (options.include_with_brackets () ? L'<' : L'"');
      wchar_t cl (options.include_with_brackets () ? L'>' : L'"');
      r = op + r + cl;
    }

    return r;
  }

  // Namespace
  //

  void Namespace::
  pre (Type& n)
  {
    String ns (ctx_.ns_name (n));

    String::size_type b (0), e;

    if (st_)
      st_->enter (n, L"", ns ? false : true);

    do
    {
      e = ns.find (L"::", b);

      String name (ns, b, e == ns.npos ? e : e - b);

      if (!name.empty ())
      {
        if (st_)
          st_->enter (n, name, e == ns.npos);

        ctx_.os << "namespace " << name << "{";
      }

      b = e;

      if (b == ns.npos)
        break;

      b += 2;

    } while (true);
  }

  void Namespace::
  post (Type& n)
  {
    String ns (ctx_.ns_name (n));

    String::size_type b (0), e;

    do
    {
      e = ns.find (L"::", b);

      String name (ns, b, e == ns.npos ? e : e - b);

      if (!name.empty ())
      {
        ctx_.os << "}";

        if (st_)
          st_->leave ();
      }


      b = e;

      if (b == ns.npos)
        break;

      b += 2;

    }
    while (true);

    if (st_)
      st_->leave ();
  }
}
