/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackTGZGenerator_h
#define cmCPackTGZGenerator_h

#include "cmCPackArchiveGenerator.h"

/** \class cmCPackTGZGenerator
 * \brief A generator for TGZ files
 *
 */
class cmCPackTGZGenerator : public cmCPackArchiveGenerator
{
public:
  cmCPackTypeMacro(cmCPackTGZGenerator, cmCPackArchiveGenerator);
  /**
   * Construct generator
   */
  cmCPackTGZGenerator();
  virtual ~cmCPackTGZGenerator();
protected:
  virtual const char* GetOutputExtension() { return ".tar.gz"; }
};

#endif
