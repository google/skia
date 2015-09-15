
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBlend_DEFINED
#define GrBlend_DEFINED

#include "GrColor.h"
#include "../private/SkTLogic.h"

/**
 * Equations for alpha-blending.
 */
enum GrBlendEquation {
    // Basic blend equations.
    kAdd_GrBlendEquation,             //<! Cs*S + Cd*D
    kSubtract_GrBlendEquation,        //<! Cs*S - Cd*D
    kReverseSubtract_GrBlendEquation, //<! Cd*D - Cs*S

    // Advanced blend equations. These are described in the SVG and PDF specs.
    kScreen_GrBlendEquation,
    kOverlay_GrBlendEquation,
    kDarken_GrBlendEquation,
    kLighten_GrBlendEquation,
    kColorDodge_GrBlendEquation,
    kColorBurn_GrBlendEquation,
    kHardLight_GrBlendEquation,
    kSoftLight_GrBlendEquation,
    kDifference_GrBlendEquation,
    kExclusion_GrBlendEquation,
    kMultiply_GrBlendEquation,
    kHSLHue_GrBlendEquation,
    kHSLSaturation_GrBlendEquation,
    kHSLColor_GrBlendEquation,
    kHSLLuminosity_GrBlendEquation,

    kFirstAdvancedGrBlendEquation = kScreen_GrBlendEquation,
    kLast_GrBlendEquation = kHSLLuminosity_GrBlendEquation
};

static const int kGrBlendEquationCnt = kLast_GrBlendEquation + 1;


/**
 * Coefficients for alpha-blending.
 */
enum GrBlendCoeff {
    kZero_GrBlendCoeff,    //<! 0
    kOne_GrBlendCoeff,     //<! 1
    kSC_GrBlendCoeff,      //<! src color
    kISC_GrBlendCoeff,     //<! one minus src color
    kDC_GrBlendCoeff,      //<! dst color
    kIDC_GrBlendCoeff,     //<! one minus dst color
    kSA_GrBlendCoeff,      //<! src alpha
    kISA_GrBlendCoeff,     //<! one minus src alpha
    kDA_GrBlendCoeff,      //<! dst alpha
    kIDA_GrBlendCoeff,     //<! one minus dst alpha
    kConstC_GrBlendCoeff,  //<! constant color
    kIConstC_GrBlendCoeff, //<! one minus constant color
    kConstA_GrBlendCoeff,  //<! constant color alpha
    kIConstA_GrBlendCoeff, //<! one minus constant color alpha
    kS2C_GrBlendCoeff,
    kIS2C_GrBlendCoeff,
    kS2A_GrBlendCoeff,
    kIS2A_GrBlendCoeff,

    kLast_GrBlendCoeff = kIS2A_GrBlendCoeff
};

static const int kGrBlendCoeffCnt = kLast_GrBlendCoeff + 1;

/**
 * Given a known blend equation in the form of srcCoeff * srcColor + dstCoeff * dstColor where
 * there may be partial knowledge of the srcColor and dstColor component values, determine what
 * components of the blended output color are known. Coeffs must not refer to the constant or
 * secondary src color.
 */
void GrGetCoeffBlendKnownComponents(GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff,
                                    GrColor srcColor,
                                    GrColorComponentFlags srcColorFlags,
                                    GrColor dstColor,
                                    GrColorComponentFlags dstColorFlags,
                                    GrColor* outColor,
                                    GrColorComponentFlags* outFlags);

template<GrBlendCoeff Coeff>
struct GrTBlendCoeffRefsSrc : skstd::bool_constant<kSC_GrBlendCoeff == Coeff ||
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
struct GrTBlendCoeffRefsDst : skstd::bool_constant<kDC_GrBlendCoeff == Coeff ||
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
struct GrTBlendCoeffRefsSrc2 : skstd::bool_constant<kS2C_GrBlendCoeff == Coeff ||
                                                    kIS2C_GrBlendCoeff == Coeff ||
                                                    kS2A_GrBlendCoeff == Coeff ||
                                                    kIS2A_GrBlendCoeff == Coeff> {};

#define GR_BLEND_COEFF_REFS_SRC2(COEFF) \
    GrTBlendCoeffRefsSrc2<COEFF>::value

inline bool GrBlendCoeffRefsSrc2(GrBlendCoeff coeff) {
    switch (coeff) {
        case kS2C_GrBlendCoeff:
        case kIS2C_GrBlendCoeff:
        case kS2A_GrBlendCoeff:
        case kIS2A_GrBlendCoeff:
            return true;
        default:
            return false;
    }
}


template<GrBlendCoeff SrcCoeff, GrBlendCoeff DstCoeff>
struct GrTBlendCoeffsUseSrcColor : skstd::bool_constant<kZero_GrBlendCoeff != SrcCoeff ||
                                                        GR_BLEND_COEFF_REFS_SRC(DstCoeff)> {};

#define GR_BLEND_COEFFS_USE_SRC_COLOR(SRC_COEFF, DST_COEFF) \
    GrTBlendCoeffsUseSrcColor<SRC_COEFF, DST_COEFF>::value


template<GrBlendCoeff SrcCoeff, GrBlendCoeff DstCoeff>
struct GrTBlendCoeffsUseDstColor : skstd::bool_constant<GR_BLEND_COEFF_REFS_DST(SrcCoeff) ||
                                                        kZero_GrBlendCoeff != DstCoeff> {};

#define GR_BLEND_COEFFS_USE_DST_COLOR(SRC_COEFF, DST_COEFF) \
    GrTBlendCoeffsUseDstColor<SRC_COEFF, DST_COEFF>::value


template<GrBlendEquation Equation>
struct GrTBlendEquationIsAdvanced : skstd::bool_constant<Equation >= kFirstAdvancedGrBlendEquation> {};

#define GR_BLEND_EQUATION_IS_ADVANCED(EQUATION) \
    GrTBlendEquationIsAdvanced<EQUATION>::value

inline bool GrBlendEquationIsAdvanced(GrBlendEquation equation) {
    return equation >= kFirstAdvancedGrBlendEquation;
}


template<GrBlendEquation BlendEquation, GrBlendCoeff SrcCoeff, GrBlendCoeff DstCoeff>
struct GrTBlendModifiesDst : skstd::bool_constant<
    (kAdd_GrBlendEquation != BlendEquation && kReverseSubtract_GrBlendEquation != BlendEquation) ||
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
struct GrTBlendCanTweakAlphaForCoverage : skstd::bool_constant<
    GR_BLEND_EQUATION_IS_ADVANCED(Equation) ||
    ((kAdd_GrBlendEquation == Equation || kReverseSubtract_GrBlendEquation == Equation) &&
      !GR_BLEND_COEFF_REFS_SRC(SrcCoeff) &&
      (kOne_GrBlendCoeff == DstCoeff ||
       kISC_GrBlendCoeff == DstCoeff ||
       kISA_GrBlendCoeff == DstCoeff))> {};

#define GR_BLEND_CAN_TWEAK_ALPHA_FOR_COVERAGE(EQUATION, SRC_COEFF, DST_COEFF) \
    GrTBlendCanTweakAlphaForCoverage<EQUATION, SRC_COEFF, DST_COEFF>::value

#endif
