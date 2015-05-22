
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTypes.h"
#include "SkTLogic.h"
#include "GrXferProcessor.h"

#ifndef GrBlend_DEFINED
#define GrBlend_DEFINED

template<GrBlendCoeff Coeff>
struct GrTBlendCoeffRefsSrc : SkTBool<kSC_GrBlendCoeff == Coeff ||
                                      kISC_GrBlendCoeff == Coeff ||
                                      kSA_GrBlendCoeff == Coeff ||
                                      kISA_GrBlendCoeff == Coeff> {};

#define GR_BLEND_COEFF_REFS_SRC(COEFF) \
    GrTBlendCoeffRefsSrc<COEFF>::value

inline bool GrBlendCoeffRefsSrc(GrBlendCoeff coeff) {
    switch (coeff) {
        case kSC_GrBlendCoeff:
        case kISC_GrBlendCoeff:
        case kSA_GrBlendCoeff:
        case kISA_GrBlendCoeff:
            return true;
        default:
            return false;
    }
}


template<GrBlendCoeff Coeff>
struct GrTBlendCoeffRefsDst : SkTBool<kDC_GrBlendCoeff == Coeff ||
                                      kIDC_GrBlendCoeff == Coeff ||
                                      kDA_GrBlendCoeff == Coeff ||
                                      kIDA_GrBlendCoeff == Coeff> {};

#define GR_BLEND_COEFF_REFS_DST(COEFF) \
    GrTBlendCoeffRefsDst<COEFF>::value

inline bool GrBlendCoeffRefsDst(GrBlendCoeff coeff) {
    switch (coeff) {
        case kDC_GrBlendCoeff:
        case kIDC_GrBlendCoeff:
        case kDA_GrBlendCoeff:
        case kIDA_GrBlendCoeff:
            return true;
        default:
            return false;
    }
}


template<GrBlendCoeff Coeff>
struct GrTBlendCoeffRefsSrc2 : SkTBool<kS2C_GrBlendCoeff == Coeff ||
                                       kIS2C_GrBlendCoeff == Coeff ||
                                       kS2A_GrBlendCoeff == Coeff ||
                                       kIS2A_GrBlendCoeff == Coeff> {};

#define GR_BLEND_COEFF_REFS_SRC2(COEFF) \
    GrTBlendCoeffRefsSrc2<COEFF>::value


template<GrBlendCoeff SrcCoeff, GrBlendCoeff DstCoeff>
struct GrTBlendCoeffsUseSrcColor : SkTBool<kZero_GrBlendCoeff != SrcCoeff ||
                                           GR_BLEND_COEFF_REFS_SRC(DstCoeff)> {};

#define GR_BLEND_COEFFS_USE_SRC_COLOR(SRC_COEFF, DST_COEFF) \
    GrTBlendCoeffsUseSrcColor<SRC_COEFF, DST_COEFF>::value


template<GrBlendCoeff SrcCoeff, GrBlendCoeff DstCoeff>
struct GrTBlendCoeffsUseDstColor : SkTBool<GR_BLEND_COEFF_REFS_DST(SrcCoeff) ||
                                           kZero_GrBlendCoeff != DstCoeff> {};

#define GR_BLEND_COEFFS_USE_DST_COLOR(SRC_COEFF, DST_COEFF) \
    GrTBlendCoeffsUseDstColor<SRC_COEFF, DST_COEFF>::value


template<GrBlendEquation Equation>
struct GrTBlendEquationIsAdvanced : SkTBool<Equation >= kFirstAdvancedGrBlendEquation> {};

#define GR_BLEND_EQUATION_IS_ADVANCED(EQUATION) \
    GrTBlendEquationIsAdvanced<EQUATION>::value

inline bool GrBlendEquationIsAdvanced(GrBlendEquation equation) {
    return equation >= kFirstAdvancedGrBlendEquation;
}


template<GrBlendEquation BlendEquation, GrBlendCoeff SrcCoeff, GrBlendCoeff DstCoeff>
struct GrTBlendModifiesDst : SkTBool<(kAdd_GrBlendEquation != BlendEquation &&
                                      kReverseSubtract_GrBlendEquation != BlendEquation) ||
                                     kZero_GrBlendCoeff != SrcCoeff ||
                                     kOne_GrBlendCoeff != DstCoeff> {};

#define GR_BLEND_MODIFIES_DST(EQUATION, SRC_COEFF, DST_COEFF) \
    GrTBlendModifiesDst<EQUATION, SRC_COEFF, DST_COEFF>::value


/**
 * Advanced blend equations can always tweak alpha for coverage. (See GrCustomXfermode.cpp)
 *
 * For "add" and "reverse subtract" the blend equation with f=coverage is:
 *
 *   D' = f * (S * srcCoeff + D * dstCoeff) + (1-f) * D
 *      = f * S * srcCoeff + D * (f * dstCoeff + (1 - f))
 *
 * (Let srcCoeff be negative for reverse subtract.) We can tweak alpha for coverage when the
 * following relationship holds:
 *
 *   (f*S) * srcCoeff' + D * dstCoeff' == f * S * srcCoeff + D * (f * dstCoeff + (1 - f))
 *
 * (Where srcCoeff' and dstCoeff' have any reference to S pre-multiplied by f.)
 *
 * It's easy to see this works for the src term as long as srcCoeff' == srcCoeff (meaning srcCoeff
 * does not reference S). For the dst term, this will work as long as the following is true:
 *|
 *   dstCoeff' == f * dstCoeff + (1 - f)
 *   dstCoeff' == 1 - f * (1 - dstCoeff)
 *
 * By inspection we can see this will work as long as dstCoeff has a 1, and any other term in
 * dstCoeff references S.
 */
template<GrBlendEquation Equation, GrBlendCoeff SrcCoeff, GrBlendCoeff DstCoeff>
struct GrTBlendCanTweakAlphaForCoverage : SkTBool<GR_BLEND_EQUATION_IS_ADVANCED(Equation) ||
                                                  ((kAdd_GrBlendEquation == Equation ||
                                                    kReverseSubtract_GrBlendEquation == Equation) &&
                                                   !GR_BLEND_COEFF_REFS_SRC(SrcCoeff) &&
                                                   (kOne_GrBlendCoeff == DstCoeff ||
                                                    kISC_GrBlendCoeff == DstCoeff ||
                                                    kISA_GrBlendCoeff == DstCoeff))> {};

#define GR_BLEND_CAN_TWEAK_ALPHA_FOR_COVERAGE(EQUATION, SRC_COEFF, DST_COEFF) \
    GrTBlendCanTweakAlphaForCoverage<EQUATION, SRC_COEFF, DST_COEFF>::value

#endif
