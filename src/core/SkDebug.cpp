
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTypes.h"

#ifdef SK_DEBUG

int8_t SkToS8(intmax_t x) {
    SkASSERT((int8_t)x == x);
    return (int8_t)x;
}

uint8_t SkToU8(uintmax_t x) {
    SkASSERT((uint8_t)x == x);
    return (uint8_t)x;
}

int16_t SkToS16(intmax_t x) {
    SkASSERT((int16_t)x == x);
    return (int16_t)x;
}

uint16_t SkToU16(uintmax_t x) {
    SkASSERT((uint16_t)x == x);
    return (uint16_t)x;
}

int32_t SkToS32(intmax_t x) {
    SkASSERT((int32_t)x == x);
    return (int32_t)x;
}

uint32_t SkToU32(uintmax_t x) {
    SkASSERT((uint32_t)x == x);
    return (uint32_t)x;
}

int SkToInt(intmax_t x) {
    SkASSERT((int)x == x);
    return (int)x;
}

unsigned SkToUInt(uintmax_t x) {
    SkASSERT((unsigned)x == x);
    return (unsigned)x;
}

size_t SkToSizeT(uintmax_t x) {
    SkASSERT((size_t)x == x);
    return (size_t)x;
}

#endif
