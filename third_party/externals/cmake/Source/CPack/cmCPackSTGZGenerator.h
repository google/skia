/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackSTGZGenerator_h
#define cmCPackSTGZGenerator_h


#include "cmCPackTGZGenerator.h"

/** \class cmCPackSTGZGenerator
 * \brief A generator for Self extractable TGZ files
 *
 */
class cmCPackSTGZGenerator : public cmCPackTGZGenerator
{
public:
  cmCPackTypeMacro(cmCPackSTGZGenerator, cmCPackTGZGenerator);

  /**
   * Construct generator
   */
  cmCPackSTGZGenerator();
  virtual ~cmCPackSTGZGenerator();

protected:
  int PackageFiles();
  virtual int InitializeInternal();
  int GenerateHeader(std::ostream* os);
  virtual const char* GetOutputExtension() { return ".sh"; }
};

#endif
