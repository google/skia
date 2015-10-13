/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmWIXSourceWriter_h
#define cmWIXSourceWriter_h

#include <vector>
#include <string>
#include <cmsys/FStream.hxx>

#include <CPack/cmCPackLog.h>

/** \class cmWIXSourceWriter
 * \brief Helper class to generate XML WiX source files
 */
class cmWIXSourceWriter
{
public:
  cmWIXSourceWriter(cmCPackLog* logger,
    std::string const& filename, bool isIncludeFile = false);

  ~cmWIXSourceWriter();

  void BeginElement(std::string const& name);

  void EndElement(std::string const& name);

  void AddProcessingInstruction(
    std::string const& target, std::string const& content);

  void AddAttribute(
    std::string const& key, std::string const& value);

  void AddAttributeUnlessEmpty(
    std::string const& key, std::string const& value);

  static std::string CMakeEncodingToUtf8(std::string const& value);

protected:
   cmCPackLog* Logger;

private:
  enum State
  {
    DEFAULT,
    BEGIN
  };

  void WriteXMLDeclaration();

  void Indent(size_t count);

  static std::string EscapeAttributeValue(std::string const& value);

  cmsys::ofstream File;

  State State;

  std::vector<std::string> Elements;

  std::string SourceFilename;
};

#endif
