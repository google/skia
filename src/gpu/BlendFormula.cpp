/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/BlendFormula.h"

#include "include/core/SkBlendMode.h"

namespace {

using skgpu::BlendFormula;

/**
 * When there is no coverage, or the blend mode can tweak alpha for coverage, we use the standard
 * Porter Duff formula.
 */
constexpr BlendFormula MakeCoeffFormula(skgpu::BlendCoeff srcCoeff, skgpu::BlendCoeff dstCoeff) {
    // When the coeffs are (Zero, Zero) or (Zero, One) we set the primary output to none.
    return (skgpu::BlendCoeff::kZero == srcCoeff &&
            (skgpu::BlendCoeff::kZero == dstCoeff || skgpu::BlendCoeff::kOne == dstCoeff))
           ? BlendFormula(BlendFormula::kNone_OutputType, BlendFormula::kNone_OutputType,
                          skgpu::BlendEquation::kAdd, skgpu::BlendCoeff::kZero, dstCoeff)
           : BlendFormula(BlendFormula::kModulate_OutputType, BlendFormula::kNone_OutputType,
                          skgpu::BlendEquation::kAdd, srcCoeff, dstCoeff);
}

/**
 * Basic coeff formula similar to MakeCoeffFormula but we will make the src f*Sa. This is used in
 * LCD dst-out.
 */
constexpr BlendFormula MakeSAModulateFormula(skgpu::BlendCoeff srcCoeff,
                                             skgpu::BlendCoeff dstCoeff) {
    return BlendFormula(BlendFormula::kSAModulate_OutputType, BlendFormula::kNone_OutputType,
                        skgpu::BlendEquation::kAdd, srcCoeff, dstCoeff);
}

/**
 * When there is coverage, the equation with f=coverage is:
 *
 *   D' = f * (S * srcCoeff + D * dstCoeff) + (1-f) * D
 *
 * This can be rewritten as:
 *
 *   D' = f * S * srcCoeff + D * (1 - [f * (1 - dstCoeff)])
 *
 * To implement this formula, we output [f * (1 - dstCoeff)] for the secondary color and replace the
 * HW dst coeff with IS2C.
 *
 * Xfer modes: dst-atop (Sa!=1)
 */
constexpr BlendFormula MakeCoverageFormula(BlendFormula::OutputType oneMinusDstCoeffModulateOutput,
                                           skgpu::BlendCoeff srcCoeff) {
    return BlendFormula(BlendFormula::kModulate_OutputType, oneMinusDstCoeffModulateOutput,
                        skgpu::BlendEquation::kAdd, srcCoeff, skgpu::BlendCoeff::kIS2C);
}

/**
 * When there is coverage and the src coeff is Zero, the equation with f=coverage becomes:
 *
 *   D' = f * D * dstCoeff + (1-f) * D
 *
 * This can be rewritten as:
 *
 *   D' = D - D * [f * (1 - dstCoeff)]
 *
 * To implement this formula, we output [f * (1 - dstCoeff)] for the primary color and use a reverse
 * subtract HW blend equation with coeffs of (DC, One).
 *
 * Xfer modes: clear, dst-out (Sa=1), dst-in (Sa!=1), modulate (Sc!=1)
 */
constexpr BlendFormula MakeCoverageSrcCoeffZeroFormula(
        BlendFormula::OutputType oneMinusDstCoeffModulateOutput) {
    return BlendFormula(oneMinusDstCoeffModulateOutput, BlendFormula::kNone_OutputType,
                        skgpu::BlendEquation::kReverseSubtract, skgpu::BlendCoeff::kDC,
                        skgpu::BlendCoeff::kOne);
}

/**
 * When there is coverage and the dst coeff is Zero, the equation with f=coverage becomes:
 *
 *   D' = f * S * srcCoeff + (1-f) * D
 *
 * To implement this formula, we output [f] for the secondary color and replace the HW dst coeff
 * with IS2A. (Note that we can avoid dual source blending when Sa=1 by using ISA.)
 *
 * Xfer modes (Sa!=1): src, src-in, src-out
 */
constexpr BlendFormula MakeCoverageDstCoeffZeroFormula(skgpu::BlendCoeff srcCoeff) {
    return BlendFormula(BlendFormula::kModulate_OutputType, BlendFormula::kCoverage_OutputType,
                        skgpu::BlendEquation::kAdd, srcCoeff, skgpu::BlendCoeff::kIS2A);
}

/**
 * This table outlines the blend formulas we will use with each xfermode, with and without coverage,
 * with and without an opaque input color. Optimization properties are deduced at compile time so we
 * can make runtime decisions quickly. RGB coverage is not supported.
 */
constexpr BlendFormula gBlendTable[2][2][(int)SkBlendMode::kLastCoeffMode + 1] = {
                     /*>> No coverage, input color unknown <<*/ {{

    /* clear */      MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kZero),
    /* src */        MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kZero),
    /* dst */        MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kOne),
    /* src-over */   MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISA),
    /* dst-over */   MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kOne),
    /* src-in */     MakeCoeffFormula(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kZero),
    /* dst-in */     MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSA),
    /* src-out */    MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kZero),
    /* dst-out */    MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kISA),
    /* src-atop */   MakeCoeffFormula(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kISA),
    /* dst-atop */   MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kSA),
    /* xor */        MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kISA),
    /* plus */       MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kOne),
    /* modulate */   MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSC),
    /* screen */     MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISC),

                     }, /*>> Has coverage, input color unknown <<*/ {

    /* clear */      MakeCoverageSrcCoeffZeroFormula(BlendFormula::kCoverage_OutputType),
    /* src */        MakeCoverageDstCoeffZeroFormula(skgpu::BlendCoeff::kOne),
    /* dst */        MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kOne),
    /* src-over */   MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISA),
    /* dst-over */   MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kOne),
    /* src-in */     MakeCoverageDstCoeffZeroFormula(skgpu::BlendCoeff::kDA),
    /* dst-in */     MakeCoverageSrcCoeffZeroFormula(BlendFormula::kISAModulate_OutputType),
    /* src-out */    MakeCoverageDstCoeffZeroFormula(skgpu::BlendCoeff::kIDA),
    /* dst-out */    MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kISA),
    /* src-atop */   MakeCoeffFormula(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kISA),
    /* dst-atop */   MakeCoverageFormula(BlendFormula::kISAModulate_OutputType,
                                         skgpu::BlendCoeff::kIDA),
    /* xor */        MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kISA),
    /* plus */       MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kOne),
    /* modulate */   MakeCoverageSrcCoeffZeroFormula(BlendFormula::kISCModulate_OutputType),
    /* screen */     MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISC),

                     }}, /*>> No coverage, input color opaque <<*/ {{

    /* clear */      MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kZero),
    /* src */        MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kZero),
    /* dst */        MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kOne),
    /* src-over */   MakeCoeffFormula(skgpu::BlendCoeff::kOne,
                                      skgpu::BlendCoeff::kISA), // see comment below
    /* dst-over */   MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kOne),
    /* src-in */     MakeCoeffFormula(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kZero),
    /* dst-in */     MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kOne),
    /* src-out */    MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kZero),
    /* dst-out */    MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kZero),
    /* src-atop */   MakeCoeffFormula(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kZero),
    /* dst-atop */   MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kOne),
    /* xor */        MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kZero),
    /* plus */       MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kOne),
    /* modulate */   MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSC),
    /* screen */     MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISC),

                     }, /*>> Has coverage, input color opaque <<*/ {

    /* clear */      MakeCoverageSrcCoeffZeroFormula(BlendFormula::kCoverage_OutputType),
    /* src */        MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISA),
    /* dst */        MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kOne),
    /* src-over */   MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISA),
    /* dst-over */   MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kOne),
    /* src-in */     MakeCoeffFormula(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kISA),
    /* dst-in */     MakeCoeffFormula(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kOne),
    /* src-out */    MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kISA),
    /* dst-out */    MakeCoverageSrcCoeffZeroFormula(BlendFormula::kCoverage_OutputType),
    /* src-atop */   MakeCoeffFormula(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kISA),
    /* dst-atop */   MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kOne),
    /* xor */        MakeCoeffFormula(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kISA),
    /* plus */       MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kOne),
    /* modulate */   MakeCoverageSrcCoeffZeroFormula(BlendFormula::kISCModulate_OutputType),
    /* screen */     MakeCoeffFormula(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISC),
}}};
// In the above table src-over is not optimized to src mode when the color is opaque because we
// found no advantage to doing so. Also, we are using a global src-over blend in most cases which is
// not specialized for opaque input. For GPUs where dropping to src (and thus able to disable
// blending) is an advantage we change the blend mode to src before getting the blend formula from
// this table.

constexpr BlendFormula gLCDBlendTable[(int)SkBlendMode::kLastCoeffMode + 1] = {
    /* clear */      MakeCoverageSrcCoeffZeroFormula(BlendFormula::kCoverage_OutputType),
    /* src */        MakeCoverageFormula(BlendFormula::kCoverage_OutputType,
                                         skgpu::BlendCoeff::kOne),
    /* dst */        MakeCoeffFormula(skgpu::BlendCoeff::kZero,
                                      skgpu::BlendCoeff::kOne),
    /* src-over */   MakeCoverageFormula(BlendFormula::kSAModulate_OutputType,
                                         skgpu::BlendCoeff::kOne),
    /* dst-over */   MakeCoeffFormula(skgpu::BlendCoeff::kIDA,
                                      skgpu::BlendCoeff::kOne),
    /* src-in */     MakeCoverageFormula(BlendFormula::kCoverage_OutputType,
                                         skgpu::BlendCoeff::kDA),
    /* dst-in */     MakeCoverageSrcCoeffZeroFormula(BlendFormula::kISAModulate_OutputType),
    /* src-out */    MakeCoverageFormula(BlendFormula::kCoverage_OutputType,
                                         skgpu::BlendCoeff::kIDA),
    /* dst-out */    MakeSAModulateFormula(skgpu::BlendCoeff::kZero,
                                           skgpu::BlendCoeff::kISC),
    /* src-atop */   MakeCoverageFormula(BlendFormula::kSAModulate_OutputType,
                                         skgpu::BlendCoeff::kDA),
    /* dst-atop */   MakeCoverageFormula(BlendFormula::kISAModulate_OutputType,
                                         skgpu::BlendCoeff::kIDA),
    /* xor */        MakeCoverageFormula(BlendFormula::kSAModulate_OutputType,
                                         skgpu::BlendCoeff::kIDA),
    /* plus */       MakeCoeffFormula(skgpu::BlendCoeff::kOne,
                                      skgpu::BlendCoeff::kOne),
    /* modulate */   MakeCoverageSrcCoeffZeroFormula(BlendFormula::kISCModulate_OutputType),
    /* screen */     MakeCoeffFormula(skgpu::BlendCoeff::kOne,
                                      skgpu::BlendCoeff::kISC),
};

}  // anonymous namespace

namespace skgpu {

BlendFormula GetBlendFormula(bool isOpaque, bool hasCoverage, SkBlendMode xfermode) {
    SkASSERT((unsigned)xfermode <= (unsigned)SkBlendMode::kLastCoeffMode);
    return gBlendTable[isOpaque][hasCoverage][(int)xfermode];
}

BlendFormula GetLCDBlendFormula(SkBlendMode xfermode) {
    SkASSERT((unsigned)xfermode <= (unsigned)SkBlendMode::kLastCoeffMode);
    return gLCDBlendTable[(int)xfermode];
}

}  // namespace skgpu
