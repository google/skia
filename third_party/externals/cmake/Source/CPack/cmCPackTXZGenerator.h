/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackTXZGenerator_h
#define cmCPackTXZGenerator_h

#include "cmCPackArchiveGenerator.h"

/** \class cmCPackTXZGenerator
 * \brief A generator for TXZ files
 *
 */
class cmCPackTXZGenerator : public cmCPackArchiveGenerator
{
public:
  cmCPackTypeMacro(cmCPackTXZGenerator, cmCPackArchiveGenerator);
  /**
   * Construct generator
   */
  cmCPackTXZGenerator();
  virtual ~cmCPackTXZGenerator();
protected:
  virtual const char* GetOutputExtension() { return ".tar.xz"; }
};

#endif
