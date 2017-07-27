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

static uint32_t safe_mul_32(uint32_t x, uint32_t y, bool* ok) {
#if defined(_MSC_VER)
    uint64_t result = __emulu(x,y);
    if (!SkTFitsIn<uint32_t>(result)) {
        *ok = false;
    }
    return result;
#else
    uint32_t result;
    if (__builtin_mul_overflow(x,y,&result)) {
        *ok = false;
    }
    return result;
#endif
}

static uint64_t safe_mul_64(uint64_t x, uint64_t y, bool* ok) {
#if defined(_MSC_VER)
    uint64_t hi, lo = _umul128(x,y,&hi);
    if (hi) {
        *ok = false;
    }
    return lo;
#else
    uint64_t result;
    if (__builtin_mul_overflow(x,y,&result)) {
        *ok = false;
    }
    return result;
#endif
}

static uint32_t safe_add_32(uint32_t x, uint32_t y, bool* ok) {
#if defined(_MSC_VER)
    uint32_t result = x+y;
    if (result < x) {
        *ok = false;
    }
    return result;
#else
    uint32_t result;
    if (__builtin_add_overflow(x,y,&result)) {
        *ok = false;
    }
    return result;
#endif
}

static uint64_t safe_add_64(uint64_t x, uint64_t y, bool* ok) {
#if defined(_MSC_VER)
    uint64_t result = x+y;
    if (result < x) {
        *ok = false;
    }
    return result;
#else
    uint64_t result;
    if (__builtin_add_overflow(x,y,&result)) {
        *ok = false;
    }
    return result;
#endif
}

size_t SkSafeMath::mul(size_t x, size_t y) {
    if (sizeof(size_t) == 4) {
        return safe_mul_32(x,y,&fOK);
    } else {
        return safe_mul_64(x,y,&fOK);
    }
}

size_t SkSafeMath::add(size_t x, size_t y) {
    if (sizeof(size_t) == 4) {
        return safe_add_32(x,y,&fOK);
    } else {
        return safe_add_64(x,y,&fOK);
    }
}
