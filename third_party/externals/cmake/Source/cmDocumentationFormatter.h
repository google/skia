/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef _cmDocumentationFormatter_h
#define _cmDocumentationFormatter_h

#include "cmStandardIncludes.h"

/** This is just a helper class to make it build with MSVC 6.0.
Actually the enums and internal classes could directly go into
cmDocumentation, but then MSVC6 complains in RequestedHelpItem that
cmDocumentation is an undefined type and so it doesn't know the enums.
Moving the enums to a class which is then already completely parsed helps
against this. */
class cmDocumentationEnums
{
public:
  /** Types of help provided.  */
  enum Type
  {
    None, Version, Usage, Help, Full, ListManuals, ListCommands,
    ListModules, ListProperties, ListVariables, ListPolicies, ListGenerators,
    OneManual, OneCommand, OneModule, OneProperty, OneVariable, OnePolicy,
    OldCustomModules
  };
};

class cmDocumentationSection;

/** Print documentation in a simple text format. */
class cmDocumentationFormatter
{
public:
  cmDocumentationFormatter();
  virtual ~cmDocumentationFormatter();
  void PrintFormatted(std::ostream& os, const char* text);

  virtual void PrintSection(std::ostream& os,
                            cmDocumentationSection const& section);
  virtual void PrintPreformatted(std::ostream& os, const char* text);
  virtual void PrintParagraph(std::ostream& os, const char* text);
  void PrintColumn(std::ostream& os, const char* text);
  void SetIndent(const char* indent);
private:
  int TextWidth;
  const char* TextIndent;
};

#endif
