/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpaceXform.h"

static inline bool compute_gamut_xform(SkMatrix44* srcToDst, const SkMatrix44& srcToXYZ,
                                       const SkMatrix44& dstToXYZ) {
    if (!dstToXYZ.invert(srcToDst)) {
        return false;
    }

    srcToDst->postConcat(srcToXYZ);
    return true;
}

std::unique_ptr<SkColorSpaceXform> SkColorSpaceXform::New(const sk_sp<SkColorSpace>& srcSpace,
                                                          const sk_sp<SkColorSpace>& dstSpace) {
    if (!srcSpace || !dstSpace) {
        // Invalid input
        return nullptr;
    }

    if (as_CSB(srcSpace)->colorLUT() || as_CSB(dstSpace)->colorLUT()) {
        // Unimplemented
        return nullptr;
    }

    SkMatrix44 srcToDst(SkMatrix44::kUninitialized_Constructor);
    if (!compute_gamut_xform(&srcToDst, srcSpace->xyz(), dstSpace->xyz())) {
        return nullptr;
    }

    if (as_CSB(srcSpace)->gammas()->isValues() && as_CSB(dstSpace)->gammas()->isValues()) {
        float srcGammas[3];
        float dstGammas[3];
        srcGammas[0] = as_CSB(srcSpace)->gammas()->fRed.fValue;
        srcGammas[1] = as_CSB(srcSpace)->gammas()->fGreen.fValue;
        srcGammas[2] = as_CSB(srcSpace)->gammas()->fBlue.fValue;
        dstGammas[0] = 1.0f / as_CSB(dstSpace)->gammas()->fRed.fValue;
        dstGammas[1] = 1.0f / as_CSB(dstSpace)->gammas()->fGreen.fValue;
        dstGammas[2] = 1.0f / as_CSB(dstSpace)->gammas()->fBlue.fValue;

        return std::unique_ptr<SkColorSpaceXform>(
                new SkGammaByValueXform(srcGammas, srcToDst, dstGammas));
    }

    return std::unique_ptr<SkColorSpaceXform>(
            new SkDefaultXform(as_CSB(srcSpace)->gammas(), srcToDst, as_CSB(dstSpace)->gammas()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline float byte_to_float(uint8_t v) {
    return ((float) v) * (1.0f / 255.0f);
}

static inline uint8_t clamp_float_to_byte(float v) {
    v = v * 255.0f;
    if (v > 255.0f) {
        return 255;
    } else if (v <= 0.0f) {
        return 0;
    } else {
        return (uint8_t) (v + 0.5f);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkGammaByValueXform::SkGammaByValueXform(float srcGammas[3], const SkMatrix44& srcToDst,
                                         float dstGammas[3])
    : fSrcToDst(srcToDst)
{
    memcpy(fSrcGammas, srcGammas, 3 * sizeof(float));
    memcpy(fDstGammas, dstGammas, 3 * sizeof(float));
}

void SkGammaByValueXform::xform_RGBA_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const {
    while (len-- > 0) {
        float srcFloats[3];
        srcFloats[0] = byte_to_float((*src >>  0) & 0xFF);
        srcFloats[1] = byte_to_float((*src >>  8) & 0xFF);
        srcFloats[2] = byte_to_float((*src >> 16) & 0xFF);

        // Convert to linear.
        srcFloats[0] = pow(srcFloats[0], fSrcGammas[0]);
        srcFloats[1] = pow(srcFloats[1], fSrcGammas[1]);
        srcFloats[2] = pow(srcFloats[2], fSrcGammas[2]);

        // Convert to dst gamut.
        float dstFloats[3];
        dstFloats[0] = srcFloats[0] * fSrcToDst.getFloat(0, 0) +
                       srcFloats[1] * fSrcToDst.getFloat(1, 0) +
                       srcFloats[2] * fSrcToDst.getFloat(2, 0) + fSrcToDst.getFloat(3, 0);
        dstFloats[1] = srcFloats[0] * fSrcToDst.getFloat(0, 1) +
                       srcFloats[1] * fSrcToDst.getFloat(1, 1) +
                       srcFloats[2] * fSrcToDst.getFloat(2, 1) + fSrcToDst.getFloat(3, 1);
        dstFloats[2] = srcFloats[0] * fSrcToDst.getFloat(0, 2) +
                       srcFloats[1] * fSrcToDst.getFloat(1, 2) +
                       srcFloats[2] * fSrcToDst.getFloat(2, 2) + fSrcToDst.getFloat(3, 2);

        // Convert to dst gamma.
        dstFloats[0] = pow(dstFloats[0], fDstGammas[0]);
        dstFloats[1] = pow(dstFloats[1], fDstGammas[1]);
        dstFloats[2] = pow(dstFloats[2], fDstGammas[2]);

        *dst = SkPackARGB32NoCheck(((*src >> 24) & 0xFF),
                                   clamp_float_to_byte(dstFloats[0]),
                                   clamp_float_to_byte(dstFloats[1]),
                                   clamp_float_to_byte(dstFloats[2]));

        dst++;
        src++;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// Interpolating lookup in a variably sized table.
static inline float interp_lut(uint8_t byte, float* table, size_t tableSize) {
    float index = byte_to_float(byte) * (tableSize - 1);
    float diff = index - sk_float_floor2int(index);
    return table[(int) sk_float_floor2int(index)] * (1.0f - diff) +
            table[(int) sk_float_ceil2int(index)] * diff;
}

// Inverse table lookup.  Ex: what index corresponds to the input value?  This will
// have strange results when the table is non-increasing.  But any sane gamma
// function will be increasing.
// FIXME (msarett):
// This is a placeholder implementation for inverting table gammas.  First, I need to
// verify if there are actually destination profiles that require this functionality.
// Next, there are certainly faster and more robust approaches to solving this problem.
// The LUT based approach in QCMS would be a good place to start.
static inline float interp_lut_inv(float input, float* table, size_t tableSize) {
    if (input <= table[0]) {
        return table[0];
    } else if (input >= table[tableSize - 1]) {
        return 1.0f;
    }

    for (uint32_t i = 1; i < tableSize; i++) {
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

SkDefaultXform::SkDefaultXform(const sk_sp<SkGammas>& srcGammas, const SkMatrix44& srcToDst,
                               const sk_sp<SkGammas>& dstGammas)
    : fSrcGammas(srcGammas)
    , fSrcToDst(srcToDst)
    , fDstGammas(dstGammas)
{}

void SkDefaultXform::xform_RGBA_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const {
    while (len-- > 0) {
        // Convert to linear.
        // FIXME (msarett):
        // Rather than support three different strategies of transforming gamma, QCMS
        // builds a 256 entry float lookup table from the gamma info.  This handles
        // the gamma transform and the conversion from bytes to floats.  This may
        // be simpler and faster than our current approach.
        float srcFloats[3];
        for (int i = 0; i < 3; i++) {
            const SkGammaCurve& gamma = (*fSrcGammas)[i];
            uint8_t byte = (*src >> (8 * i)) & 0xFF;
            if (gamma.isValue()) {
                srcFloats[i] = pow(byte_to_float(byte), gamma.fValue);
            } else if (gamma.isTable()) {
                srcFloats[i] = interp_lut(byte, gamma.fTable.get(), gamma.fTableSize);
            } else {
                SkASSERT(gamma.isParametric());
                float component = byte_to_float(byte);
                if (component < gamma.fD) {
                    // Y = E * X + F
                    srcFloats[i] = gamma.fE * component + gamma.fF;
                } else {
                    // Y = (A * X + B)^G + C
                    srcFloats[i] = pow(gamma.fA * component + gamma.fB, gamma.fG) + gamma.fC;
                }
            }
        }

        // Convert to dst gamut.
        float dstFloats[3];
        dstFloats[0] = srcFloats[0] * fSrcToDst.getFloat(0, 0) +
                       srcFloats[1] * fSrcToDst.getFloat(1, 0) +
                       srcFloats[2] * fSrcToDst.getFloat(2, 0) + fSrcToDst.getFloat(3, 0);
        dstFloats[1] = srcFloats[0] * fSrcToDst.getFloat(0, 1) +
                       srcFloats[1] * fSrcToDst.getFloat(1, 1) +
                       srcFloats[2] * fSrcToDst.getFloat(2, 1) + fSrcToDst.getFloat(3, 1);
        dstFloats[2] = srcFloats[0] * fSrcToDst.getFloat(0, 2) +
                       srcFloats[1] * fSrcToDst.getFloat(1, 2) +
                       srcFloats[2] * fSrcToDst.getFloat(2, 2) + fSrcToDst.getFloat(3, 2);

        // Convert to dst gamma.
        // FIXME (msarett):
        // Rather than support three different strategies of transforming inverse gamma,
        // QCMS builds a large float lookup table from the gamma info.  Is this faster or
        // better than our approach?
        for (int i = 0; i < 3; i++) {
            const SkGammaCurve& gamma = (*fDstGammas)[i];
            if (gamma.isValue()) {
                dstFloats[i] = pow(dstFloats[i], 1.0f / gamma.fValue);
            } else if (gamma.isTable()) {
                // FIXME (msarett):
                // An inverse table lookup is particularly strange and non-optimal.
                dstFloats[i] = interp_lut_inv(dstFloats[i], gamma.fTable.get(), gamma.fTableSize);
            } else {
                SkASSERT(gamma.isParametric());
                // FIXME (msarett):
                // This is a placeholder implementation for inverting parametric gammas.
                // First, I need to verify if there are actually destination profiles that
                // require this functionality. Next, I need to explore other possibilities
                // for this implementation.  The LUT based approach in QCMS would be a good
                // place to start.

                // We need to take the inverse of a piecewise function.  Assume that
                // the gamma function is continuous, or this won't make much sense
                // anyway.
                // Plug in |fD| to the first equation to calculate the new piecewise
                // interval.  Then simply use the inverse of the original functions.
                float interval = gamma.fE * gamma.fD + gamma.fF;
                if (dstFloats[i] < interval) {
                    // X = (Y - F) / E
                    if (0.0f == gamma.fE) {
                        // The gamma curve for this segment is constant, so the inverse
                        // is undefined.
                        dstFloats[i] = 0.0f;
                    } else {
                        dstFloats[i] = (dstFloats[i] - gamma.fF) / gamma.fE;
                    }
                } else {
                    // X = ((Y - C)^(1 / G) - B) / A
                    if (0.0f == gamma.fA || 0.0f == gamma.fG) {
                        // The gamma curve for this segment is constant, so the inverse
                        // is undefined.
                        dstFloats[i] = 0.0f;
                    } else {
                        dstFloats[i] = (pow(dstFloats[i] - gamma.fC, 1.0f / gamma.fG) - gamma.fB)
                                       / gamma.fA;
                    }
                }
            }
        }

        *dst = SkPackARGB32NoCheck(((*src >> 24) & 0xFF),
                                   clamp_float_to_byte(dstFloats[0]),
                                   clamp_float_to_byte(dstFloats[1]),
                                   clamp_float_to_byte(dstFloats[2]));

        dst++;
        src++;
    }
}
