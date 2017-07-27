/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSafeMath.h"

#if defined(_M_IX86) || (defined(__SIZEOF_SIZE_T__) && __SIZEOF_SIZE_T__ == 4)
    #include "SkTFitsIn.h"
    size_t SkSafeMath::mul(size_t x, size_t y) {
        auto result = (uint64_t)x * y;
        if (!SkTFitsIn<size_t>(result)) {
            fOK = false;
        }
        return result;
    }

#elif defined(_M_X64)
    #include <intrin.h>
    size_t SkSafeMath::mul(size_t x, size_t y) {
        uint64_t hi = 0;
        uint64_t lo = _umul128(x,y,&hi);
        if (hi) {
            fOK = false;
        }
        return lo;
    }

#elif defined(__SIZEOF_SIZE_T__) && __SIZEOF_SIZE_T__ == 8
    #include "SkTFitsIn.h"
    size_t SkSafeMath::mul(size_t x, size_t y) {
        auto result = (unsigned __int128)x * y;
        if (!SkTFitsIn<size_t>(result)) {
            fOK = false;
        }
        return result;
    }

#endif

size_t SkSafeMath::add(size_t x, size_t y) {
    size_t result = x+y;
    if (result < x) {
        fOK = false;
    }
    return result;
}
