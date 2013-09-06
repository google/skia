
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBlend.h"

static inline GrBlendCoeff swap_coeff_src_dst(GrBlendCoeff coeff) {
    switch (coeff) {
        case kDC_GrBlendCoeff:
            return kSC_GrBlendCoeff;
        case kIDC_GrBlendCoeff:
            return kISC_GrBlendCoeff;
        case kDA_GrBlendCoeff:
            return kSA_GrBlendCoeff;
        case kIDA_GrBlendCoeff:
            return kISA_GrBlendCoeff;
        case kSC_GrBlendCoeff:
            return kDC_GrBlendCoeff;
        case kISC_GrBlendCoeff:
            return kIDC_GrBlendCoeff;
        case kSA_GrBlendCoeff:
            return kDA_GrBlendCoeff;
        case kISA_GrBlendCoeff:
            return kIDA_GrBlendCoeff;
        default:
            return coeff;
    }
}

static inline unsigned saturated_add(unsigned a, unsigned b) {
    SkASSERT(a <= 255);
    SkASSERT(b <= 255);
    unsigned sum = a + b;
    if (sum > 255) {
        sum = 255;
    }
    return sum;
}

static GrColor add_colors(GrColor src, GrColor dst) {
    unsigned r = saturated_add(GrColorUnpackR(src), GrColorUnpackR(dst));
    unsigned g = saturated_add(GrColorUnpackG(src), GrColorUnpackG(dst));
    unsigned b = saturated_add(GrColorUnpackB(src), GrColorUnpackB(dst));
    unsigned a = saturated_add(GrColorUnpackA(src), GrColorUnpackA(dst));
    return GrColorPackRGBA(r, g, b, a);
}

static inline bool valid_color(uint32_t compFlags) {
     return (kRGBA_GrColorComponentFlags & compFlags) == kRGBA_GrColorComponentFlags;
}

static GrColor simplify_blend_term(GrBlendCoeff* srcCoeff,
                                   GrColor srcColor, uint32_t srcCompFlags,
                                   GrColor dstColor, uint32_t dstCompFlags,
                                   GrColor constantColor) {

    SkASSERT(!GrBlendCoeffRefsSrc(*srcCoeff));
    SkASSERT(NULL != srcCoeff);

    // Check whether srcCoeff can be reduced to kOne or kZero based on known color inputs.
    // We could pick out the coeff r,g,b,a values here and use them to compute the blend term color,
    // if possible, below but that is not implemented now.
    switch (*srcCoeff) {
        case kIDC_GrBlendCoeff:
            dstColor = ~dstColor; // fallthrough
        case kDC_GrBlendCoeff:
            if (valid_color(dstCompFlags)) {
                if (0xffffffff == dstColor) {
                    *srcCoeff = kOne_GrBlendCoeff;
                } else if (0 == dstColor) {
                    *srcCoeff = kZero_GrBlendCoeff;
                }
            }
            break;

        case kIDA_GrBlendCoeff:
            dstColor = ~dstColor; // fallthrough
        case kDA_GrBlendCoeff:
            if (kA_GrColorComponentFlag & dstCompFlags) {
                if (0xff == GrColorUnpackA(dstColor)) {
                    *srcCoeff = kOne_GrBlendCoeff;
                } else if (0 == GrColorUnpackA(dstColor)) {
                    *srcCoeff = kZero_GrBlendCoeff;
                }
            }
            break;

        case kIConstC_GrBlendCoeff:
            constantColor = ~constantColor; // fallthrough
        case kConstC_GrBlendCoeff:
            if (0xffffffff == constantColor) {
                *srcCoeff = kOne_GrBlendCoeff;
            } else if (0 == constantColor) {
                *srcCoeff = kZero_GrBlendCoeff;
            }
            break;

        case kIConstA_GrBlendCoeff:
            constantColor = ~constantColor; // fallthrough
        case kConstA_GrBlendCoeff:
            if (0xff == GrColorUnpackA(constantColor)) {
                *srcCoeff = kOne_GrBlendCoeff;
            } else if (0 == GrColorUnpackA(constantColor)) {
                *srcCoeff = kZero_GrBlendCoeff;
            }
            break;

        default:
            break;
    }
    // We may have invalidated these above and shouldn't read them again.
    SkDEBUGCODE(dstColor = constantColor = GrColor_ILLEGAL;)

    if (kZero_GrBlendCoeff == *srcCoeff || (valid_color(srcCompFlags) && 0 == srcColor)) {
        *srcCoeff = kZero_GrBlendCoeff;
        return 0;
    }

    if (kOne_GrBlendCoeff == *srcCoeff && valid_color(srcCompFlags)) {
        return srcColor;
    } else {
        return GrColor_ILLEGAL;
    }
}

GrColor GrSimplifyBlend(GrBlendCoeff* srcCoeff,
                        GrBlendCoeff* dstCoeff,
                        GrColor srcColor, uint32_t srcCompFlags,
                        GrColor dstColor, uint32_t dstCompFlags,
                        GrColor constantColor) {
    GrColor srcTermColor = simplify_blend_term(srcCoeff,
                                               srcColor, srcCompFlags,
                                               dstColor, dstCompFlags,
                                               constantColor);

    // We call the same function to simplify the dst blend coeff. We trick it out by swapping the
    // src and dst.
    GrBlendCoeff spoofedCoeff = swap_coeff_src_dst(*dstCoeff);
    GrColor dstTermColor = simplify_blend_term(&spoofedCoeff,
                                               dstColor, dstCompFlags,
                                               srcColor, srcCompFlags,
                                               constantColor);
    *dstCoeff = swap_coeff_src_dst(spoofedCoeff);

    if (GrColor_ILLEGAL != srcTermColor && GrColor_ILLEGAL != dstTermColor) {
        return add_colors(srcTermColor, dstTermColor);
    } else {
        return GrColor_ILLEGAL;
    }
}
