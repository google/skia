/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTo.h"
#include "src/effects/SkPackBits.h"

#include <cstring>

size_t SkPackBits::ComputeMaxSize8(size_t srcSize) {
    // worst case is the number of 8bit values + 1 byte per (up to) 128 entries.
    return ((srcSize + 127) >> 7) + srcSize;
}

static uint8_t* flush_same8(uint8_t dst[], uint8_t value, size_t count) {
    while (count > 0) {
        size_t n = count > 128 ? 128 : count;
        *dst++ = (uint8_t)(n - 1);
        *dst++ = (uint8_t)value;
        count -= n;
    }
    return dst;
}

static uint8_t* flush_diff8(uint8_t* SK_RESTRICT dst,
                            const uint8_t* SK_RESTRICT src, size_t count) {
    while (count > 0) {
        size_t n = count > 128 ? 128 : count;
        *dst++ = (uint8_t)(n + 127);
        memcpy(dst, src, n);
        src += n;
        dst += n;
        count -= n;
    }
    return dst;
}

size_t SkPackBits::Pack8(const uint8_t* SK_RESTRICT src, size_t srcSize,
                         uint8_t* SK_RESTRICT dst, size_t dstSize) {
    if (dstSize < ComputeMaxSize8(srcSize)) {
        return 0;
    }

    uint8_t* const origDst = dst;
    const uint8_t* stop = src + srcSize;

    for (intptr_t count = stop - src; count > 0; count = stop - src) {
        if (1 == count) {
            *dst++ = 0;
            *dst++ = *src;
            break;
        }

        unsigned value = *src;
        const uint8_t* s = src + 1;

        if (*s == value) { // accumulate same values...
            do {
                s++;
                if (s == stop) {
                    break;
                }
            } while (*s == value);
            dst = flush_same8(dst, value, SkToInt(s - src));
        } else {    // accumulate diff values...
            do {
                if (++s == stop) {
                    goto FLUSH_DIFF;
                }
                // only stop if we hit 3 in a row,
                // otherwise we get bigger than compuatemax
            } while (*s != s[-1] || s[-1] != s[-2]);
            s -= 2; // back up so we don't grab the "same" values that follow
        FLUSH_DIFF:
            dst = flush_diff8(dst, src, SkToInt(s - src));
        }
        src = s;
    }
    return dst - origDst;
}

int SkPackBits::Unpack8(const uint8_t* SK_RESTRICT src, size_t srcSize,
                        uint8_t* SK_RESTRICT dst, size_t dstSize) {
    uint8_t* const origDst = dst;
    uint8_t* const endDst = dst + dstSize;
    const uint8_t* stop = src + srcSize;

    while (src < stop) {
        unsigned n = *src++;
        if (n <= 127) {   // repeat count (n + 1)
            n += 1;
            if (dst > (endDst - n) || src >= stop) {
                return 0;
            }
            memset(dst, *src++, n);
        } else {    // same count (n - 127)
            n -= 127;
            if (dst > (endDst - n) || src > (stop - n)) {
                return 0;
            }
            memcpy(dst, src, n);
            src += n;
        }
        dst += n;
    }
    SkASSERT(src <= stop);
    SkASSERT(dst <= endDst);
    return SkToInt(dst - origDst);
}
