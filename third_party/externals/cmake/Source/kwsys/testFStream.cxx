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

#if defined(_MSC_VER)
# pragma warning (disable:4786)
#endif

#include KWSYS_HEADER(FStream.hxx)
#include KWSYS_HEADER(ios/iostream)
#include <string.h>
#ifdef __BORLANDC__
# include <mem.h> /* memcmp */
#endif

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "FStream.hxx.in"
# include "kwsys_ios_iostream.h.in"
#endif


//----------------------------------------------------------------------------
static int testNoFile()
{
  kwsys::ifstream in_file("NoSuchFile.txt");
  if(in_file)
    {
    return 1;
    }

  return 0;
}

static kwsys::FStream::BOM expected_bom[5] =
{
  kwsys::FStream::BOM_UTF8,
  kwsys::FStream::BOM_UTF16LE,
  kwsys::FStream::BOM_UTF16BE,
  kwsys::FStream::BOM_UTF32LE,
  kwsys::FStream::BOM_UTF32BE
};

static unsigned char expected_bom_data[5][5] =
{
    {3, 0xEF, 0xBB, 0xBF},
    {2, 0xFF, 0xFE},
    {2, 0xFE, 0xFF},
    {4, 0xFF, 0xFE, 0x00, 0x00},
    {4, 0x00, 0x00, 0xFE, 0xFF},
};

static unsigned char file_data[5][45] =
{
    {11, 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'},
    {22, 0x48, 0x00, 0x65, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x6F, 0x00, 0x20, 0x00,
    0x57, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x6C, 0x00, 0x64, 0x00},
    {22, 0x00, 0x48, 0x00, 0x65, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x6F, 0x00, 0x20,
    0x00, 0x57, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x6C, 0x00, 0x64},
    {44, 0x48, 0x00, 0x00, 0x00, 0x65, 0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00,
    0x6C, 0x00, 0x00, 0x00, 0x6F, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
    0x57, 0x00, 0x00, 0x00, 0x6F, 0x00, 0x00, 0x00, 0x72, 0x00, 0x00, 0x00,
    0x6C, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00},
    {44, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x65, 0x00, 0x00, 0x00, 0x6C,
    0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x6F, 0x00, 0x00, 0x00, 0x20,
    0x00, 0x00, 0x00, 0x57, 0x00, 0x00, 0x00, 0x6F, 0x00, 0x00, 0x00, 0x72,
    0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x64},
};

//----------------------------------------------------------------------------
static int testBOM()
{
  // test various encodings in binary mode
  for(int i=0; i<5; i++)
    {
      {
      kwsys::ofstream out("bom.txt", kwsys::ofstream::binary);
      out.write(reinterpret_cast<const char*>(expected_bom_data[i]+1),
                *expected_bom_data[i]);
      out.write(reinterpret_cast<const char*>(file_data[i]+1),
                file_data[i][0]);
      }

    kwsys::ifstream in("bom.txt", kwsys::ofstream::binary);
    kwsys::FStream::BOM bom = kwsys::FStream::ReadBOM(in);
    if(bom != expected_bom[i])
      {
      kwsys_ios::cout << "Unexpected BOM " << i << std::endl;
      return 1;
      }
    char data[45];
    in.read(data, file_data[i][0]);
    if(!in.good())
      {
      kwsys_ios::cout << "Unable to read data " << i << std::endl;
      return 1;
      }

    if(memcmp(data, file_data[i]+1, file_data[i][0]) != 0)
      {
      kwsys_ios::cout << "Incorrect read data " << i << std::endl;
      return 1;
      }

    }

  // test text file without bom
  {
    {
    kwsys::ofstream out("bom.txt");
    out << "Hello World";
    }

    kwsys::ifstream in("bom.txt");
    kwsys::FStream::BOM bom = kwsys::FStream::ReadBOM(in);
    if(bom != kwsys::FStream::BOM_None)
      {
      kwsys_ios::cout << "Unexpected BOM for none case" << std::endl;
      return 1;
      }
    char data[45];
    in.read(data, file_data[0][0]);
    if(!in.good())
      {
      kwsys_ios::cout << "Unable to read data for none case" << std::endl;
      return 1;
      }

    if(memcmp(data, file_data[0]+1, file_data[0][0]) != 0)
      {
      kwsys_ios::cout << "Incorrect read data for none case" << std::endl;
      return 1;
      }
  }

  // test text file with utf-8 bom
  {
    {
    kwsys::ofstream out("bom.txt");
    out.write(reinterpret_cast<const char*>(expected_bom_data[0]+1),
              *expected_bom_data[0]);
    out << "Hello World";
    }

    kwsys::ifstream in("bom.txt");
    kwsys::FStream::BOM bom = kwsys::FStream::ReadBOM(in);
    if(bom != kwsys::FStream::BOM_UTF8)
      {
      kwsys_ios::cout << "Unexpected BOM for utf-8 case" << std::endl;
      return 1;
      }
    char data[45];
    in.read(data, file_data[0][0]);
    if(!in.good())
      {
      kwsys_ios::cout << "Unable to read data for utf-8 case" << std::endl;
      return 1;
      }

    if(memcmp(data, file_data[0]+1, file_data[0][0]) != 0)
      {
      kwsys_ios::cout << "Incorrect read data for utf-8 case" << std::endl;
      return 1;
      }
  }

  return 0;
}


//----------------------------------------------------------------------------
int testFStream(int, char*[])
{
  int ret = 0;

  ret |= testNoFile();
  ret |= testBOM();

  return ret;
}
