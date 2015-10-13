/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmWIXPatchParser.h"

#include <CPack/cmCPackGenerator.h>

#include <cm_expat.h>

cmWIXPatchElement::~cmWIXPatchElement()
{
  for(child_list_t::iterator i = children.begin(); i != children.end(); ++i)
    {
    delete *i;
    }
}

cmWIXPatchParser::cmWIXPatchParser(
  fragment_map_t& fragments, cmCPackLog* logger):
    Logger(logger),
    State(BEGIN_DOCUMENT),
    Valid(true),
    Fragments(fragments)
{

}

void cmWIXPatchParser::StartElement(const std::string& name, const char **atts)
{
  if(State == BEGIN_DOCUMENT)
    {
    if(name == "CPackWiXPatch")
      {
      State = BEGIN_FRAGMENTS;
      }
    else
      {
      ReportValidationError("Expected root element 'CPackWiXPatch'");
      }
    }
  else if(State == BEGIN_FRAGMENTS)
    {
      if(name == "CPackWiXFragment")
        {
        State = INSIDE_FRAGMENT;
        StartFragment(atts);
        }
      else
        {
        ReportValidationError("Expected 'CPackWixFragment' element");
        }
    }
  else if(State == INSIDE_FRAGMENT)
    {
      cmWIXPatchElement &parent = *ElementStack.back();

      parent.children.resize(parent.children.size() + 1);
      cmWIXPatchElement*& currentElement = parent.children.back();
      currentElement = new cmWIXPatchElement;
      currentElement->name = name;

      for(size_t i = 0; atts[i]; i += 2)
        {
        std::string key = atts[i];
        std::string value = atts[i+1];

        currentElement->attributes[key] = value;
        }

      ElementStack.push_back(currentElement);
    }
}

void cmWIXPatchParser::StartFragment(const char **attributes)
{
  for(size_t i = 0; attributes[i]; i += 2)
    {
    std::string key = attributes[i];
    std::string value = attributes[i+1];

    if(key == "Id")
      {
      if(Fragments.find(value) != Fragments.end())
        {
        std::stringstream tmp;
        tmp << "Invalid reuse of 'CPackWixFragment' 'Id': " << value;
        ReportValidationError(tmp.str());
        }

      ElementStack.push_back(&Fragments[value]);
      }
    else
      {
      ReportValidationError(
        "The only allowed 'CPackWixFragment' attribute is 'Id'");
      }
    }
}

void cmWIXPatchParser::EndElement(const std::string& name)
{
  if(State == INSIDE_FRAGMENT)
    {
      if(name == "CPackWiXFragment")
        {
        State = BEGIN_FRAGMENTS;
        ElementStack.clear();
        }
      else
        {
          ElementStack.pop_back();
        }
    }
}

void cmWIXPatchParser::ReportError(int line, int column, const char* msg)
{
  cmCPackLogger(cmCPackLog::LOG_ERROR,
    "Error while processing XML patch file at " << line << ":" << column <<
      ":  "<< msg << std::endl);
  Valid = false;
}

void cmWIXPatchParser::ReportValidationError(std::string const& message)
{
  ReportError(XML_GetCurrentLineNumber(static_cast<XML_Parser>(this->Parser)),
    XML_GetCurrentColumnNumber(static_cast<XML_Parser>(this->Parser)),
    message.c_str());
}

bool cmWIXPatchParser::IsValid() const
{
  return Valid;
}
