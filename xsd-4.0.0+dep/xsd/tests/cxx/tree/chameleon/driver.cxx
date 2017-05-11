// file      : tests/cxx/tree/chameleon/driver.cxx
// copyright : Copyright (c) 2006-2014 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

// Test chameleon inclusion.
//

#include <memory> // std::auto_ptr/unique_ptr
#include <iostream>

#include "includer.hxx"

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
    XSD_AUTO_PTR<root_t> r (root (argv[1]));

    cout << *r << endl;
  }
  catch (xml_schema::exception const& e)
  {
    cerr << e << endl;
    return 1;
  }
}
