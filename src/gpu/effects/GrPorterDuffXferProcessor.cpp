/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrPorterDuffXferProcessor.h"

#include "include/gpu/GrBlend.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkTo.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProcessorAnalysis.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/glsl/GrGLSLBlend.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLXferProcessor.h"

/**
 * Wraps the shader outputs and HW blend state that comprise a Porter Duff blend mode with coverage.
 */
class BlendFormula {
public:
    /**
     * Values the shader can write to primary and secondary outputs. These must all be modulated by
     * coverage to support mixed samples. The XP will ignore the multiplies when not using coverage.
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

    BlendFormula() = default;

    constexpr BlendFormula(OutputType primaryOut, OutputType secondaryOut, GrBlendEquation equation,
                           GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff)
            : fPrimaryOutputType(primaryOut)
            , fSecondaryOutputType(secondaryOut)
            , fBlendEquation(equation)
            , fSrcCoeff(srcCoeff)
            , fDstCoeff(dstCoeff)
            , fProps(GetProperties(primaryOut, secondaryOut, equation, srcCoeff, dstCoeff)) {}

    BlendFormula& operator=(const BlendFormula& other) {
        SkDEBUGCODE(other.validatePreoptimized());
        fData = other.fData;
        return *this;
    }

    bool operator==(const BlendFormula& other) const {
        SkDEBUGCODE(this->validatePreoptimized());
        SkDEBUGCODE(other.validatePreoptimized());
        return fData == other.fData;
    }

    bool hasSecondaryOutput() const {
        SkDEBUGCODE(this->validatePreoptimized());
        return kNone_OutputType != fSecondaryOutputType;
    }
    bool modifiesDst() const {
        SkDEBUGCODE(this->validatePreoptimized());
        return SkToBool(fProps & kModifiesDst_Property);
    }
    bool usesDstColor() const {
        SkDEBUGCODE(this->validatePreoptimized());
        return SkToBool(fProps & kUsesDstColor_Property);
    }
    bool usesInputColor() const {
        SkDEBUGCODE(this->validatePreoptimized());
        return SkToBool(fProps & kUsesInputColor_Property);
    }
    bool canTweakAlphaForCoverage() const {
        SkDEBUGCODE(this->validatePreoptimized());
        return SkToBool(fProps & kCanTweakAlphaForCoverage_Property);
    }

    GrBlendEquation equation() const {
        SkDEBUGCODE(this->validatePreoptimized());
        return fBlendEquation;
    }

    GrBlendCoeff srcCoeff() const {
        SkDEBUGCODE(this->validatePreoptimized());
        return fSrcCoeff;
    }

    GrBlendCoeff dstCoeff() const {
        SkDEBUGCODE(this->validatePreoptimized());
        return fDstCoeff;
    }

    OutputType primaryOutput() const {
        SkDEBUGCODE(this->validatePreoptimized());
        return fPrimaryOutputType;
    }

    OutputType secondaryOutput() const {
        SkDEBUGCODE(this->validatePreoptimized());
        return fSecondaryOutputType;
    }

private:
    enum Properties {
        kModifiesDst_Property              = 1,
        kUsesDstColor_Property             = 1 << 1,
        kUsesInputColor_Property           = 1 << 2,
        kCanTweakAlphaForCoverage_Property = 1 << 3,

        kLast_Property = kCanTweakAlphaForCoverage_Property
    };
    GR_DECL_BITFIELD_OPS_FRIENDS(Properties)

#ifdef SK_DEBUG
    void validatePreoptimized() const {
        // The provided formula should already be optimized before a BlendFormula is constructed.
        // Preferably these asserts would be done statically in the constexpr constructor, but this
        // is not allowed in C++11.
        SkASSERT((kNone_OutputType == fPrimaryOutputType) ==
                 !GrBlendCoeffsUseSrcColor(fSrcCoeff, fDstCoeff));
        SkASSERT(!GrBlendCoeffRefsSrc2(fSrcCoeff));
        SkASSERT((kNone_OutputType == fSecondaryOutputType) == !GrBlendCoeffRefsSrc2(fDstCoeff));
        SkASSERT(fPrimaryOutputType != fSecondaryOutputType ||
                 kNone_OutputType == fPrimaryOutputType);
        SkASSERT(kNone_OutputType != fPrimaryOutputType ||
                 kNone_OutputType == fSecondaryOutputType);
    }
#endif

    /**
     * Deduce the properties of a BlendFormula.
     */
    static constexpr Properties GetProperties(OutputType PrimaryOut, OutputType SecondaryOut,
                                              GrBlendEquation BlendEquation, GrBlendCoeff SrcCoeff,
                                              GrBlendCoeff DstCoeff);

    union {
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
        uint32_t fData;
    };

    GR_STATIC_ASSERT(kLast_OutputType      < (1 << 3));
    GR_STATIC_ASSERT(kLast_GrBlendEquation < (1 << 5));
    GR_STATIC_ASSERT(kLast_GrBlendCoeff    < (1 << 5));
    GR_STATIC_ASSERT(kLast_Property        < (1 << 6));
};

GR_STATIC_ASSERT(4 == sizeof(BlendFormula));

GR_MAKE_BITFIELD_OPS(BlendFormula::Properties);

constexpr BlendFormula::Properties BlendFormula::GetProperties(OutputType PrimaryOut,
                                                               OutputType SecondaryOut,
                                                               GrBlendEquation BlendEquation,
                                                               GrBlendCoeff SrcCoeff,
                                                               GrBlendCoeff DstCoeff) {
    return static_cast<Properties>(
            (GrBlendModifiesDst(BlendEquation, SrcCoeff, DstCoeff) ? kModifiesDst_Property : 0) |
            (GrBlendCoeffsUseDstColor(SrcCoeff, DstCoeff) ? kUsesDstColor_Property : 0) |
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

// Older GCC won't like the constexpr arrays because of
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61484.
// MSVC 2015 crashes with an internal compiler error.
#if !defined(__clang__) && ((defined(__GNUC__) && __GNUC__ < 5) || (defined(_MSC_VER) && _MSC_VER <= 1910))
#   define MAYBE_CONSTEXPR const
#else
#   define MAYBE_CONSTEXPR constexpr
#endif

/**
 * This table outlines the blend formulas we will use with each xfermode, with and without coverage,
 * with and without an opaque input color. Optimization properties are deduced at compile time so we
 * can make runtime decisions quickly. RGB coverage is not supported.
 */
static MAYBE_CONSTEXPR BlendFormula gBlendTable[2][2][(int)SkBlendMode::kLastCoeffMode + 1] = {
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
// not specialized for opaque input. If the table were set to use the src formula then we'd have to
// change when we use this global XP to keep analysis and practice in sync.

static MAYBE_CONSTEXPR BlendFormula gLCDBlendTable[(int)SkBlendMode::kLastCoeffMode + 1] = {
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

#undef MAYBE_CONSTEXPR

static BlendFormula get_blend_formula(bool isOpaque,
                                      bool hasCoverage,
                                      bool hasMixedSamples,
                                      SkBlendMode xfermode) {
    SkASSERT((unsigned)xfermode <= (unsigned)SkBlendMode::kLastCoeffMode);
    bool conflatesCoverage = hasCoverage || hasMixedSamples;
    return gBlendTable[isOpaque][conflatesCoverage][(int)xfermode];
}

static BlendFormula get_lcd_blend_formula(SkBlendMode xfermode) {
    SkASSERT((unsigned)xfermode <= (unsigned)SkBlendMode::kLastCoeffMode);

    return gLCDBlendTable[(int)xfermode];
}

///////////////////////////////////////////////////////////////////////////////

class PorterDuffXferProcessor : public GrXferProcessor {
public:
    PorterDuffXferProcessor(BlendFormula blendFormula, GrProcessorAnalysisCoverage coverage)
            : INHERITED(kPorterDuffXferProcessor_ClassID, false, false, coverage)
            , fBlendFormula(blendFormula) {
    }

    const char* name() const override { return "Porter Duff"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

    BlendFormula getBlendFormula() const { return fBlendFormula; }

private:
    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

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

    typedef GrXferProcessor INHERITED;
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
            // We can have a coverage formula while not reading coverage if there are mixed samples.
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

class GLPorterDuffXferProcessor : public GrGLSLXferProcessor {
public:
    static void GenKey(const GrProcessor& processor, GrProcessorKeyBuilder* b) {
        const PorterDuffXferProcessor& xp = processor.cast<PorterDuffXferProcessor>();
        b->add32(xp.getBlendFormula().primaryOutput() |
                 (xp.getBlendFormula().secondaryOutput() << 3));
        GR_STATIC_ASSERT(BlendFormula::kLast_OutputType < 8);
    }

private:
    void emitOutputsForBlendState(const EmitArgs& args) override {
        const PorterDuffXferProcessor& xp = args.fXP.cast<PorterDuffXferProcessor>();
        GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;

        BlendFormula blendFormula = xp.getBlendFormula();
        if (blendFormula.hasSecondaryOutput()) {
            append_color_output(xp, fragBuilder, blendFormula.secondaryOutput(),
                                args.fOutputSecondary, args.fInputColor, args.fInputCoverage);
        }
        append_color_output(xp, fragBuilder, blendFormula.primaryOutput(), args.fOutputPrimary,
                            args.fInputColor, args.fInputCoverage);
    }

    void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) override {}

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void PorterDuffXferProcessor::onGetGLSLProcessorKey(const GrShaderCaps&,
                                                    GrProcessorKeyBuilder* b) const {
    GLPorterDuffXferProcessor::GenKey(*this, b);
}

GrGLSLXferProcessor* PorterDuffXferProcessor::createGLSLInstance() const {
    return new GLPorterDuffXferProcessor;
}

///////////////////////////////////////////////////////////////////////////////

class ShaderPDXferProcessor : public GrXferProcessor {
public:
    ShaderPDXferProcessor(bool hasMixedSamples, SkBlendMode xfermode,
                          GrProcessorAnalysisCoverage coverage)
            : INHERITED(kShaderPDXferProcessor_ClassID, true, hasMixedSamples, coverage)
            , fXfermode(xfermode) {
    }

    const char* name() const override { return "Porter Duff Shader"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

    SkBlendMode getXfermode() const { return fXfermode; }

private:
    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const ShaderPDXferProcessor& xp = xpBase.cast<ShaderPDXferProcessor>();
        return fXfermode == xp.fXfermode;
    }

    const SkBlendMode fXfermode;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLShaderPDXferProcessor : public GrGLSLXferProcessor {
public:
    static void GenKey(const GrProcessor& processor, GrProcessorKeyBuilder* b) {
        const ShaderPDXferProcessor& xp = processor.cast<ShaderPDXferProcessor>();
        b->add32((int)xp.getXfermode());
    }

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

        GrGLSLBlend::AppendMode(fragBuilder, srcColor, dstColor, outColor, xp.getXfermode());

        // Apply coverage.
        INHERITED::DefaultCoverageModulation(fragBuilder, srcCoverage, dstColor, outColor,
                                             outColorSecondary, xp);
    }

    void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) override {}

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void ShaderPDXferProcessor::onGetGLSLProcessorKey(const GrShaderCaps&,
                                                  GrProcessorKeyBuilder* b) const {
    GLShaderPDXferProcessor::GenKey(*this, b);
}

GrGLSLXferProcessor* ShaderPDXferProcessor::createGLSLInstance() const {
    return new GLShaderPDXferProcessor;
}

///////////////////////////////////////////////////////////////////////////////

class PDLCDXferProcessor : public GrXferProcessor {
public:
    static sk_sp<const GrXferProcessor> Make(SkBlendMode mode,
                                             const GrProcessorAnalysisColor& inputColor);

    ~PDLCDXferProcessor() override;

    const char* name() const override { return "Porter Duff LCD"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

    float alpha() const { return fAlpha; }

private:
    PDLCDXferProcessor(const SkPMColor4f& blendConstant, float alpha);

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

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

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLPDLCDXferProcessor : public GrGLSLXferProcessor {
public:
    GLPDLCDXferProcessor(const GrProcessor&) : fLastAlpha(SK_FloatNaN) {}

    ~GLPDLCDXferProcessor() override {}

    static void GenKey(const GrProcessor& processor, const GrShaderCaps& caps,
                       GrProcessorKeyBuilder* b) {}

private:
    void emitOutputsForBlendState(const EmitArgs& args) override {
        const char* alpha;
        fAlphaUniform = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                                         "alpha", &alpha);
        GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;
        // We want to force our primary output to be alpha * Coverage, where alpha is the alpha
        // value of the src color. We know that there are no color stages (or we wouldn't have
        // created this xp) and the r,g, and b channels of the op's input color are baked into the
        // blend constant.
        SkASSERT(args.fInputCoverage);
        fragBuilder->codeAppendf("%s = %s * %s;", args.fOutputPrimary, alpha, args.fInputCoverage);
    }

    void onSetData(const GrGLSLProgramDataManager& pdm, const GrXferProcessor& xp) override {
        float alpha = xp.cast<PDLCDXferProcessor>().alpha();
        if (fLastAlpha != alpha) {
            pdm.set1f(fAlphaUniform, alpha);
            fLastAlpha = alpha;
        }
    }

    GrGLSLUniformHandler::UniformHandle fAlphaUniform;
    float fLastAlpha;
    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

PDLCDXferProcessor::PDLCDXferProcessor(const SkPMColor4f& blendConstant, float alpha)
    : INHERITED(kPDLCDXferProcessor_ClassID, false, false, GrProcessorAnalysisCoverage::kLCD)
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

PDLCDXferProcessor::~PDLCDXferProcessor() {
}

void PDLCDXferProcessor::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                               GrProcessorKeyBuilder* b) const {
    GLPDLCDXferProcessor::GenKey(*this, caps, b);
}

GrGLSLXferProcessor* PDLCDXferProcessor::createGLSLInstance() const {
    return new GLPDLCDXferProcessor(*this);
}

///////////////////////////////////////////////////////////////////////////////

constexpr GrPorterDuffXPFactory::GrPorterDuffXPFactory(SkBlendMode xfermode)
        : fBlendMode(xfermode) {}

const GrXPFactory* GrPorterDuffXPFactory::Get(SkBlendMode blendMode) {
    SkASSERT((unsigned)blendMode <= (unsigned)SkBlendMode::kLastCoeffMode);

    // If these objects are constructed as static constexpr by cl.exe (2015 SP2) the vtables are
    // null.
#ifdef SK_BUILD_FOR_WIN
#define _CONSTEXPR_
#else
#define _CONSTEXPR_ constexpr
#endif
    static _CONSTEXPR_ const GrPorterDuffXPFactory gClearPDXPF(SkBlendMode::kClear);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gSrcPDXPF(SkBlendMode::kSrc);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gDstPDXPF(SkBlendMode::kDst);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gSrcOverPDXPF(SkBlendMode::kSrcOver);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gDstOverPDXPF(SkBlendMode::kDstOver);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gSrcInPDXPF(SkBlendMode::kSrcIn);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gDstInPDXPF(SkBlendMode::kDstIn);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gSrcOutPDXPF(SkBlendMode::kSrcOut);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gDstOutPDXPF(SkBlendMode::kDstOut);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gSrcATopPDXPF(SkBlendMode::kSrcATop);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gDstATopPDXPF(SkBlendMode::kDstATop);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gXorPDXPF(SkBlendMode::kXor);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gPlusPDXPF(SkBlendMode::kPlus);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gModulatePDXPF(SkBlendMode::kModulate);
    static _CONSTEXPR_ const GrPorterDuffXPFactory gScreenPDXPF(SkBlendMode::kScreen);
#undef _CONSTEXPR_

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
            return nullptr;
    }
}

sk_sp<const GrXferProcessor> GrPorterDuffXPFactory::makeXferProcessor(
        const GrProcessorAnalysisColor& color, GrProcessorAnalysisCoverage coverage,
        bool hasMixedSamples, const GrCaps& caps, GrClampType clampType) const {
    BlendFormula blendFormula;
    bool isLCD = coverage == GrProcessorAnalysisCoverage::kLCD;
    if (isLCD) {
        // See comment in MakeSrcOverXferProcessor about color.isOpaque here
        if (SkBlendMode::kSrcOver == fBlendMode && color.isConstant() && /*color.isOpaque() &&*/
            !caps.shaderCaps()->dualSourceBlendingSupport() &&
            !caps.shaderCaps()->dstReadInShaderSupport()) {
            // If we don't have dual source blending or in shader dst reads, we fall back to this
            // trick for rendering SrcOver LCD text instead of doing a dst copy.
            return PDLCDXferProcessor::Make(fBlendMode, color);
        }
        blendFormula = get_lcd_blend_formula(fBlendMode);
    } else {
        blendFormula =
                get_blend_formula(color.isOpaque(), GrProcessorAnalysisCoverage::kNone != coverage,
                                  hasMixedSamples, fBlendMode);
    }

    // Skia always saturates after the kPlus blend mode, so it requires shader-based blending when
    // pixels aren't guaranteed to automatically be normalized (i.e. any floating point config).
    if ((blendFormula.hasSecondaryOutput() && !caps.shaderCaps()->dualSourceBlendingSupport()) ||
        (isLCD && (SkBlendMode::kSrcOver != fBlendMode /*|| !color.isOpaque()*/)) ||
        (GrClampType::kAuto != clampType && SkBlendMode::kPlus == fBlendMode)) {
        return sk_sp<const GrXferProcessor>(new ShaderPDXferProcessor(hasMixedSamples, fBlendMode,
                                                                      coverage));
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
    BlendFormula formula;
    if (isLCD) {
        formula = gLCDBlendTable[(int)mode];
    } else {
        formula = gBlendTable[color.isOpaque()][hasCoverage][(int)mode];
    }

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
            // Mixed samples implicity computes a fractional coverage from sample coverage. This
            // could affect the formula used. However, we don't expect to have mixed samples without
            // dual source blending.
            SkASSERT(!caps.mixedSamplesSupport());
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
        bool hasMixedSamples, const GrCaps& caps) {
    // We want to not make an xfer processor if possible. Thus for the simple case where we are not
    // doing lcd blending we will just use our global SimpleSrcOverXP. This slightly differs from
    // the general case where we convert a src-over blend that has solid coverage and an opaque
    // color to src-mode, which allows disabling of blending.
    if (coverage != GrProcessorAnalysisCoverage::kLCD) {
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

    BlendFormula blendFormula;
    blendFormula = get_lcd_blend_formula(SkBlendMode::kSrcOver);
    // See comment above regarding why the opaque check is commented out here.
    if (/*!color.isOpaque() ||*/
        (blendFormula.hasSecondaryOutput() && !caps.shaderCaps()->dualSourceBlendingSupport())) {
        return sk_sp<GrXferProcessor>(
                new ShaderPDXferProcessor(hasMixedSamples, SkBlendMode::kSrcOver, coverage));
    }
    return sk_sp<GrXferProcessor>(new PorterDuffXferProcessor(blendFormula, coverage));
}

sk_sp<const GrXferProcessor> GrPorterDuffXPFactory::MakeNoCoverageXP(SkBlendMode blendmode) {
    BlendFormula formula = get_blend_formula(false, false, false, blendmode);
    return sk_make_sp<PorterDuffXferProcessor>(formula, GrProcessorAnalysisCoverage::kNone);
}

GrXPFactory::AnalysisProperties GrPorterDuffXPFactory::SrcOverAnalysisProperties(
        const GrProcessorAnalysisColor& color,
        const GrProcessorAnalysisCoverage& coverage,
        const GrCaps& caps,
        GrClampType clampType) {
    return analysis_properties(color, coverage, caps, clampType, SkBlendMode::kSrcOver);
}
