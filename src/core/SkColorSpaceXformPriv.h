/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformPriv_DEFINED
#define SkColorSpaceXformPriv_DEFINED

#include "SkColorSpace_Base.h"
#include "SkColorSpaceXform.h"
#include "SkHalf.h"
#include "SkSRGB.h"

#define SkCSXformPrintfDefined 0
#define SkCSXformPrintf(...)

// Interpolating lookup in a variably sized table.
static inline float interp_lut(float input, const float* table, int tableSize) {
    float index = input * (tableSize - 1);
    float diff = index - sk_float_floor2int(index);
    return table[(int) sk_float_floor2int(index)] * (1.0f - diff) +
           table[(int) sk_float_ceil2int(index)] * diff;
}

// Expand range from 0-1 to 0-255, then convert.
static inline uint8_t clamp_normalized_float_to_byte(float v) {
    // The ordering of the logic is a little strange here in order
    // to make sure we convert NaNs to 0.
    v = v * 255.0f;
    if (v >= 254.5f) {
        return 255;
    } else if (v >= 0.5f) {
        return (uint8_t) (v + 0.5f);
    } else {
        return 0;
    }
}

static inline float clamp_0_1(float v) {
    // The ordering of the logic is a little strange here in order
    // to make sure we convert NaNs to 0.
    if (v >= 1.0f) {
        return 1.0f;
    } else if (v >= 0.0f) {
        return v;
    } else {
        return 0.0f;
    }
}

/**
 *  Invert table lookup.  Ex: what indices corresponds to the input values?
 *  This will have strange results when the table is not increasing.
 *  But any sane gamma function will be increasing.
 *  @param outTableFloat Destination table for float (0-1) results. Can be nullptr if not wanted.
 *  @param outTableByte  Destination table for byte (0-255) results. Can be nullptr if not wanted.
 *  @param outTableSize  Number of elements in |outTableFloat| or |outTableBytes|
 *  @param inTable       The source table to invert
 *  @param inTableSize   The number of elements in |inTable|
 */
static inline void invert_table_gamma(float* outTableFloat, uint8_t* outTableByte,
                                      int outTableSize, const float* inTable, int inTableSize) {
    // should never have a gamma table this small anyway, 0/1 are either not allowed
    // or imply a non-table gamma such as linear/exponential
    SkASSERT(inTableSize >= 2);
    int inIndex = 1;
    for (int outIndex = 0; outIndex < outTableSize; ++outIndex) {
        const float input = outIndex / (outTableSize - 1.0f);
        while (inIndex < inTableSize - 1 && inTable[inIndex] < input) {
            ++inIndex;
        }
        const float diff            = input - inTable[inIndex - 1];
        const float distance        = inTable[inIndex] - inTable[inIndex - 1];
        const float normalizedIndex = (inIndex - 1) + diff / distance;
        const float index           = normalizedIndex / (inTableSize - 1);
        if (outTableByte) {
            outTableByte[outIndex] = clamp_normalized_float_to_byte(index);
        }
        if (outTableFloat) {
            outTableFloat[outIndex] = clamp_0_1(index);
        }
    }
}

static inline SkColorSpaceXform::ColorFormat select_xform_format(SkColorType colorType) {
    switch (colorType) {
        case kRGBA_8888_SkColorType:
            return SkColorSpaceXform::kRGBA_8888_ColorFormat;
        case kBGRA_8888_SkColorType:
            return SkColorSpaceXform::kBGRA_8888_ColorFormat;
        case kRGBA_F16_SkColorType:
            return SkColorSpaceXform::kRGBA_F16_ColorFormat;
        case kRGB_565_SkColorType:
            return SkColorSpaceXform::kBGR_565_ColorFormat;
        default:
            SkASSERT(false);
            return SkColorSpaceXform::kRGBA_8888_ColorFormat;
    }
}

#endif
