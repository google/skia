/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackZIPGenerator_h
#define cmCPackZIPGenerator_h

#include "cmCPackArchiveGenerator.h"

/** \class cmCPackZIPGenerator
 * \brief A generator for ZIP files
 */
class cmCPackZIPGenerator : public cmCPackArchiveGenerator
{
public:
  cmCPackTypeMacro(cmCPackZIPGenerator, cmCPackArchiveGenerator);

  /**
   * Construct generator
   */
  cmCPackZIPGenerator();
  virtual ~cmCPackZIPGenerator();

protected:
  virtual const char* GetOutputExtension() { return ".zip"; }
};

#endif
