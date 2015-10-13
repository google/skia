/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Geoffrey Viola <geoffrey.viola@asirobots.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocalGhsMultiGenerator_h
#define cmLocalGhsMultiGenerator_h

#include "cmLocalGenerator.h"

class cmGeneratedFileStream;

/** \class cmLocalGhsMultiGenerator
 * \brief Write Green Hills MULTI project files.
 *
 * cmLocalGhsMultiGenerator produces a set of .gpj
 * file for each target in its mirrored directory.
 */
class cmLocalGhsMultiGenerator : public cmLocalGenerator
{
public:
  cmLocalGhsMultiGenerator(cmGlobalGenerator* gg, cmLocalGenerator* parent,
                           cmState::Snapshot snapshot);

  virtual ~cmLocalGhsMultiGenerator();

  /**
   * Generate the makefile for this directory.
   */
  virtual void Generate();
};

#endif
