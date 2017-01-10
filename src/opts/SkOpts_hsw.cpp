/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSafe_math.h"   // Keep this first.

// Please note carefully.
// It is not safe for _opts.h files included here to use STL types, for the
// same reason we just had to include SkSafe_math.h: STL types are templated,
// defined in headers, but not in anonymous namespaces.  It's very easy to
// cause ODR violations with these types and AVX+ code generation.

#include "SkOpts.h"
#define SK_OPTS_NS hsw
#include "SkBitmapFilter_opts.h"

#if defined(_INC_MATH) && !defined(INC_MATH_IS_SAFE_NOW)
    #error We have included ucrt\math.h without protecting it against ODR violation.
#endif

namespace SkOpts {
    void Init_hsw() {
        convolve_vertically = hsw::convolve_vertically;
    }
}

