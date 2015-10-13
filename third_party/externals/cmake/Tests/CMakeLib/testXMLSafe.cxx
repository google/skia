/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include <cmXMLSafe.h>

#include "cmStandardIncludes.h"

struct test_pair
{
  const char* in;
  const char* out;
};

static test_pair const pairs[] = {
  {"copyright \xC2\xA9", "copyright \xC2\xA9"},
  {"form-feed \f", "form-feed [NON-XML-CHAR-0xC]"},
  {"angles <>", "angles &lt;&gt;"},
  {"ampersand &", "ampersand &amp;"},
  {"bad-byte \x80", "bad-byte [NON-UTF-8-BYTE-0x80]"},
  {0,0}
};

int testXMLSafe(int, char*[])
{
  int result = 0;
  for(test_pair const* p = pairs; p->in; ++p)
    {
    cmXMLSafe xs(p->in);
    std::ostringstream oss;
    oss << xs;
    std::string out = oss.str();
    if(out != p->out)
      {
      printf("expected [%s], got [%s]\n", p->out, out.c_str());
      result = 1;
      }
    }
  return result;
}
