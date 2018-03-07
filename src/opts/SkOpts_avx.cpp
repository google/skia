/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSafe_math.h"   // Keep this first.
#include "SkOpts.h"

#if defined(_INC_MATH) && !defined(INC_MATH_IS_SAFE_NOW)
    #error We have included ucrt\math.h without protecting it against ODR violation.
#endif

#define SK_OPTS_NS avx
#include "SkUtils_opts.h"

namespace SkOpts {
    void Init_avx() {
        memset16 = SK_OPTS_NS::memset16;
        memset32 = SK_OPTS_NS::memset32;
        memset64 = SK_OPTS_NS::memset64;
    }
}
