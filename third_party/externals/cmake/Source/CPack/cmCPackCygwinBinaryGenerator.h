/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackCygwinBinaryGenerator_h
#define cmCPackCygwinBinaryGenerator_h

#include "cmCPackTarBZip2Generator.h"

/** \class cmCPackCygwinBinaryGenerator
 * \brief A generator for TarBZip2 files
 */
class cmCPackCygwinBinaryGenerator : public cmCPackTarBZip2Generator
{
public:
  cmCPackTypeMacro(cmCPackCygwinBinaryGenerator, cmCPackTarBZip2Generator);

  /**
   * Construct generator
   */
  cmCPackCygwinBinaryGenerator();
  virtual ~cmCPackCygwinBinaryGenerator();
protected:
  virtual int InitializeInternal();
  int PackageFiles();
  virtual const char* GetOutputExtension();
  std::string OutputExtension;
};

#endif
