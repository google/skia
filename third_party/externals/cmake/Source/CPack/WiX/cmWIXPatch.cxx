/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmWIXPatch.h"

#include <CPack/cmCPackGenerator.h>

cmWIXPatch::cmWIXPatch(cmCPackLog* logger):
  Logger(logger)
{

}

void cmWIXPatch::LoadFragments(std::string const& patchFilePath)
{
  cmWIXPatchParser parser(Fragments, Logger);
  parser.ParseFile(patchFilePath.c_str());
}

void cmWIXPatch::ApplyFragment(
  std::string const& id, cmWIXSourceWriter& writer)
{
  cmWIXPatchParser::fragment_map_t::iterator i = Fragments.find(id);
  if(i == Fragments.end()) return;

  const cmWIXPatchElement& fragment = i->second;
  for(cmWIXPatchElement::child_list_t::const_iterator
    j = fragment.children.begin(); j != fragment.children.end(); ++j)
    {
    ApplyElement(**j, writer);
    }

  Fragments.erase(i);
}

void cmWIXPatch::ApplyElement(
  const cmWIXPatchElement& element, cmWIXSourceWriter& writer)
{
  writer.BeginElement(element.name);

  for(cmWIXPatchElement::attributes_t::const_iterator
    i = element.attributes.begin(); i != element.attributes.end(); ++i)
    {
    writer.AddAttribute(i->first, i->second);
    }

  for(cmWIXPatchElement::child_list_t::const_iterator
    i = element.children.begin(); i != element.children.end(); ++i)
    {
    ApplyElement(**i, writer);
    }

  writer.EndElement(element.name);
}


bool cmWIXPatch::CheckForUnappliedFragments()
{
  std::string fragmentList;
  for(cmWIXPatchParser::fragment_map_t::const_iterator
    i = Fragments.begin(); i != Fragments.end(); ++i)
    {
    if(!fragmentList.empty())
      {
      fragmentList += ", ";
      }

    fragmentList += "'";
    fragmentList += i->first;
    fragmentList += "'";
    }

  if(fragmentList.size())
    {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Some XML patch fragments did not have matching IDs: " <<
        fragmentList << std::endl);
      return false;
    }

  return true;
}
