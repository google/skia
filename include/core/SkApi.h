/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkApi_DEFINED
#define SkApi_DEFINED

#if !defined(_MSC_VER)
   #define SK_API __attribute__((visibility("default")))
#elif defined(SKIA_IMPLEMENTATION)
   #define SK_API __declspec(dllexport)
#else
   #define SK_API
#endif


#endif//SkApi_DEFINED
