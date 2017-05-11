// file      : tests/cxx/tree/enumeration/inheritance/driver.cxx
// copyright : Copyright (c) 2006-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

// Insert test description here.
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
    XSD_AUTO_PTR<top_bottom> r (root (argv[1]));

    switch (*r)
    {
    case top_bottom::top:
      {
        cout << "top" << endl;
        break;
      }
    case top_bottom::bottom:
      {
        cout << "bottom" << endl;
        break;
      }
    default: // Suppress warning.
      {
        assert (false);
        break;
      }
    }
  }
  catch (xml_schema::exception const& e)
  {
    cerr << e << endl;
    return 1;
  }
}
