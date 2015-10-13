/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Daniel Pfeifer <daniel@pfeifer-mail.de>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmXMLWiter_h
#define cmXMLWiter_h

#include "cmStandardIncludes.h"
#include "cmXMLSafe.h"

#include <ostream>
#include <stack>
#include <string>
#include <vector>

class cmXMLWriter
{
public:
  cmXMLWriter(std::ostream& output, std::size_t level = 0);
  ~cmXMLWriter();

  void StartDocument(const char* encoding = "UTF-8");
  void EndDocument();

  void StartElement(std::string const& name);
  void EndElement();

  void BreakAttributes();

  template <typename T>
  void Attribute(const char* name, T const& value)
    {
    this->PreAttribute();
    this->Output << name << "=\"" << SafeAttribute(value) << '"';
    }

  template <typename T>
  void Element(std::string const& name, T const& value)
    {
    this->StartElement(name);
    this->Content(value);
    this->EndElement();
    }

  template <typename T>
  void Content(T const& content)
    {
    this->PreContent();
    this->Output << SafeContent(content);
    }

  void Comment(const char* comment);

  void CData(std::string const& data);

  void ProcessingInstruction(const char* target, const char* data);

  void FragmentFile(const char* fname);

private:
  cmXMLWriter(const cmXMLWriter&);
  cmXMLWriter& operator=(const cmXMLWriter&);

  void ConditionalLineBreak(bool condition, std::size_t indent);

  void PreAttribute();
  void PreContent();

  void CloseStartElement();

private:
  static cmXMLSafe SafeAttribute(const char* value)
    {
    return cmXMLSafe(value);
    }

  static cmXMLSafe SafeAttribute(std::string const& value)
    {
    return cmXMLSafe(value);
    }

  template <typename T>
  static T SafeAttribute(T value)
    {
    return value;
    }

  static cmXMLSafe SafeContent(const char* value)
    {
    return cmXMLSafe(value).Quotes(false);
    }

  static cmXMLSafe SafeContent(std::string const& value)
    {
    return cmXMLSafe(value).Quotes(false);
    }

  template <typename T>
  static T SafeContent(T value)
    {
    return value;
    }

private:
  std::ostream& Output;
  std::stack<std::string, std::vector<std::string> > Elements;
  std::size_t Level;
  bool ElementOpen;
  bool BreakAttrib;
  bool IsContent;
};

#endif
