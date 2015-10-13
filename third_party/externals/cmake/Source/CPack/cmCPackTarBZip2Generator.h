/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackTarBZip2Generator_h
#define cmCPackTarBZip2Generator_h

#include "cmCPackArchiveGenerator.h"

/** \class cmCPackTarBZip2Generator
 * \brief A generator for TarBZip2 files
 */
class cmCPackTarBZip2Generator : public cmCPackArchiveGenerator
{
public:
  cmCPackTypeMacro(cmCPackTarBZip2Generator, cmCPackArchiveGenerator);
  /**
   * Construct generator
   */
  cmCPackTarBZip2Generator();
  virtual ~cmCPackTarBZip2Generator();
protected:
  virtual const char* GetOutputExtension() { return ".tar.bz2"; }
};

#endif
