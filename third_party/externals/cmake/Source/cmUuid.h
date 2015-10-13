/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmUuid_h
#define cmUuid_h

#include "cmStandardIncludes.h"

/** \class cmUuid
 * \brief Utility class to generate UUIDs as defined by RFC4122
 *
 */
class cmUuid
{
public:
  cmUuid();

  std::string FromMd5(std::vector<unsigned char> const& uuidNamespace,
    std::string const& name) const;

  std::string FromSha1(std::vector<unsigned char> const& uuidNamespace,
    std::string const& name) const;

  bool StringToBinary(std::string const& input,
    std::vector<unsigned char> &output) const;

private:
  std::string ByteToHex(unsigned char byte) const;

  void CreateHashInput(std::vector<unsigned char> const& uuidNamespace,
    std::string const& name, std::vector<unsigned char> &output) const;

  std::string FromDigest(const unsigned char* digest,
    unsigned char version) const;

  bool StringToBinaryImpl(std::string const& input,
    std::vector<unsigned char> &output) const;

  std::string BinaryToString(const unsigned char* input) const;

  bool IntFromHexDigit(char input, char& output) const;

  std::vector<int> Groups;
};


#endif
