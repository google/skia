/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Geoffrey Viola <geoffrey.viola@asirobots.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGhsMultiGpj_h
#define cmGhsMultiGpj_h

class cmGeneratedFileStream;

class GhsMultiGpj
{
public:
  enum Types
  {
    INTERGRITY_APPLICATION,
    LIBRARY,
    PROJECT,
    PROGRAM,
    REFERENCE,
    SUBPROJECT
  };

  static void WriteGpjTag(Types const gpjType,
                          cmGeneratedFileStream *filestream);
};

#endif // ! cmGhsMultiGpjType_h
