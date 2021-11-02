/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrPorterDuffXferProcessor.h"

#include "include/gpu/GrTypes.h"
#include "include/private/SkMacros.h"
#include "include/private/SkTo.h"
#include "src/gpu/GrBlend.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProcessorAnalysis.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/glsl/GrGLSLBlend.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

/**
 * Wraps the shader outputs and HW blend state that comprise a Porter Duff blend mode with coverage.
 */
class BlendFormula {
public:
    /**
     * Values the shader can write to primary and secondary outputs. These are all modulated by
     * coverage. The XP will ignore the multiplies when not using coverage.
     */
    enum OutputType {
        kNone_OutputType,        //<! 0
        kCoverage_OutputType,    //<! inputCoverage
        kModulate_OutputType,    //<! inputColor * inputCoverage
        kSAModulate_OutputType,  //<! inputColor.a * inputCoverage
        kISAModulate_OutputType, //<! (1 - inputColor.a) * inputCoverage
        kISCModulate_OutputType, //<! (1 - inputColor) * inputCoverage

        kLast_OutputType = kISCModulate_OutputType
    };

    constexpr BlendFormula(OutputType primaryOut, OutputType secondaryOut, GrBlendEquation equation,
                           GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff)
            : fPrimaryOutputType(primaryOut)
            , fSecondaryOutputType(secondaryOut)
            , fBlendEquation(equation)
            , fSrcCoeff(srcCoeff)
            , fDstCoeff(dstCoeff)
            , fProps(GetProperties(primaryOut, secondaryOut, equation, srcCoeff, dstCoeff)) {}

    BlendFormula(const BlendFormula&) = default;
    BlendFormula& operator=(const BlendFormula&) = default;

    bool operator==(const BlendFormula& that) const {
        return fPrimaryOutputType == that.fPrimaryOutputType &&
               fSecondaryOutputType == that. fSecondaryOutputType &&
               fBlendEquation == that.fBlendEquation &&
               fSrcCoeff == that.fSrcCoeff &&
               fDstCoeff == that.fDstCoeff &&
               fProps == that.fProps;
    }

    bool hasSecondaryOutput() const {
        return kNone_OutputType != fSecondaryOutputType;
    }
    bool modifiesDst() const {
        return SkToBool(fProps & kModifiesDst_Property);
    }
    bool unaffectedByDst() const {
        return SkToBool(fProps & kUnaffectedByDst_Property);
    }
    // We don't always fully optimize the blend formula (e.g., for opaque src-over), so we include
    // an "IfOpaque" variant to help set AnalysisProperties::kUnaffectedByDstValue in those cases.
    bool unaffectedByDstIfOpaque() const {
        return SkToBool(fProps & kUnaffectedByDstIfOpaque_Property);
    }
    bool usesInputColor() const {
        return SkToBool(fProps & kUsesInputColor_Property);
    }
    bool canTweakAlphaForCoverage() const {
        return SkToBool(fProps & kCanTweakAlphaForCoverage_Property);
    }

    GrBlendEquation equation() const {
        return fBlendEquation;
    }

    GrBlendCoeff srcCoeff() const {
        return fSrcCoeff;
    }

    GrBlendCoeff dstCoeff() const {
        return fDstCoeff;
    }

    OutputType primaryOutput() const {
        return fPrimaryOutputType;
    }

    OutputType secondaryOutput() const {
        return fSecondaryOutputType;
    }

private:
    enum Properties {
        kModifiesDst_Property              = 1 << 0,
        kUnaffectedByDst_Property          = 1 << 1,
        kUnaffectedByDstIfOpaque_Property  = 1 << 2,
        kUsesInputColor_Property           = 1 << 3,
        kCanTweakAlphaForCoverage_Property = 1 << 4,

        kLast_Property = kCanTweakAlphaForCoverage_Property
    };
    SK_DECL_BITFIELD_OPS_FRIENDS(Properties)

    /**
     * Deduce the properties of a BlendFormula.
     */
    static constexpr Properties GetProperties(OutputType PrimaryOut, OutputType SecondaryOut,
                                              GrBlendEquation BlendEquation, GrBlendCoeff SrcCoeff,
                                              GrBlendCoeff DstCoeff);

    struct {
        // We allot the enums one more bit than they require because MSVC seems to sign-extend
        // them when the top bit is set. (This is in violation of the C++03 standard 9.6/4)
        OutputType        fPrimaryOutputType    : 4;
        OutputType        fSecondaryOutputType  : 4;
        GrBlendEquation   fBlendEquation        : 6;
        GrBlendCoeff      fSrcCoeff             : 6;
        GrBlendCoeff      fDstCoeff             : 6;
        Properties        fProps                : 32 - (4 + 4 + 6 + 6 + 6);
    };

    static_assert(kLast_OutputType      < (1 << 3));
    static_assert(kLast_GrBlendEquation < (1 << 5));
    static_assert(kLast_GrBlendCoeff    < (1 << 5));
    static_assert(kLast_Property        < (1 << 6));
};

static_assert(4 == sizeof(BlendFormula));

SK_MAKE_BITFIELD_OPS(BlendFormula::Properties)

constexpr BlendFormula::Properties BlendFormula::GetProperties(OutputType PrimaryOut,
                                                               OutputType SecondaryOut,
                                                               GrBlendEquation BlendEquation,
                                                               GrBlendCoeff SrcCoeff,
                                                               GrBlendCoeff DstCoeff) {
    return
    // The provided formula should already be optimized before a BlendFormula is constructed.
    // Assert that here while setting up the properties in the constexpr constructor.
    SkASSERT((kNone_OutputType == PrimaryOut) == !GrBlendCoeffsUseSrcColor(SrcCoeff, DstCoeff)),
    SkASSERT(!GrBlendCoeffRefsSrc2(SrcCoeff)),
    SkASSERT((kNone_OutputType == SecondaryOut) == !GrBlendCoeffRefsSrc2(DstCoeff)),
    SkASSERT(PrimaryOut != SecondaryOut || kNone_OutputType == PrimaryOut),
    SkASSERT(kNone_OutputType != PrimaryOut || kNone_OutputType == SecondaryOut),

    static_cast<Properties>(
        (GrBlendModifiesDst(BlendEquation, SrcCoeff, DstCoeff) ? kModifiesDst_Property : 0) |
        (!GrBlendCoeffsUseDstColor(SrcCoeff, DstCoeff, false/*srcColorIsOpaque*/)
                    ? kUnaffectedByDst_Property
                    : 0) |
        (!GrBlendCoeffsUseDstColor(SrcCoeff, DstCoeff, true/*srcColorIsOpaque*/)
                    ? kUnaffectedByDstIfOpaque_Property
                    : 0) |
        ((PrimaryOut >= kModulate_OutputType && GrBlendCoeffsUseSrcColor(SrcCoeff, DstCoeff)) ||
                            (SecondaryOut >= kModulate_OutputType &&
                            GrBlendCoeffRefsSrc2(DstCoeff))
                    ? kUsesInputColor_Property
                    : 0) |  // We assert later that SrcCoeff doesn't ref src2.
        ((kModulate_OutputType == PrimaryOut || kNone_OutputType == PrimaryOut) &&
                            kNone_OutputType == SecondaryOut &&
                            GrBlendAllowsCoverageAsAlpha(BlendEquation, SrcCoeff, DstCoeff)
                    ? kCanTweakAlphaForCoverage_Property
                    : 0));
}

/**
 * When there is no coverage, or the blend mode can tweak alpha for coverage, we use the standard
 * Porter Duff formula.
 */
static constexpr BlendFormula MakeCoeffFormula(GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff) {
    // When the coeffs are (Zero, Zero) or (Zero, One) we set the primary output to none.
    return (kZero_GrBlendCoeff == srcCoeff &&
            (kZero_GrBlendCoeff == dstCoeff || kOne_GrBlendCoeff == dstCoeff))
           ? BlendFormula(BlendFormula::kNone_OutputType, BlendFormula::kNone_OutputType,
                          kAdd_GrBlendEquation, kZero_GrBlendCoeff, dstCoeff)
           : BlendFormula(BlendFormula::kModulate_OutputType, BlendFormula::kNone_OutputType,
                        kAdd_GrBlendEquation, srcCoeff, dstCoeff);
}

/**
 * Basic coeff formula similar to MakeCoeffFormula but we will make the src f*Sa. This is used in
 * LCD dst-out.
 */
static constexpr BlendFormula MakeSAModulateFormula(GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff) {
    return BlendFormula(BlendFormula::kSAModulate_OutputType, BlendFormula::kNone_OutputType,
                        kAdd_GrBlendEquation, srcCoeff, dstCoeff);
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
static constexpr BlendFormula MakeCoverageFormula(
        BlendFormula::OutputType oneMinusDstCoeffModulateOutput, GrBlendCoeff srcCoeff) {
    return BlendFormula(BlendFormula::kModulate_OutputType, oneMinusDstCoeffModulateOutput,
                        kAdd_GrBlendEquation, srcCoeff, kIS2C_GrBlendCoeff);
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
static constexpr BlendFormula MakeCoverageSrcCoeffZeroFormula(
        BlendFormula::OutputType oneMinusDstCoeffModulateOutput) {
    return BlendFormula(oneMinusDstCoeffModulateOutput, BlendFormula::kNone_OutputType,
                        kReverseSubtract_GrBlendEquation, kDC_GrBlendCoeff, kOne_GrBlendCoeff);
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
static constexpr BlendFormula MakeCoverageDstCoeffZeroFormula(GrBlendCoeff srcCoeff) {
    return BlendFormula(BlendFormula::kModulate_OutputType, BlendFormula::kCoverage_OutputType,
                        kAdd_GrBlendEquation, srcCoeff, kIS2A_GrBlendCoeff);
}

/**
 * This table outlines the blend formulas we will use with each xfermode, with and without coverage,
 * with and without an opaque input color. Optimization properties are deduced at compile time so we
 * can make runtime decisions quickly. RGB coverage is not supported.
 */
static constexpr BlendFormula gBlendTable[2][2][(int)SkBlendMode::kLastCoeffMode + 1] = {
                     /*>> No coverage, input color unknown <<*/ {{

    /* clear */      MakeCoeffFormula(kZero_GrBlendCoeff, kZero_GrBlendCoeff),
    /* src */        MakeCoeffFormula(kOne_GrBlendCoeff,  kZero_GrBlendCoeff),
    /* dst */        MakeCoeffFormula(kZero_GrBlendCoeff, kOne_GrBlendCoeff),
    /* src-over */   MakeCoeffFormula(kOne_GrBlendCoeff,  kISA_GrBlendCoeff),
    /* dst-over */   MakeCoeffFormula(kIDA_GrBlendCoeff,  kOne_GrBlendCoeff),
    /* src-in */     MakeCoeffFormula(kDA_GrBlendCoeff,   kZero_GrBlendCoeff),
    /* dst-in */     MakeCoeffFormula(kZero_GrBlendCoeff, kSA_GrBlendCoeff),
    /* src-out */    MakeCoeffFormula(kIDA_GrBlendCoeff,  kZero_GrBlendCoeff),
    /* dst-out */    MakeCoeffFormula(kZero_GrBlendCoeff, kISA_GrBlendCoeff),
    /* src-atop */   MakeCoeffFormula(kDA_GrBlendCoeff,   kISA_GrBlendCoeff),
    /* dst-atop */   MakeCoeffFormula(kIDA_GrBlendCoeff,  kSA_GrBlendCoeff),
    /* xor */        MakeCoeffFormula(kIDA_GrBlendCoeff,  kISA_GrBlendCoeff),
    /* plus */       MakeCoeffFormula(kOne_GrBlendCoeff,  kOne_GrBlendCoeff),
    /* modulate */   MakeCoeffFormula(kZero_GrBlendCoeff, kSC_GrBlendCoeff),
    /* screen */     MakeCoeffFormula(kOne_GrBlendCoeff,  kISC_GrBlendCoeff),

                     }, /*>> Has coverage, input color unknown <<*/ {

    /* clear */      MakeCoverageSrcCoeffZeroFormula(BlendFormula::kCoverage_OutputType),
    /* src */        MakeCoverageDstCoeffZeroFormula(kOne_GrBlendCoeff),
    /* dst */        MakeCoeffFormula(kZero_GrBlendCoeff, kOne_GrBlendCoeff),
    /* src-over */   MakeCoeffFormula(kOne_GrBlendCoeff,  kISA_GrBlendCoeff),
    /* dst-over */   MakeCoeffFormula(kIDA_GrBlendCoeff,  kOne_GrBlendCoeff),
    /* src-in */     MakeCoverageDstCoeffZeroFormula(kDA_GrBlendCoeff),
    /* dst-in */     MakeCoverageSrcCoeffZeroFormula(BlendFormula::kISAModulate_OutputType),
    /* src-out */    MakeCoverageDstCoeffZeroFormula(kIDA_GrBlendCoeff),
    /* dst-out */    MakeCoeffFormula(kZero_GrBlendCoeff, kISA_GrBlendCoeff),
    /* src-atop */   MakeCoeffFormula(kDA_GrBlendCoeff,   kISA_GrBlendCoeff),
    /* dst-atop */   MakeCoverageFormula(BlendFormula::kISAModulate_OutputType, kIDA_GrBlendCoeff),
    /* xor */        MakeCoeffFormula(kIDA_GrBlendCoeff,  kISA_GrBlendCoeff),
    /* plus */       MakeCoeffFormula(kOne_GrBlendCoeff,  kOne_GrBlendCoeff),
    /* modulate */   MakeCoverageSrcCoeffZeroFormula(BlendFormula::kISCModulate_OutputType),
    /* screen */     MakeCoeffFormula(kOne_GrBlendCoeff,  kISC_GrBlendCoeff),

                     }}, /*>> No coverage, input color opaque <<*/ {{

    /* clear */      MakeCoeffFormula(kZero_GrBlendCoeff, kZero_GrBlendCoeff),
    /* src */        MakeCoeffFormula(kOne_GrBlendCoeff,  kZero_GrBlendCoeff),
    /* dst */        MakeCoeffFormula(kZero_GrBlendCoeff, kOne_GrBlendCoeff),
    /* src-over */   MakeCoeffFormula(kOne_GrBlendCoeff,  kISA_GrBlendCoeff), // see comment below
    /* dst-over */   MakeCoeffFormula(kIDA_GrBlendCoeff,  kOne_GrBlendCoeff),
    /* src-in */     MakeCoeffFormula(kDA_GrBlendCoeff,   kZero_GrBlendCoeff),
    /* dst-in */     MakeCoeffFormula(kZero_GrBlendCoeff, kOne_GrBlendCoeff),
    /* src-out */    MakeCoeffFormula(kIDA_GrBlendCoeff,  kZero_GrBlendCoeff),
    /* dst-out */    MakeCoeffFormula(kZero_GrBlendCoeff, kZero_GrBlendCoeff),
    /* src-atop */   MakeCoeffFormula(kDA_GrBlendCoeff,   kZero_GrBlendCoeff),
    /* dst-atop */   MakeCoeffFormula(kIDA_GrBlendCoeff,  kOne_GrBlendCoeff),
    /* xor */        MakeCoeffFormula(kIDA_GrBlendCoeff,  kZero_GrBlendCoeff),
    /* plus */       MakeCoeffFormula(kOne_GrBlendCoeff,  kOne_GrBlendCoeff),
    /* modulate */   MakeCoeffFormula(kZero_GrBlendCoeff, kSC_GrBlendCoeff),
    /* screen */     MakeCoeffFormula(kOne_GrBlendCoeff,  kISC_GrBlendCoeff),

                     }, /*>> Has coverage, input color opaque <<*/ {

    /* clear */      MakeCoverageSrcCoeffZeroFormula(BlendFormula::kCoverage_OutputType),
    /* src */        MakeCoeffFormula(kOne_GrBlendCoeff,  kISA_GrBlendCoeff),
    /* dst */        MakeCoeffFormula(kZero_GrBlendCoeff, kOne_GrBlendCoeff),
    /* src-over */   MakeCoeffFormula(kOne_GrBlendCoeff,  kISA_GrBlendCoeff),
    /* dst-over */   MakeCoeffFormula(kIDA_GrBlendCoeff,  kOne_GrBlendCoeff),
    /* src-in */     MakeCoeffFormula(kDA_GrBlendCoeff,   kISA_GrBlendCoeff),
    /* dst-in */     MakeCoeffFormula(kZero_GrBlendCoeff, kOne_GrBlendCoeff),
    /* src-out */    MakeCoeffFormula(kIDA_GrBlendCoeff,  kISA_GrBlendCoeff),
    /* dst-out */    MakeCoverageSrcCoeffZeroFormula(BlendFormula::kCoverage_OutputType),
    /* src-atop */   MakeCoeffFormula(kDA_GrBlendCoeff,   kISA_GrBlendCoeff),
    /* dst-atop */   MakeCoeffFormula(kIDA_GrBlendCoeff,  kOne_GrBlendCoeff),
    /* xor */        MakeCoeffFormula(kIDA_GrBlendCoeff,  kISA_GrBlendCoeff),
    /* plus */       MakeCoeffFormula(kOne_GrBlendCoeff,  kOne_GrBlendCoeff),
    /* modulate */   MakeCoverageSrcCoeffZeroFormula(BlendFormula::kISCModulate_OutputType),
    /* screen */     MakeCoeffFormula(kOne_GrBlendCoeff,  kISC_GrBlendCoeff),
}}};
// In the above table src-over is not optimized to src mode when the color is opaque because we
// found no advantage to doing so. Also, we are using a global src-over XP in most cases which is
// not specialized for opaque input. For GPUs where dropping to src (and thus able to disable
// blending) is an advantage we change the blend mode to src before getitng the blend formula from
// this table.
static constexpr BlendFormula gLCDBlendTable[(int)SkBlendMode::kLastCoeffMode + 1] = {
    /* clear */      MakeCoverageSrcCoeffZeroFormula(BlendFormula::kCoverage_OutputType),
    /* src */        MakeCoverageFormula(BlendFormula::kCoverage_OutputType, kOne_GrBlendCoeff),
    /* dst */        MakeCoeffFormula(kZero_GrBlendCoeff, kOne_GrBlendCoeff),
    /* src-over */   MakeCoverageFormula(BlendFormula::kSAModulate_OutputType, kOne_GrBlendCoeff),
    /* dst-over */   MakeCoeffFormula(kIDA_GrBlendCoeff, kOne_GrBlendCoeff),
    /* src-in */     MakeCoverageFormula(BlendFormula::kCoverage_OutputType, kDA_GrBlendCoeff),
    /* dst-in */     MakeCoverageSrcCoeffZeroFormula(BlendFormula::kISAModulate_OutputType),
    /* src-out */    MakeCoverageFormula(BlendFormula::kCoverage_OutputType, kIDA_GrBlendCoeff),
    /* dst-out */    MakeSAModulateFormula(kZero_GrBlendCoeff, kISC_GrBlendCoeff),
    /* src-atop */   MakeCoverageFormula(BlendFormula::kSAModulate_OutputType, kDA_GrBlendCoeff),
    /* dst-atop */   MakeCoverageFormula(BlendFormula::kISAModulate_OutputType, kIDA_GrBlendCoeff),
    /* xor */        MakeCoverageFormula(BlendFormula::kSAModulate_OutputType, kIDA_GrBlendCoeff),
    /* plus */       MakeCoeffFormula(kOne_GrBlendCoeff, kOne_GrBlendCoeff),
    /* modulate */   MakeCoverageSrcCoeffZeroFormula(BlendFormula::kISCModulate_OutputType),
    /* screen */     MakeCoeffFormula(kOne_GrBlendCoeff, kISC_GrBlendCoeff),
};

static BlendFormula get_blend_formula(bool isOpaque,
                                      bool hasCoverage,
                                      SkBlendMode xfermode) {
    SkASSERT((unsigned)xfermode <= (unsigned)SkBlendMode::kLastCoeffMode);
    return gBlendTable[isOpaque][hasCoverage][(int)xfermode];
}

static BlendFormula get_lcd_blend_formula(SkBlendMode xfermode) {
    SkASSERT((unsigned)xfermode <= (unsigned)SkBlendMode::kLastCoeffMode);

    return gLCDBlendTable[(int)xfermode];
}

///////////////////////////////////////////////////////////////////////////////

class PorterDuffXferProcessor : public GrXferProcessor {
public:
    PorterDuffXferProcessor(BlendFormula blendFormula, GrProcessorAnalysisCoverage coverage)
            : INHERITED(kPorterDuffXferProcessor_ClassID, /*willReadDstColor=*/false, coverage)
            , fBlendFormula(blendFormula) {
    }

    const char* name() const override { return "Porter Duff"; }

    std::unique_ptr<ProgramImpl> makeProgramImpl() const override;

    BlendFormula getBlendFormula() const { return fBlendFormula; }

private:
    void onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onHasSecondaryOutput() const override { return fBlendFormula.hasSecondaryOutput(); }

    void onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const override {
        blendInfo->fEquation = fBlendFormula.equation();
        blendInfo->fSrcBlend = fBlendFormula.srcCoeff();
        blendInfo->fDstBlend = fBlendFormula.dstCoeff();
        blendInfo->fWriteColor = fBlendFormula.modifiesDst();
    }

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const PorterDuffXferProcessor& xp = xpBase.cast<PorterDuffXferProcessor>();
        return fBlendFormula == xp.fBlendFormula;
    }

    const BlendFormula fBlendFormula;

    using INHERITED = GrXferProcessor;
};

///////////////////////////////////////////////////////////////////////////////

static void append_color_output(const PorterDuffXferProcessor& xp,
                                GrGLSLXPFragmentBuilder* fragBuilder,
                                BlendFormula::OutputType outputType, const char* output,
                                const char* inColor, const char* inCoverage) {
    SkASSERT(inCoverage);
    SkASSERT(inColor);
    switch (outputType) {
        case BlendFormula::kNone_OutputType:
            fragBuilder->codeAppendf("%s = half4(0.0);", output);
            break;
        case BlendFormula::kCoverage_OutputType:
            fragBuilder->codeAppendf("%s = %s;", output, inCoverage);
            break;
        case BlendFormula::kModulate_OutputType:
            fragBuilder->codeAppendf("%s = %s * %s;", output, inColor, inCoverage);
            break;
        case BlendFormula::kSAModulate_OutputType:
            fragBuilder->codeAppendf("%s = %s.a * %s;", output, inColor, inCoverage);
            break;
        case BlendFormula::kISAModulate_OutputType:
            fragBuilder->codeAppendf("%s = (1.0 - %s.a) * %s;", output, inColor, inCoverage);
            break;
        case BlendFormula::kISCModulate_OutputType:
            fragBuilder->codeAppendf("%s = (half4(1.0) - %s) * %s;", output, inColor, inCoverage);
            break;
        default:
            SK_ABORT("Unsupported output type.");
            break;
    }
}

void PorterDuffXferProcessor::onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const {
    b->add32(fBlendFormula.primaryOutput() | (fBlendFormula.secondaryOutput() << 3));
    static_assert(BlendFormula::kLast_OutputType < 8);
}

std::unique_ptr<GrXferProcessor::ProgramImpl> PorterDuffXferProcessor::makeProgramImpl() const {
    class Impl : public ProgramImpl {
    private:
        void emitOutputsForBlendState(const EmitArgs& args) override {
            const PorterDuffXferProcessor& xp = args.fXP.cast<PorterDuffXferProcessor>();
            GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;

            const BlendFormula& blendFormula = xp.fBlendFormula;
            if (blendFormula.hasSecondaryOutput()) {
                append_color_output(xp,
                                    fragBuilder,
                                    blendFormula.secondaryOutput(),
                                    args.fOutputSecondary,
                                    args.fInputColor,
                                    args.fInputCoverage);
            }
            append_color_output(xp,
                                fragBuilder,
                                blendFormula.primaryOutput(),
                                args.fOutputPrimary,
                                args.fInputColor,
                                args.fInputCoverage);
        }
    };

    return std::make_unique<Impl>();
}

///////////////////////////////////////////////////////////////////////////////

class ShaderPDXferProcessor : public GrXferProcessor {
public:
    ShaderPDXferProcessor(SkBlendMode xfermode, GrProcessorAnalysisCoverage coverage)
            : INHERITED(kShaderPDXferProcessor_ClassID, /*willReadDstColor=*/true, coverage)
            , fXfermode(xfermode) {
    }

    const char* name() const override { return "Porter Duff Shader"; }

    std::unique_ptr<ProgramImpl> makeProgramImpl() const override;

private:
    void onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const ShaderPDXferProcessor& xp = xpBase.cast<ShaderPDXferProcessor>();
        return fXfermode == xp.fXfermode;
    }

    const SkBlendMode fXfermode;

    using INHERITED = GrXferProcessor;
};

///////////////////////////////////////////////////////////////////////////////


void ShaderPDXferProcessor::onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const {
    b->add32(static_cast<int>(fXfermode));
}

std::unique_ptr<GrXferProcessor::ProgramImpl> ShaderPDXferProcessor::makeProgramImpl() const {
    class Impl : public ProgramImpl {
    private:
        void emitBlendCodeForDstRead(GrGLSLXPFragmentBuilder* fragBuilder,
                                     GrGLSLUniformHandler* uniformHandler,
                                     const char* srcColor,
                                     const char* srcCoverage,
                                     const char* dstColor,
                                     const char* outColor,
                                     const char* outColorSecondary,
                                     const GrXferProcessor& proc) override {
            const ShaderPDXferProcessor& xp = proc.cast<ShaderPDXferProcessor>();

            GrGLSLBlend::AppendMode(fragBuilder, srcColor, dstColor, outColor, xp.fXfermode);

            // Apply coverage.
            DefaultCoverageModulation(fragBuilder,
                                      srcCoverage,
                                      dstColor,
                                      outColor,
                                      outColorSecondary,
                                      xp);
        }
    };

    return std::make_unique<Impl>();
}

///////////////////////////////////////////////////////////////////////////////

class PDLCDXferProcessor : public GrXferProcessor {
public:
    static sk_sp<const GrXferProcessor> Make(SkBlendMode mode,
                                             const GrProcessorAnalysisColor& inputColor);

    const char* name() const override { return "Porter Duff LCD"; }

    std::unique_ptr<ProgramImpl> makeProgramImpl() const override;

private:
    PDLCDXferProcessor(const SkPMColor4f& blendConstant, float alpha);

    void onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

    void onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const override {
        blendInfo->fSrcBlend = kConstC_GrBlendCoeff;
        blendInfo->fDstBlend = kISC_GrBlendCoeff;
        blendInfo->fBlendConstant = fBlendConstant;
    }

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const PDLCDXferProcessor& xp = xpBase.cast<PDLCDXferProcessor>();
        if (fBlendConstant != xp.fBlendConstant || fAlpha != xp.fAlpha) {
            return false;
        }
        return true;
    }

    SkPMColor4f fBlendConstant;
    float fAlpha;

    using INHERITED = GrXferProcessor;
};

PDLCDXferProcessor::PDLCDXferProcessor(const SkPMColor4f& blendConstant, float alpha)
    : INHERITED(kPDLCDXferProcessor_ClassID, /*willReadDstColor=*/false,
                GrProcessorAnalysisCoverage::kLCD)
    , fBlendConstant(blendConstant)
    , fAlpha(alpha) {
}

sk_sp<const GrXferProcessor> PDLCDXferProcessor::Make(SkBlendMode mode,
                                                      const GrProcessorAnalysisColor& color) {
    if (SkBlendMode::kSrcOver != mode) {
        return nullptr;
    }
    SkPMColor4f blendConstantPM;
    if (!color.isConstant(&blendConstantPM)) {
        return nullptr;
    }
    SkColor4f blendConstantUPM = blendConstantPM.unpremul();
    float alpha = blendConstantUPM.fA;
    blendConstantPM = { blendConstantUPM.fR, blendConstantUPM.fG, blendConstantUPM.fB, 1 };
    return sk_sp<GrXferProcessor>(new PDLCDXferProcessor(blendConstantPM, alpha));
}

std::unique_ptr<GrXferProcessor::ProgramImpl> PDLCDXferProcessor::makeProgramImpl() const {
    class Impl : public ProgramImpl {
    private:
        void emitOutputsForBlendState(const EmitArgs& args) override {
            const char* alpha;
            fAlphaUniform = args.fUniformHandler->addUniform(nullptr,
                                                             kFragment_GrShaderFlag,
                                                             kHalf_GrSLType,
                                                             "alpha",
                                                             &alpha);
            GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;
            // We want to force our primary output to be alpha * Coverage, where alpha is the alpha
            // value of the src color. We know that there are no color stages (or we wouldn't have
            // created this xp) and the r,g, and b channels of the op's input color are baked into
            // the blend constant.
            SkASSERT(args.fInputCoverage);
            fragBuilder->codeAppendf("%s = %s * %s;",
                                     args.fOutputPrimary,
                                     alpha, args.fInputCoverage);
        }

        void onSetData(const GrGLSLProgramDataManager& pdm, const GrXferProcessor& xp) override {
            float alpha = xp.cast<PDLCDXferProcessor>().fAlpha;
            if (fLastAlpha != alpha) {
                pdm.set1f(fAlphaUniform, alpha);
                fLastAlpha = alpha;
            }
        }

        GrGLSLUniformHandler::UniformHandle fAlphaUniform;
        float fLastAlpha = SK_FloatNaN;
    };

    return std::make_unique<Impl>();
}

///////////////////////////////////////////////////////////////////////////////

constexpr GrPorterDuffXPFactory::GrPorterDuffXPFactory(SkBlendMode xfermode)
        : fBlendMode(xfermode) {}

const GrXPFactory* GrPorterDuffXPFactory::Get(SkBlendMode blendMode) {
    SkASSERT((unsigned)blendMode <= (unsigned)SkBlendMode::kLastCoeffMode);

    static constexpr const GrPorterDuffXPFactory gClearPDXPF(SkBlendMode::kClear);
    static constexpr const GrPorterDuffXPFactory gSrcPDXPF(SkBlendMode::kSrc);
    static constexpr const GrPorterDuffXPFactory gDstPDXPF(SkBlendMode::kDst);
    static constexpr const GrPorterDuffXPFactory gSrcOverPDXPF(SkBlendMode::kSrcOver);
    static constexpr const GrPorterDuffXPFactory gDstOverPDXPF(SkBlendMode::kDstOver);
    static constexpr const GrPorterDuffXPFactory gSrcInPDXPF(SkBlendMode::kSrcIn);
    static constexpr const GrPorterDuffXPFactory gDstInPDXPF(SkBlendMode::kDstIn);
    static constexpr const GrPorterDuffXPFactory gSrcOutPDXPF(SkBlendMode::kSrcOut);
    static constexpr const GrPorterDuffXPFactory gDstOutPDXPF(SkBlendMode::kDstOut);
    static constexpr const GrPorterDuffXPFactory gSrcATopPDXPF(SkBlendMode::kSrcATop);
    static constexpr const GrPorterDuffXPFactory gDstATopPDXPF(SkBlendMode::kDstATop);
    static constexpr const GrPorterDuffXPFactory gXorPDXPF(SkBlendMode::kXor);
    static constexpr const GrPorterDuffXPFactory gPlusPDXPF(SkBlendMode::kPlus);
    static constexpr const GrPorterDuffXPFactory gModulatePDXPF(SkBlendMode::kModulate);
    static constexpr const GrPorterDuffXPFactory gScreenPDXPF(SkBlendMode::kScreen);

    switch (blendMode) {
        case SkBlendMode::kClear:
            return &gClearPDXPF;
        case SkBlendMode::kSrc:
            return &gSrcPDXPF;
        case SkBlendMode::kDst:
            return &gDstPDXPF;
        case SkBlendMode::kSrcOver:
            return &gSrcOverPDXPF;
        case SkBlendMode::kDstOver:
            return &gDstOverPDXPF;
        case SkBlendMode::kSrcIn:
            return &gSrcInPDXPF;
        case SkBlendMode::kDstIn:
            return &gDstInPDXPF;
        case SkBlendMode::kSrcOut:
            return &gSrcOutPDXPF;
        case SkBlendMode::kDstOut:
            return &gDstOutPDXPF;
        case SkBlendMode::kSrcATop:
            return &gSrcATopPDXPF;
        case SkBlendMode::kDstATop:
            return &gDstATopPDXPF;
        case SkBlendMode::kXor:
            return &gXorPDXPF;
        case SkBlendMode::kPlus:
            return &gPlusPDXPF;
        case SkBlendMode::kModulate:
            return &gModulatePDXPF;
        case SkBlendMode::kScreen:
            return &gScreenPDXPF;
        default:
            SK_ABORT("Unexpected blend mode.");
    }
}

sk_sp<const GrXferProcessor> GrPorterDuffXPFactory::makeXferProcessor(
        const GrProcessorAnalysisColor& color, GrProcessorAnalysisCoverage coverage,
        const GrCaps& caps, GrClampType clampType) const {
    bool isLCD = coverage == GrProcessorAnalysisCoverage::kLCD;
    // See comment in MakeSrcOverXferProcessor about color.isOpaque here
    if (isLCD &&
        SkBlendMode::kSrcOver == fBlendMode && color.isConstant() && /*color.isOpaque() &&*/
        !caps.shaderCaps()->dualSourceBlendingSupport() &&
        !caps.shaderCaps()->dstReadInShaderSupport()) {
        // If we don't have dual source blending or in shader dst reads, we fall back to this
        // trick for rendering SrcOver LCD text instead of doing a dst copy.
        return PDLCDXferProcessor::Make(fBlendMode, color);
    }
    BlendFormula blendFormula = [&](){
        if (isLCD) {
            return get_lcd_blend_formula(fBlendMode);
        }
        if (fBlendMode == SkBlendMode::kSrcOver && color.isOpaque() &&
            coverage == GrProcessorAnalysisCoverage::kNone &&
            caps.shouldCollapseSrcOverToSrcWhenAble())
        {
            return get_blend_formula(true, false, SkBlendMode::kSrc);
        }
        return get_blend_formula(color.isOpaque(), GrProcessorAnalysisCoverage::kNone != coverage,
                                 fBlendMode);
    }();

    // Skia always saturates after the kPlus blend mode, so it requires shader-based blending when
    // pixels aren't guaranteed to automatically be normalized (i.e. any floating point config).
    if ((blendFormula.hasSecondaryOutput() && !caps.shaderCaps()->dualSourceBlendingSupport()) ||
        (isLCD && (SkBlendMode::kSrcOver != fBlendMode /*|| !color.isOpaque()*/)) ||
        (GrClampType::kAuto != clampType && SkBlendMode::kPlus == fBlendMode)) {
        return sk_sp<const GrXferProcessor>(new ShaderPDXferProcessor(fBlendMode, coverage));
    }
    return sk_sp<const GrXferProcessor>(new PorterDuffXferProcessor(blendFormula, coverage));
}

static inline GrXPFactory::AnalysisProperties analysis_properties(
        const GrProcessorAnalysisColor& color, const GrProcessorAnalysisCoverage& coverage,
        const GrCaps& caps, GrClampType clampType, SkBlendMode mode) {
    using AnalysisProperties = GrXPFactory::AnalysisProperties;
    AnalysisProperties props = AnalysisProperties::kNone;
    bool hasCoverage = GrProcessorAnalysisCoverage::kNone != coverage;
    bool isLCD = GrProcessorAnalysisCoverage::kLCD == coverage;
    BlendFormula formula = [&](){
        if (isLCD) {
            return gLCDBlendTable[(int)mode];
        }
        return get_blend_formula(color.isOpaque(), hasCoverage, mode);
    }();

    if (formula.canTweakAlphaForCoverage() && !isLCD) {
        props |= AnalysisProperties::kCompatibleWithCoverageAsAlpha;
    }

    if (isLCD) {
        // See comment in MakeSrcOverXferProcessor about color.isOpaque here
        if (SkBlendMode::kSrcOver == mode && color.isConstant() && /*color.isOpaque() &&*/
            !caps.shaderCaps()->dualSourceBlendingSupport() &&
            !caps.shaderCaps()->dstReadInShaderSupport()) {
            props |= AnalysisProperties::kIgnoresInputColor;
        } else {
            // For LCD blending, if the color is not opaque we must read the dst in shader even if
            // we have dual source blending. The opaqueness check must be done after blending so for
            // simplicity we only allow src-over to not take the dst read path (though src, src-in,
            // and DstATop would also work). We also fall into the dst read case for src-over if we
            // do not have dual source blending.
            if (SkBlendMode::kSrcOver != mode ||
                /*!color.isOpaque() ||*/ // See comment in MakeSrcOverXferProcessor about isOpaque.
                (formula.hasSecondaryOutput() && !caps.shaderCaps()->dualSourceBlendingSupport())) {
                props |= AnalysisProperties::kReadsDstInShader;
            }
        }
    } else {
        // With dual-source blending we never need the destination color in the shader.
        if (!caps.shaderCaps()->dualSourceBlendingSupport()) {
            if (formula.hasSecondaryOutput()) {
                props |= AnalysisProperties::kReadsDstInShader;
            }
        }
    }

    if (GrClampType::kAuto != clampType && SkBlendMode::kPlus == mode) {
        props |= AnalysisProperties::kReadsDstInShader;
    }

    if (!formula.modifiesDst() || !formula.usesInputColor()) {
        props |= AnalysisProperties::kIgnoresInputColor;
    }
    if (formula.unaffectedByDst() || (formula.unaffectedByDstIfOpaque() && color.isOpaque() &&
                                      !hasCoverage)) {
        props |= AnalysisProperties::kUnaffectedByDstValue;
    }
    return props;
}

GrXPFactory::AnalysisProperties GrPorterDuffXPFactory::analysisProperties(
        const GrProcessorAnalysisColor& color,
        const GrProcessorAnalysisCoverage& coverage,
        const GrCaps& caps,
        GrClampType clampType) const {
    return analysis_properties(color, coverage, caps, clampType, fBlendMode);
}

GR_DEFINE_XP_FACTORY_TEST(GrPorterDuffXPFactory);

#if GR_TEST_UTILS
const GrXPFactory* GrPorterDuffXPFactory::TestGet(GrProcessorTestData* d) {
    SkBlendMode mode = SkBlendMode(d->fRandom->nextULessThan((int)SkBlendMode::kLastCoeffMode));
    return GrPorterDuffXPFactory::Get(mode);
}
#endif

void GrPorterDuffXPFactory::TestGetXPOutputTypes(const GrXferProcessor* xp,
                                                 int* outPrimary,
                                                 int* outSecondary) {
    if (!!strcmp(xp->name(), "Porter Duff")) {
        *outPrimary = *outSecondary = -1;
        return;
    }
    BlendFormula blendFormula = static_cast<const PorterDuffXferProcessor*>(xp)->getBlendFormula();
    *outPrimary = blendFormula.primaryOutput();
    *outSecondary = blendFormula.secondaryOutput();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// SrcOver Global functions
////////////////////////////////////////////////////////////////////////////////////////////////
const GrXferProcessor& GrPorterDuffXPFactory::SimpleSrcOverXP() {
    static BlendFormula gSrcOverBlendFormula =
            MakeCoeffFormula(kOne_GrBlendCoeff, kISA_GrBlendCoeff);
    static PorterDuffXferProcessor gSrcOverXP(gSrcOverBlendFormula,
                                              GrProcessorAnalysisCoverage::kSingleChannel);
    return gSrcOverXP;
}

sk_sp<const GrXferProcessor> GrPorterDuffXPFactory::MakeSrcOverXferProcessor(
        const GrProcessorAnalysisColor& color, GrProcessorAnalysisCoverage coverage,
        const GrCaps& caps) {
    // We want to not make an xfer processor if possible. Thus for the simple case where we are not
    // doing lcd blending we will just use our global SimpleSrcOverXP. This slightly differs from
    // the general case where we convert a src-over blend that has solid coverage and an opaque
    // color to src-mode, which allows disabling of blending.
    if (coverage != GrProcessorAnalysisCoverage::kLCD) {
        if (color.isOpaque() && coverage == GrProcessorAnalysisCoverage::kNone &&
            caps.shouldCollapseSrcOverToSrcWhenAble()) {
            BlendFormula blendFormula = get_blend_formula(true, false, SkBlendMode::kSrc);
            return sk_sp<GrXferProcessor>(new PorterDuffXferProcessor(blendFormula, coverage));
        }
        // We return nullptr here, which our caller interprets as meaning "use SimpleSrcOverXP".
        // We don't simply return the address of that XP here because our caller would have to unref
        // it and since it is a global object and GrProgramElement's ref-cnting system is not thread
        // safe.
        return nullptr;
    }

    // Currently up the stack Skia is requiring that the dst is opaque or that the client has said
    // the opaqueness doesn't matter. Thus for src-over we don't need to worry about the src color
    // being opaque or not. This allows us to use faster code paths as well as avoid various bugs
    // that occur with dst reads in the shader blending. For now we disable the check for
    // opaqueness, but in the future we should pass down the knowledge about dst opaqueness and make
    // the correct decision here.
    //
    // This also fixes a chrome bug on macs where we are getting random fuzziness when doing
    // blending in the shader for non opaque sources.
    if (color.isConstant() && /*color.isOpaque() &&*/
        !caps.shaderCaps()->dualSourceBlendingSupport() &&
        !caps.shaderCaps()->dstReadInShaderSupport()) {
        // If we don't have dual source blending or in shader dst reads, we fall
        // back to this trick for rendering SrcOver LCD text instead of doing a
        // dst copy.
        return PDLCDXferProcessor::Make(SkBlendMode::kSrcOver, color);
    }

    BlendFormula blendFormula = get_lcd_blend_formula(SkBlendMode::kSrcOver);
    // See comment above regarding why the opaque check is commented out here.
    if (/*!color.isOpaque() ||*/
        (blendFormula.hasSecondaryOutput() && !caps.shaderCaps()->dualSourceBlendingSupport())) {
        return sk_sp<GrXferProcessor>(new ShaderPDXferProcessor(SkBlendMode::kSrcOver, coverage));
    }
    return sk_sp<GrXferProcessor>(new PorterDuffXferProcessor(blendFormula, coverage));
}

sk_sp<const GrXferProcessor> GrPorterDuffXPFactory::MakeNoCoverageXP(SkBlendMode blendmode) {
    BlendFormula formula = get_blend_formula(false, false, blendmode);
    return sk_make_sp<PorterDuffXferProcessor>(formula, GrProcessorAnalysisCoverage::kNone);
}

GrXPFactory::AnalysisProperties GrPorterDuffXPFactory::SrcOverAnalysisProperties(
        const GrProcessorAnalysisColor& color,
        const GrProcessorAnalysisCoverage& coverage,
        const GrCaps& caps,
        GrClampType clampType) {
    return analysis_properties(color, coverage, caps, clampType, SkBlendMode::kSrcOver);
}
