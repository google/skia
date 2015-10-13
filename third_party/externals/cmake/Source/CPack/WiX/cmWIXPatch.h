/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmWIXPatch_h
#define cmWIXPatch_h

#include "cmWIXSourceWriter.h"
#include "cmWIXPatchParser.h"

#include <string>

/** \class cmWIXPatch
 * \brief Class that maintains and applies patch fragments
 */
class cmWIXPatch
{
public:
  cmWIXPatch(cmCPackLog* logger);

  void LoadFragments(std::string const& patchFilePath);

  void ApplyFragment(std::string const& id, cmWIXSourceWriter& writer);

  bool CheckForUnappliedFragments();

private:
  void ApplyElement(const cmWIXPatchElement& element,
    cmWIXSourceWriter& writer);

  cmCPackLog* Logger;

  cmWIXPatchParser::fragment_map_t Fragments;
};


#endif
