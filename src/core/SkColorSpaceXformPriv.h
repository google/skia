/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformPriv_DEFINED
#define SkColorSpaceXformPriv_DEFINED

#include "SkColorSpace_Base.h"
#include "SkHalf.h"
#include "SkSRGB.h"

#define AI SK_ALWAYS_INLINE

#define SkCSXformPrintfDefined 0
#define SkCSXformPrintf(...)

// Interpolating lookup in a variably sized table.
static AI float interp_lut(float input, const float* table, int tableSize) {
    float index = input * (tableSize - 1);
    float diff = index - sk_float_floor2int(index);
    return table[(int) sk_float_floor2int(index)] * (1.0f - diff) +
           table[(int) sk_float_ceil2int(index)] * diff;
}

// Inverse table lookup.  Ex: what index corresponds to the input value?  This will
// have strange results when the table is non-increasing.  But any sane gamma
// function will be increasing.
static float inverse_interp_lut(float input, const float* table, int tableSize) {
    if (input <= table[0]) {
        return table[0];
    } else if (input >= table[tableSize - 1]) {
        return 1.0f;
    }

    for (int i = 1; i < tableSize; i++) {
        if (table[i] >= input) {
            // We are guaranteed that input is greater than table[i - 1].
            float diff = input - table[i - 1];
            float distance = table[i] - table[i - 1];
            float index = (i - 1) + diff / distance;
            return index / (tableSize - 1);
        }
    }

    // Should be unreachable, since we'll return before the loop if input is
    // larger than the last entry.
    SkASSERT(false);
    return 0.0f;
}

#undef AI

#endif
