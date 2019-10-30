
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBlend_DEFINED
#define GrBlend_DEFINED

#include "include/core/SkTypes.h"

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

    kIllegal_GrBlendEquation,

    kFirstAdvancedGrBlendEquation = kScreen_GrBlendEquation,
    kLast_GrBlendEquation = kIllegal_GrBlendEquation,
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

    kIllegal_GrBlendCoeff,

    kLast_GrBlendCoeff = kIllegal_GrBlendCoeff,
};

static const int kGrBlendCoeffCnt = kLast_GrBlendCoeff + 1;

static constexpr bool GrBlendCoeffRefsSrc(const GrBlendCoeff coeff) {
    return kSC_GrBlendCoeff == coeff || kISC_GrBlendCoeff == coeff || kSA_GrBlendCoeff == coeff ||
           kISA_GrBlendCoeff == coeff;
}

static constexpr bool GrBlendCoeffRefsDst(const GrBlendCoeff coeff) {
    return kDC_GrBlendCoeff == coeff || kIDC_GrBlendCoeff == coeff || kDA_GrBlendCoeff == coeff ||
           kIDA_GrBlendCoeff == coeff;
}

static constexpr bool GrBlendCoeffRefsSrc2(const GrBlendCoeff coeff) {
    return kS2C_GrBlendCoeff == coeff || kIS2C_GrBlendCoeff == coeff ||
           kS2A_GrBlendCoeff == coeff || kIS2A_GrBlendCoeff == coeff;
}

static constexpr bool GrBlendCoeffsUseSrcColor(GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff) {
    return kZero_GrBlendCoeff != srcCoeff || GrBlendCoeffRefsSrc(dstCoeff);
}

static constexpr bool GrBlendCoeffsUseDstColor(GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff) {
    return GrBlendCoeffRefsDst(srcCoeff) || kZero_GrBlendCoeff != dstCoeff;
}

static constexpr bool GrBlendEquationIsAdvanced(GrBlendEquation equation) {
    return equation >= kFirstAdvancedGrBlendEquation
        && equation != kIllegal_GrBlendEquation;
}

static constexpr bool GrBlendModifiesDst(GrBlendEquation equation, GrBlendCoeff srcCoeff,
                                         GrBlendCoeff dstCoeff) {
    return (kAdd_GrBlendEquation != equation && kReverseSubtract_GrBlendEquation != equation) ||
           kZero_GrBlendCoeff != srcCoeff || kOne_GrBlendCoeff != dstCoeff;
}

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
 *
 * Moreover, if the blend doesn't modify the dst at all then it is ok to arbitrarily modify the src
 * color so folding in coverage is allowed.
 */
static constexpr bool GrBlendAllowsCoverageAsAlpha(GrBlendEquation equation,
                                                   GrBlendCoeff srcCoeff,
                                                   GrBlendCoeff dstCoeff) {
    return GrBlendEquationIsAdvanced(equation) ||
           !GrBlendModifiesDst(equation, srcCoeff, dstCoeff) ||
           ((kAdd_GrBlendEquation == equation || kReverseSubtract_GrBlendEquation == equation) &&
            !GrBlendCoeffRefsSrc(srcCoeff) &&
            (kOne_GrBlendCoeff == dstCoeff || kISC_GrBlendCoeff == dstCoeff ||
             kISA_GrBlendCoeff == dstCoeff));
}

#endif
