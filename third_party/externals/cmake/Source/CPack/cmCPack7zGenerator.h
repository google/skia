/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPack7zGenerator_h
#define cmCPack7zGenerator_h

#include "cmCPackArchiveGenerator.h"

/** \class cmCPack7zGenerator
 * \brief A generator for 7z files
 */
class cmCPack7zGenerator : public cmCPackArchiveGenerator
{
public:
  cmCPackTypeMacro(cmCPack7zGenerator, cmCPackArchiveGenerator);

  /**
   * Construct generator
   */
  cmCPack7zGenerator();
  virtual ~cmCPack7zGenerator();

protected:
  virtual const char* GetOutputExtension() { return ".7z"; }
};

#endif
