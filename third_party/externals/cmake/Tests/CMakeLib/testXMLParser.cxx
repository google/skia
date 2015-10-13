#include "testXMLParser.h"

#include "cmXMLParser.h"

#include <cmsys/ios/iostream>

int testXMLParser(int, char*[])
{
  // TODO: Derive from parser and check attributes.
  cmXMLParser parser;
  if(!parser.ParseFile(SOURCE_DIR "/testXMLParser.xml"))
    {
    cmsys_ios::cerr << "cmXMLParser failed!" << cmsys_ios::endl;
    return 1;
    }
  return 0;
}
