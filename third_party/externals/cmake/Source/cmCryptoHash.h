/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCryptoHash_h
#define cmCryptoHash_h

#include "cmStandardIncludes.h"

#include <cmsys/auto_ptr.hxx>

class cmCryptoHash
{
public:
  virtual ~cmCryptoHash() {}
  static cmsys::auto_ptr<cmCryptoHash> New(const char* algo);
  std::string HashString(const std::string& input);
  std::string HashFile(const std::string& file);
protected:
  virtual void Initialize()=0;
  virtual void Append(unsigned char const*, int)=0;
  virtual std::string Finalize()=0;
};

class cmCryptoHashMD5: public cmCryptoHash
{
  struct cmsysMD5_s* MD5;
public:
  cmCryptoHashMD5();
  ~cmCryptoHashMD5();
protected:
  virtual void Initialize();
  virtual void Append(unsigned char const* buf, int sz);
  virtual std::string Finalize();
};

#define cmCryptoHash_SHA_CLASS_DECL(SHA) \
  class cmCryptoHash##SHA: public cmCryptoHash \
  { \
    union _SHA_CTX* SHA; \
  public: \
    cmCryptoHash##SHA(); \
    ~cmCryptoHash##SHA(); \
  protected: \
    virtual void Initialize(); \
    virtual void Append(unsigned char const* buf, int sz); \
    virtual std::string Finalize(); \
  }

cmCryptoHash_SHA_CLASS_DECL(SHA1);
cmCryptoHash_SHA_CLASS_DECL(SHA224);
cmCryptoHash_SHA_CLASS_DECL(SHA256);
cmCryptoHash_SHA_CLASS_DECL(SHA384);
cmCryptoHash_SHA_CLASS_DECL(SHA512);

#undef cmCryptoHash_SHA_CLASS_DECL

#endif
