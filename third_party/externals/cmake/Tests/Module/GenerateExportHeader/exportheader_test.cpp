
#include "libshared.h"

#include "libstatic.h"

// #define BUILD_FAIL

#ifndef BUILD_FAIL
#define DOES_NOT_BUILD(function)
#else
#define DOES_NOT_BUILD(function) function
#endif

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>

void compare(const char* refName, const char* testName)
{
  std::ifstream ref;
  ref.open(refName);
  if (!ref.is_open())
    {
    std::cout << "Could not open \"" << refName << "\"." << std::endl;
    exit(1);
    }
  std::ifstream test;
  test.open(testName);
  if (!test.is_open())
    {
    std::cout << "Could not open \"" << testName << "\"." << std::endl;
    exit(1);
    }

  while (!ref.eof() && !test.eof())
    {
    std::string refLine;
    std::string testLine;
    std::getline(ref, refLine);
    std::getline(test, testLine);
    if (testLine.size() && testLine[testLine.size()-1] == ' ')
      {
      testLine = testLine.substr(0, testLine.size() - 1);
      }
    if (refLine != testLine)
      {
      std::cout << "Ref and test are not the same:\n  Ref:  \""
                          << refLine << "\"\n  Test: \"" << testLine << "\"\n";
      exit(1);
      }
    }
  if (!ref.eof() || !test.eof())
    {
    std::cout << "Ref and test have differing numbers of lines.";
    exit(1);
    }
}

int main()
{
  {
    Libshared l;
    l.libshared();
    l.libshared_exported();
    l.libshared_deprecated();
    l.libshared_not_exported();

    DOES_NOT_BUILD(l.libshared_excluded();)
  }

  {
    LibsharedNotExported l;
    DOES_NOT_BUILD(l.libshared();)
    l.libshared_exported();
    l.libshared_deprecated();
    DOES_NOT_BUILD(l.libshared_not_exported();)
    DOES_NOT_BUILD(l.libshared_excluded();)
  }

  {
    LibsharedExcluded l;
    DOES_NOT_BUILD(l.libshared();)
    l.libshared_exported();
    l.libshared_deprecated();
    DOES_NOT_BUILD(l.libshared_not_exported();)
    DOES_NOT_BUILD(l.libshared_excluded();)
  }

  libshared_exported();
  libshared_deprecated();
  DOES_NOT_BUILD(libshared_not_exported();)
  DOES_NOT_BUILD(libshared_excluded();)

  {
    Libstatic l;
    l.libstatic();
    l.libstatic_exported();
    l.libstatic_deprecated();
    l.libstatic_not_exported();
    l.libstatic_excluded();
  }

  {
    LibstaticNotExported l;
    l.libstatic();
    l.libstatic_exported();
    l.libstatic_deprecated();
    l.libstatic_not_exported();
    l.libstatic_excluded();
  }

  {
    LibstaticExcluded l;
    l.libstatic();
    l.libstatic_exported();
    l.libstatic_deprecated();
    l.libstatic_not_exported();
    l.libstatic_excluded();
  }

  libstatic_exported();
  libstatic_deprecated();
  libstatic_not_exported();
  libstatic_excluded();

#define STRINGIFY_IMPL(A) #A
#define STRINGIFY(A) STRINGIFY_IMPL(A)

  compare(STRINGIFY(SRC_DIR) "/libshared_export.h",
          STRINGIFY(BIN_DIR) "/libshared/libshared_export.h");
  compare(STRINGIFY(SRC_DIR) "/libstatic_export.h",
          STRINGIFY(BIN_DIR) "/libstatic/libstatic_export.h");

  return 0;
}
