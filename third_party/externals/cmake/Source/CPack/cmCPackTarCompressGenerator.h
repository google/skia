/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackTarCompressGenerator_h
#define cmCPackTarCompressGenerator_h

#include "cmCPackTGZGenerator.h"

/** \class cmCPackTarCompressGenerator
 * \brief A generator for TarCompress files
 */
class cmCPackTarCompressGenerator : public cmCPackArchiveGenerator
{
public:
  cmCPackTypeMacro(cmCPackTarCompressGenerator, cmCPackArchiveGenerator);
  /**
   * Construct generator
   */
  cmCPackTarCompressGenerator();
  virtual ~cmCPackTarCompressGenerator();

protected:
  virtual const char* GetOutputExtension() { return ".tar.Z"; }
};

#endif
