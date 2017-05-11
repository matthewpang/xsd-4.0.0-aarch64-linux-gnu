// file      : tests/cxx/tree/binary/cdr/driver.cxx
// copyright : Copyright (c) 2006-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

// Test non-polymorphic binary serialization to ACE CDR.
//

#include <memory> // std::auto_ptr/unique_ptr
#include <cassert>
#include <iostream>

#include "test.hxx"

using namespace std;
using namespace test;

int
main (int argc, char* argv[])
{
  if (argc != 2)
  {
    cerr << "usage: " << argv[0] << " test.xml" << endl;
    return 1;
  }

  try
  {
    XSD_AUTO_PTR<type> r (root (argv[1]));

    // Save to a CDR stream.
    //
    ACE_OutputCDR ace_ocdr;
    xml_schema::ostream<ACE_OutputCDR> ocdr (ace_ocdr);
    ocdr << *r;

    // Load from a CDR stream.
    //
    ACE_InputCDR ace_icdr (ace_ocdr);
    xml_schema::istream<ACE_InputCDR> icdr (ace_icdr);
    XSD_AUTO_PTR<type> c (new type (icdr));

    // Compare the two.
    //
    assert (r->list () == c->list ());
    assert (r->union_ () == c->union_ ());
    assert (r->enumeration () == c->enumeration ());

    type::complex_sequence rs (r->complex ()), cs (c->complex ());

    for (type::complex_iterator ri (rs.begin ()), ci (cs.begin ());
         ri != rs.end () && ci != rs.end (); ++ri, ++ci)
    {
      assert (ri->a () == ci->a ());
      if (ri->b ())
        assert (ri->b () == ci->b ());
      assert (ri->c () == ci->c ());

      assert (ri->x () == ci->x ());
      if (ri->y ())
        assert (ri->y () == ci->y ());
    }

    // integers
    //
    assert (r->byte () == c->byte ());
    assert (r->unsigned_byte () == c->unsigned_byte ());
    assert (r->short_ () == c->short_ ());
    assert (r->unsigned_short () == c->unsigned_short ());
    assert (r->int_ () == c->int_ ());
    assert (r->unsigned_int () == c->unsigned_int ());
    assert (r->long_ () == c->long_ ());
    assert (r->unsigned_long () == c->unsigned_long ());
    assert (r->integer () == c->integer ());
    assert (r->non_positive_integer () == c->non_positive_integer ());
    assert (r->non_negative_integer () == c->non_negative_integer ());
    assert (r->positive_integer () == c->positive_integer ());
    assert (r->negative_integer () == c->negative_integer ());

    // boolean
    //
    assert (r->boolean () == c->boolean ());

    // floats
    //
    assert (r->float_ () == c->float_ ());
    assert (r->double_ () == c->double_ ());
    assert (r->decimal () == c->decimal ());

    // strings
    //
    assert (r->string () == c->string ());
    assert (r->normalized_string () == c->normalized_string ());
    assert (r->token () == c->token ());
    assert (r->name () == c->name ());
    assert (r->name_token () == c->name_token ());
    assert (r->name_tokens () == c->name_tokens ());
    assert (r->ncname () == c->ncname ());
    assert (r->language () == c->language ());

    // qualified name
    //
    assert (r->qname () == c->qname ());

    // ID/IDREF
    //
    assert (r->id () == c->id ());
    assert (r->id_ref () == c->id_ref ());
    assert (r->id_refs () == c->id_refs ());

    // URI
    //
    assert (r->any_uri () == c->any_uri ());

    // binary
    //
    assert (r->base64_binary () == c->base64_binary ());
    assert (r->hex_binary () == c->hex_binary ());

    // date/time
    //
    assert (r->day () == c->day ());
    assert (r->month () == c->month ());
    assert (r->year () == c->year ());
    assert (r->month_day () == c->month_day ());
    assert (r->year_month () == c->year_month ());
    assert (r->date () == c->date ());
    assert (r->time () == c->time ());
    assert (r->date_time () == c->date_time ());
    assert (r->duration () == c->duration ());

    // anySimpleType
    //
    assert (!r->any_simple_type_attr ().text_content ().empty ());
    assert (r->any_simple_type_attr () == c->any_simple_type_attr ());

    assert (!r->any_simple_type ().text_content ().empty ());
    assert (r->any_simple_type () == c->any_simple_type ());
  }
  catch (xml_schema::exception const& e)
  {
    cerr << e << endl;
    return 1;
  }
}
