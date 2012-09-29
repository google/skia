
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTypes.h"

#ifdef SK_DEBUG

int8_t SkToS8(long x)
{
    SkASSERT((int8_t)x == x);
    return (int8_t)x;
}

uint8_t SkToU8(size_t x)
{
    SkASSERT((uint8_t)x == x);
    return (uint8_t)x;
}

int16_t SkToS16(long x)
{
    SkASSERT((int16_t)x == x);
    return (int16_t)x;
}

uint16_t SkToU16(size_t x)
{
    SkASSERT((uint16_t)x == x);
    return (uint16_t)x;
}

int32_t SkToS32(long x)
{
    SkASSERT((int32_t)x == x);
    return (int32_t)x;
}

uint32_t SkToU32(size_t x)
{
    SkASSERT((uint32_t)x == x);
    return (uint32_t)x;
}

#endif

