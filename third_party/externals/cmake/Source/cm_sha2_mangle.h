/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cm_sha2_mangle_h
#define cm_sha2_mangle_h

/* Mangle sha2 symbol names to avoid possible conflict with
   implementations in other libraries to which CMake links.  */
#define SHA1_Data                  cmSHA1_Data
#define SHA1_End                   cmSHA1_End
#define SHA1_Final                 cmSHA1_Final
#define SHA1_Init                  cmSHA1_Init
#define SHA1_Internal_Transform    cmSHA1_Internal_Transform
#define SHA1_Update                cmSHA1_Update
#define SHA224_Data                cmSHA224_Data
#define SHA224_End                 cmSHA224_End
#define SHA224_Final               cmSHA224_Final
#define SHA224_Init                cmSHA224_Init
#define SHA224_Internal_Transform  cmSHA224_Internal_Transform
#define SHA224_Update              cmSHA224_Update
#define SHA256_Data                cmSHA256_Data
#define SHA256_End                 cmSHA256_End
#define SHA256_Final               cmSHA256_Final
#define SHA256_Init                cmSHA256_Init
#define SHA256_Internal_Init       cmSHA256_Internal_Init
#define SHA256_Internal_Last       cmSHA256_Internal_Last
#define SHA256_Internal_Transform  cmSHA256_Internal_Transform
#define SHA256_Update              cmSHA256_Update
#define SHA384_Data                cmSHA384_Data
#define SHA384_End                 cmSHA384_End
#define SHA384_Final               cmSHA384_Final
#define SHA384_Init                cmSHA384_Init
#define SHA384_Update              cmSHA384_Update
#define SHA512_Data                cmSHA512_Data
#define SHA512_End                 cmSHA512_End
#define SHA512_Final               cmSHA512_Final
#define SHA512_Init                cmSHA512_Init
#define SHA512_Internal_Init       cmSHA512_Internal_Init
#define SHA512_Internal_Last       cmSHA512_Internal_Last
#define SHA512_Internal_Transform  cmSHA512_Internal_Transform
#define SHA512_Update              cmSHA512_Update

#endif
