/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmHexFileConverter_h
#define cmHexFileConverter_h

/** \class cmHexFileConverter
 * \brief Can detects Intel Hex and Motorola S-record files and convert them
 *        to binary files.
 *
 */
class cmHexFileConverter
{
public:
  enum FileType {Binary, IntelHex, MotorolaSrec};
  static FileType DetermineFileType(const char* inFileName);
  static bool TryConvert(const char* inFileName, const char* outFileName);
};

#endif
