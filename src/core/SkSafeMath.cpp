/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSafeMath.h"
#include "SkTFitsIn.h"

#if defined(_MSC_VER)
    #include <intrin.h>
#endif

#if defined(_WIN32)
    size_t SkSafeMath::mul(size_t x, size_t y) {
        uint64_t result = __emulu(x,y);
        if (!SkTFitsIn<uint32_t>(result)) {
            fOK = false;
        }
        return result;
    }
    size_t SkSafeMath::add(size_t x, size_t y) {
        uint32_t result = x+y;
        if (result < x) {
            fOK = false;
        }
        return result;
    }

#elif defined(_WIN64)
    size_t SkSafeMath::mul(size_t x, size_t y) {
        uint64_t hi, lo = _umul128(x,y,&hi);
        if (hi) {
            fOK = false;
        }
        return lo;
    }
    size_t SkSafeMath::add(size_t x, size_t y) {
        uint64_t result = x+y;
        if (result < x) {
            fOK = false;
        }
        return result;
    }

#elif defined(__SIZE_WIDTH__) && __SIZE_WIDTH__ == 32
    size_t SkSafeMath::mul(size_t x, size_t y) {
        uint32_t result;
        if (__builtin_umul_overflow(x,y,&result)) {
            fOK = false;
        }
        return result;
    }
    size_t SkSafeMath::add(size_t x, size_t y) {
        uint32_t result;
        if (__builtin_uadd_overflow(x,y,&result)) {
            fOK = false;
        }
        return result;
    }

#elif defined(__SIZE_WIDTH__) && __SIZE_WIDTH__ == 64
    size_t SkSafeMath::mul(size_t x, size_t y) {
        uint64_t result;
        if (__builtin_umulll_overflow(x,y,&result)) {
            fOK = false;
        }
        return result;
    }
    size_t SkSafeMath::add(size_t x, size_t y) {
        uint64_t result;
        if (__builtin_uaddll_overflow(x,y,&result)) {
            fOK = false;
        }
        return result;
    }

#endif
