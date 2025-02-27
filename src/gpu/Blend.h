/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Blend_DEFINED
#define skgpu_Blend_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkColorData.h"

#include <cstdint>

enum class SkBlendMode;
class SkString;

namespace skgpu {

/**
 * Equations for alpha-blending.
 */
enum class BlendEquation : uint8_t {
    // Basic blend equations.
    kAdd,             //<! Cs*S + Cd*D
    kSubtract,        //<! Cs*S - Cd*D
    kReverseSubtract, //<! Cd*D - Cs*S

    // Advanced blend equations. These are described in the SVG and PDF specs.
    kScreen,
    kOverlay,
    kDarken,
    kLighten,
    kColorDodge,
    kColorBurn,
    kHardLight,
    kSoftLight,
    kDifference,
    kExclusion,
    kMultiply,
    kHSLHue,
    kHSLSaturation,
    kHSLColor,
    kHSLLuminosity,

    kIllegal,

    kFirstAdvanced = kScreen,
    kLast = kIllegal,
};

static const int kBlendEquationCnt = static_cast<int>(BlendEquation::kLast) + 1;

/**
 * Coefficients for alpha-blending.
 */
enum class BlendCoeff : uint8_t {
    kZero,    //<! 0
    kOne,     //<! 1
    kSC,      //<! src color
    kISC,     //<! one minus src color
    kDC,      //<! dst color
    kIDC,     //<! one minus dst color
    kSA,      //<! src alpha
    kISA,     //<! one minus src alpha
    kDA,      //<! dst alpha
    kIDA,     //<! one minus dst alpha
    kConstC,  //<! constant color
    kIConstC, //<! one minus constant color
    kS2C,
    kIS2C,
    kS2A,
    kIS2A,

    kIllegal,

    kLast = kIllegal,
};

struct BlendInfo {
    SkDEBUGCODE(SkString dump() const;)

    bool operator==(const BlendInfo& other) const {
        return fEquation == other.fEquation &&
               fSrcBlend == other.fSrcBlend &&
               fDstBlend == other.fDstBlend &&
               fBlendConstant == other.fBlendConstant &&
               fWritesColor == other.fWritesColor;
    }

    skgpu::BlendEquation fEquation = skgpu::BlendEquation::kAdd;
    skgpu::BlendCoeff    fSrcBlend = skgpu::BlendCoeff::kOne;
    skgpu::BlendCoeff    fDstBlend = skgpu::BlendCoeff::kZero;
    SkPMColor4f          fBlendConstant = SK_PMColor4fTRANSPARENT;
    bool                 fWritesColor = true;
};

static const int kBlendCoeffCnt = static_cast<int>(BlendCoeff::kLast) + 1;

static constexpr bool BlendCoeffRefsSrc(const BlendCoeff coeff) {
    return BlendCoeff::kSC == coeff || BlendCoeff::kISC == coeff || BlendCoeff::kSA == coeff ||
           BlendCoeff::kISA == coeff;
}

static constexpr bool BlendCoeffRefsDst(const BlendCoeff coeff) {
    return BlendCoeff::kDC == coeff || BlendCoeff::kIDC == coeff || BlendCoeff::kDA == coeff ||
           BlendCoeff::kIDA == coeff;
}

static constexpr bool BlendCoeffRefsSrc2(const BlendCoeff coeff) {
    return BlendCoeff::kS2C == coeff || BlendCoeff::kIS2C == coeff ||
           BlendCoeff::kS2A == coeff || BlendCoeff::kIS2A == coeff;
}

static constexpr bool BlendCoeffsUseSrcColor(BlendCoeff srcCoeff, BlendCoeff dstCoeff) {
    return BlendCoeff::kZero != srcCoeff || BlendCoeffRefsSrc(dstCoeff);
}

static constexpr bool BlendCoeffsUseDstColor(BlendCoeff srcCoeff,
                                             BlendCoeff dstCoeff,
                                             bool srcColorIsOpaque) {
    return BlendCoeffRefsDst(srcCoeff) ||
           (dstCoeff != BlendCoeff::kZero && !(dstCoeff == BlendCoeff::kISA && srcColorIsOpaque));
}

static constexpr bool BlendEquationIsAdvanced(BlendEquation equation) {
    return equation >= BlendEquation::kFirstAdvanced &&
           equation != BlendEquation::kIllegal;
}

static constexpr bool BlendModifiesDst(BlendEquation equation,
                                       BlendCoeff srcCoeff,
                                       BlendCoeff dstCoeff) {
    return (BlendEquation::kAdd != equation && BlendEquation::kReverseSubtract != equation) ||
            BlendCoeff::kZero != srcCoeff || BlendCoeff::kOne != dstCoeff;
}

static constexpr bool BlendCoeffRefsConstant(const BlendCoeff coeff) {
    return coeff == BlendCoeff::kConstC || coeff == BlendCoeff::kIConstC;
}

static constexpr bool BlendShouldDisable(BlendEquation equation,
                                         BlendCoeff srcCoeff,
                                         BlendCoeff dstCoeff) {
    return (BlendEquation::kAdd == equation || BlendEquation::kSubtract == equation) &&
            BlendCoeff::kOne == srcCoeff && BlendCoeff::kZero == dstCoeff;
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
static constexpr bool BlendAllowsCoverageAsAlpha(BlendEquation equation,
                                                 BlendCoeff srcCoeff,
                                                 BlendCoeff dstCoeff) {
    return BlendEquationIsAdvanced(equation) ||
           !BlendModifiesDst(equation, srcCoeff, dstCoeff) ||
           ((BlendEquation::kAdd == equation || BlendEquation::kReverseSubtract == equation) &&
            !BlendCoeffRefsSrc(srcCoeff) &&
            (BlendCoeff::kOne == dstCoeff || BlendCoeff::kISC == dstCoeff ||
             BlendCoeff::kISA == dstCoeff));
}

/**
 * Returns the name of the SkSL built-in blend function for a SkBlendMode.
 */
const char* BlendFuncName(SkBlendMode mode);

/**
 * If a blend can be represented by `blend_porter_duff`, returns the associated blend constants as
 * an array of four floats. If not, returns an empty span.
 */
SkSpan<const float> GetPorterDuffBlendConstants(SkBlendMode mode);

/**
 * Returns a pair of "blend function + uniform data" for a particular SkBlendMode.
 * This allows us to use fewer unique functions when generating shaders, e.g. every Porter-Duff
 * blend can use the same function.
 */
struct ReducedBlendModeInfo {
    const char*         fFunction;
    SkSpan<const float> fUniformData;
};
ReducedBlendModeInfo GetReducedBlendModeInfo(SkBlendMode mode);

} // namespace skgpu

#endif // skgpu_Blend_DEFINED
