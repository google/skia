/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Geoffrey Viola <geoffrey.viola@asirobots.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGhsMultiGpj.h"

#include "cmGeneratedFileStream.h"

void GhsMultiGpj::WriteGpjTag(Types const gpjType,
                              cmGeneratedFileStream *const filestream)
{
  char const *tag;
  switch (gpjType)
    {
    case INTERGRITY_APPLICATION:
      tag = "INTEGRITY Application";
      break;
    case LIBRARY:
      tag = "Library";
      break;
    case PROJECT:
      tag = "Project";
      break;
    case PROGRAM:
      tag = "Program";
      break;
    case REFERENCE:
      tag = "Reference";
      break;
    case SUBPROJECT:
      tag = "Subproject";
      break;
    default:
      tag = "";
    }
  *filestream << "[" << tag << "]" << std::endl;
}
