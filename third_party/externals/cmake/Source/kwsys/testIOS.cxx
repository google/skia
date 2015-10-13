/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(stl/vector)
#include KWSYS_HEADER(ios/sstream)
#include KWSYS_HEADER(ios/fstream)
#include KWSYS_HEADER(ios/iostream)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "kwsys_stl_string.hxx.in"
# include "kwsys_stl_vector.h.in"
# include "kwsys_ios_sstream.h.in"
# include "kwsys_ios_fstream.h.in"
# include "kwsys_ios_iostream.h.in"
#endif

#include <string.h> /* strlen */

int testIOS(int, char*[])
{
  kwsys_ios::ostringstream ostr;
  const char hello[] = "hello";
  ostr << hello;
  if(ostr.str() != hello)
    {
    kwsys_ios::cerr << "failed to write hello to ostr" << kwsys_ios::endl;
    return 1;
    }
  const char world[] = "world";
  kwsys_ios::ostringstream ostr2;
  ostr2.write( hello, strlen(hello) ); /* I could do sizeof */
  ostr2.put( '\0' );
  ostr2.write( world, strlen(world) );
  if(ostr2.str().size() !=  strlen(hello) + 1 + strlen(world) )
    {
    kwsys_ios::cerr << "failed to write hello to ostr2" << kwsys_ios::endl;
    return 1;
    }
  static const unsigned char array[] = { 0xff,0x4f,0xff,0x51,0x00,0x29,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x3e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x3e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x07,0x01,0x01,0xff,0x52,0x00,0x0c,0x00,0x00,0x00,0x01,0x00,0x05,0x04,0x04,0x00,0x01,0xff,0x5c,0x00,0x13,0x40,0x40,0x48,0x48,0x50,0x48,0x48,0x50,0x48,0x48,0x50,0x48,0x48,0x50,0x48,0x48,0x50,0xff,0x64,0x00,0x2c,0x00,0x00,0x43,0x72,0x65,0x61,0x74,0x65,0x64,0x20,0x62,0x79,0x20,0x49,0x54,0x4b,0x2f,0x47,0x44,0x43,0x4d,0x2f,0x4f,0x70,0x65,0x6e,0x4a,0x50,0x45,0x47,0x20,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x31,0x2e,0x30,0xff,0x90,0x00,0x0a,0x00,0x00,0x00,0x00,0x06,0x2c,0x00,0x01,0xff,0x93,0xcf,0xb0,0x18,0x08,0x7f,0xc6,0x99,0xbf,0xff,0xc0,0xf8,0xc1,0xc1,0xf3,0x05,0x81,0xf2,0x83,0x0a,0xa5,0xff,0x10,0x90,0xbf,0x2f,0xff,0x04,0xa8,0x7f,0xc0,0xf8,0xc4,0xc1,0xf3,0x09,0x81,0xf3,0x0c,0x19,0x34 };
  const size_t narray = sizeof(array); // 180
  kwsys_ios::stringstream strstr;
  strstr.write( (char*)array, narray );
  //strstr.seekp( narray / 2 ); // set position of put pointer in mid string
  if(strstr.str().size() != narray )
    {
    kwsys_ios::cerr << "failed to write array to strstr" << kwsys_ios::endl;
    return 1;
    }

  kwsys_ios::istringstream istr(" 10 20 str ");
  kwsys_stl::string s;
  int x;
  if(istr >> x)
    {
    if(x != 10)
      {
      kwsys_ios::cerr << "x != 10" << kwsys_ios::endl;
      return 1;
      }
    }
  else
    {
    kwsys_ios::cerr << "Failed to read 10 from istr" << kwsys_ios::endl;
    return 1;
    }
  if(istr >> x)
    {
    if(x != 20)
      {
      kwsys_ios::cerr << "x != 20" << kwsys_ios::endl;
      return 1;
      }
    }
  else
    {
    kwsys_ios::cerr << "Failed to read 20 from istr" << kwsys_ios::endl;
    return 1;
    }
  if(istr >> s)
    {
    if(s != "str")
      {
      kwsys_ios::cerr << "s != \"str\"" << kwsys_ios::endl;
      return 1;
      }
    }
  else
    {
    kwsys_ios::cerr << "Failed to read str from istr" << kwsys_ios::endl;
    return 1;
    }
  if(istr >> s)
    {
    kwsys_ios::cerr << "Able to read past end of stream" << kwsys_ios::endl;
    return 1;
    }
  else
    {
    // Clear the failure.
    istr.clear(istr.rdstate() & ~kwsys_ios::ios::eofbit);
    istr.clear(istr.rdstate() & ~kwsys_ios::ios::failbit);
    }
  istr.str("30");
  if(istr >> x)
    {
    if(x != 30)
      {
      kwsys_ios::cerr << "x != 30" << kwsys_ios::endl;
      return 1;
      }
    }
  else
    {
    kwsys_ios::cerr << "Failed to read 30 from istr" << kwsys_ios::endl;
    return 1;
    }

  kwsys_ios::stringstream sstr;
  sstr << "40 str2";
  if(sstr >> x)
    {
    if(x != 40)
      {
      kwsys_ios::cerr << "x != 40" << kwsys_ios::endl;
      return 1;
      }
    }
  else
    {
    kwsys_ios::cerr << "Failed to read 40 from sstr" << kwsys_ios::endl;
    return 1;
    }
  if(sstr >> s)
    {
    if(s != "str2")
      {
      kwsys_ios::cerr << "s != \"str2\"" << kwsys_ios::endl;
      return 1;
      }
    }
  else
    {
    kwsys_ios::cerr << "Failed to read str2 from sstr" << kwsys_ios::endl;
    return 1;
    }

  // Just try to compile this.
  if(x == 12345)
    {
    kwsys_ios::ifstream fin("/does_not_exist",
                            kwsys_ios::ios::in | kwsys_ios_binary);
    }

  kwsys_ios::cout << "IOS tests passed" << kwsys_ios::endl;
  return 0;
}
