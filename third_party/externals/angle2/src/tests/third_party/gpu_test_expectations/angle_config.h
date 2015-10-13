//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_config.h:
//   Helpers for importing the gpu test expectations package from Chrome.
//

#ifndef GPU_TEST_EXPECTATIONS_ANGLE_CONFIG_H_
#define GPU_TEST_EXPECTATIONS_ANGLE_CONFIG_H_

#include <stdint.h>

#include <iostream>

#include "common/debug.h"
#include "common/string_utils.h"

#define DCHECK_EQ(A,B) ASSERT((A) == (B))
#define DCHECK_NE(A,B) ASSERT((A) != (B))
#define DCHECK(X) ASSERT(X)
#define LOG(X) std::cerr

#define GPU_EXPORT

typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

// Shim Chromium's base by importing functions in the bsae namespace.
namespace base
{
    using angle::HexStringToUInt;
    using angle::ReadFileToString;
    using angle::SplitString;
    using angle::SplitStringAlongWhitespace;

    // StringPrintf is called differently in ANGLE but using cannot change
    // the name of the imported function. Use a define to change the name.
    using ::FormatString;
    #define StringPrintf FormatString
}

// TODO(jmadill): other platforms
#if defined(_WIN32) || defined(_WIN64)
#    define OS_WIN
#elif defined(__linux__)
#    define OS_LINUX
#elif defined(__APPLE__)
#    define OS_MACOSX
#else
#    error "Unsupported platform"
#endif

#endif
