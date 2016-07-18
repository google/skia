/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrPorterDuffXferProcessor.h"

#include "GrBlend.h"
#include "GrCaps.h"
#include "GrPipeline.h"
#include "GrProcessor.h"
#include "GrProcOptInfo.h"
#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "glsl/GrGLSLBlend.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLXferProcessor.h"
#include <utility>

/**
 * Wraps the shader outputs and HW blend state that comprise a Porter Duff blend mode with coverage.
 */
struct BlendFormula {
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

    enum Properties {
        kModifiesDst_Property              = 1,
        kUsesDstColor_Property             = 1 << 1,
        kUsesInputColor_Property           = 1 << 2,
        kCanTweakAlphaForCoverage_Property = 1 << 3,

        kLast_Property = kCanTweakAlphaForCoverage_Property
    };

    BlendFormula& operator =(const BlendFormula& other) {
        fData = other.fData;
        return *this;
    }

    bool operator ==(const BlendFormula& other) const {
        return fData == other.fData;
    }

    bool hasSecondaryOutput() const { return kNone_OutputType != fSecondaryOutputType; }
    bool modifiesDst() const { return SkToBool(fProps & kModifiesDst_Property); }
    bool usesDstColor() const { return SkToBool(fProps & kUsesDstColor_Property); }
    bool usesInputColor() const { return SkToBool(fProps & kUsesInputColor_Property); }
    bool canTweakAlphaForCoverage() const {
        return SkToBool(fProps & kCanTweakAlphaForCoverage_Property);
    }

    /**
     * Deduce the properties of a compile-time constant BlendFormula.
     */
    template<OutputType PrimaryOut, OutputType SecondaryOut,
             GrBlendEquation BlendEquation, GrBlendCoeff SrcCoeff, GrBlendCoeff DstCoeff>
    struct get_properties : std::integral_constant<Properties, static_cast<Properties>(

        (GR_BLEND_MODIFIES_DST(BlendEquation, SrcCoeff, DstCoeff) ?
            kModifiesDst_Property : 0) |

        (GR_BLEND_COEFFS_USE_DST_COLOR(SrcCoeff, DstCoeff) ?
            kUsesDstColor_Property : 0) |

        ((PrimaryOut >= kModulate_OutputType && GR_BLEND_COEFFS_USE_SRC_COLOR(SrcCoeff,DstCoeff)) ||
         (SecondaryOut >= kModulate_OutputType && GR_BLEND_COEFF_REFS_SRC2(DstCoeff)) ?
            kUsesInputColor_Property : 0) |  // We assert later that SrcCoeff doesn't ref src2.

        (kModulate_OutputType == PrimaryOut &&
         kNone_OutputType == SecondaryOut &&
         GR_BLEND_CAN_TWEAK_ALPHA_FOR_COVERAGE(BlendEquation, SrcCoeff, DstCoeff) ?
            kCanTweakAlphaForCoverage_Property : 0))> {

        // The provided formula should already be optimized.
        GR_STATIC_ASSERT((kNone_OutputType == PrimaryOut) ==
                         !GR_BLEND_COEFFS_USE_SRC_COLOR(SrcCoeff, DstCoeff));
        GR_STATIC_ASSERT(!GR_BLEND_COEFF_REFS_SRC2(SrcCoeff));
        GR_STATIC_ASSERT((kNone_OutputType == SecondaryOut) ==
                         !GR_BLEND_COEFF_REFS_SRC2(DstCoeff));
        GR_STATIC_ASSERT(PrimaryOut != SecondaryOut || kNone_OutputType == PrimaryOut);
        GR_STATIC_ASSERT(kNone_OutputType != PrimaryOut || kNone_OutputType == SecondaryOut);
    };

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

/**
 * Initialize a compile-time constant BlendFormula and automatically deduce fProps.
 */
#define INIT_BLEND_FORMULA(PRIMARY_OUT, SECONDARY_OUT, BLEND_EQUATION, SRC_COEFF, DST_COEFF) \
    {{{PRIMARY_OUT, \
       SECONDARY_OUT, \
       BLEND_EQUATION, SRC_COEFF, DST_COEFF, \
       BlendFormula::get_properties<PRIMARY_OUT, SECONDARY_OUT, \
                                    BLEND_EQUATION, SRC_COEFF, DST_COEFF>::value}}}

/**
 * When there is no coverage, or the blend mode can tweak alpha for coverage, we use the standard
 * Porter Duff formula.
 */
#define COEFF_FORMULA(SRC_COEFF, DST_COEFF) \
    INIT_BLEND_FORMULA(BlendFormula::kModulate_OutputType, \
                       BlendFormula::kNone_OutputType, \
                       kAdd_GrBlendEquation, SRC_COEFF, DST_COEFF)

/**
 * Basic coeff formula similar to COEFF_FORMULA but we will make the src f*Sa. This is used in
 * LCD dst-out.
 */
#define COEFF_FORMULA_SA_MODULATE(SRC_COEFF, DST_COEFF) \
    INIT_BLEND_FORMULA(BlendFormula::kSAModulate_OutputType, \
                       BlendFormula::kNone_OutputType, \
                       kAdd_GrBlendEquation, SRC_COEFF, DST_COEFF)

/**
 * When the coeffs are (Zero, Zero), we clear the dst. This formula has its own macro so we can set
 * the primary output type to none.
 */
#define DST_CLEAR_FORMULA \
    INIT_BLEND_FORMULA(BlendFormula::kNone_OutputType, \
                       BlendFormula::kNone_OutputType, \
                       kAdd_GrBlendEquation, kZero_GrBlendCoeff, kZero_GrBlendCoeff)

/**
 * When the coeffs are (Zero, One), we don't write to the dst at all. This formula has its own macro
 * so we can set the primary output type to none.
 */
#define NO_DST_WRITE_FORMULA \
    INIT_BLEND_FORMULA(BlendFormula::kNone_OutputType, \
                       BlendFormula::kNone_OutputType, \
                       kAdd_GrBlendEquation, kZero_GrBlendCoeff, kOne_GrBlendCoeff)

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
#define COVERAGE_FORMULA(ONE_MINUS_DST_COEFF_MODULATE_OUTPUT, SRC_COEFF) \
    INIT_BLEND_FORMULA(BlendFormula::kModulate_OutputType, \
                       ONE_MINUS_DST_COEFF_MODULATE_OUTPUT, \
                       kAdd_GrBlendEquation, SRC_COEFF, kIS2C_GrBlendCoeff)

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
#define COVERAGE_SRC_COEFF_ZERO_FORMULA(ONE_MINUS_DST_COEFF_MODULATE_OUTPUT) \
    INIT_BLEND_FORMULA(ONE_MINUS_DST_COEFF_MODULATE_OUTPUT, \
                       BlendFormula::kNone_OutputType, \
                       kReverseSubtract_GrBlendEquation, kDC_GrBlendCoeff, kOne_GrBlendCoeff)

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
#define COVERAGE_DST_COEFF_ZERO_FORMULA(SRC_COEFF) \
    INIT_BLEND_FORMULA(BlendFormula::kModulate_OutputType, \
                       BlendFormula::kCoverage_OutputType, \
                       kAdd_GrBlendEquation, SRC_COEFF, kIS2A_GrBlendCoeff)

/**
 * This table outlines the blend formulas we will use with each xfermode, with and without coverage,
 * with and without an opaque input color. Optimization properties are deduced at compile time so we
 * can make runtime decisions quickly. RGB coverage is not supported.
 */
static const BlendFormula gBlendTable[2][2][SkXfermode::kLastCoeffMode + 1] = {

                     /*>> No coverage, input color unknown <<*/ {{

    /* clear */      DST_CLEAR_FORMULA,
    /* src */        COEFF_FORMULA(   kOne_GrBlendCoeff,    kZero_GrBlendCoeff),
    /* dst */        NO_DST_WRITE_FORMULA,
    /* src-over */   COEFF_FORMULA(   kOne_GrBlendCoeff,    kISA_GrBlendCoeff),
    /* dst-over */   COEFF_FORMULA(   kIDA_GrBlendCoeff,    kOne_GrBlendCoeff),
    /* src-in */     COEFF_FORMULA(   kDA_GrBlendCoeff,     kZero_GrBlendCoeff),
    /* dst-in */     COEFF_FORMULA(   kZero_GrBlendCoeff,   kSA_GrBlendCoeff),
    /* src-out */    COEFF_FORMULA(   kIDA_GrBlendCoeff,    kZero_GrBlendCoeff),
    /* dst-out */    COEFF_FORMULA(   kZero_GrBlendCoeff,   kISA_GrBlendCoeff),
    /* src-atop */   COEFF_FORMULA(   kDA_GrBlendCoeff,     kISA_GrBlendCoeff),
    /* dst-atop */   COEFF_FORMULA(   kIDA_GrBlendCoeff,    kSA_GrBlendCoeff),
    /* xor */        COEFF_FORMULA(   kIDA_GrBlendCoeff,    kISA_GrBlendCoeff),
    /* plus */       COEFF_FORMULA(   kOne_GrBlendCoeff,    kOne_GrBlendCoeff),
    /* modulate */   COEFF_FORMULA(   kZero_GrBlendCoeff,   kSC_GrBlendCoeff),
    /* screen */     COEFF_FORMULA(   kOne_GrBlendCoeff,    kISC_GrBlendCoeff),

                     }, /*>> Has coverage, input color unknown <<*/ {

    /* clear */      COVERAGE_SRC_COEFF_ZERO_FORMULA(BlendFormula::kCoverage_OutputType),
    /* src */        COVERAGE_DST_COEFF_ZERO_FORMULA(kOne_GrBlendCoeff),
    /* dst */        NO_DST_WRITE_FORMULA,
    /* src-over */   COEFF_FORMULA(   kOne_GrBlendCoeff,    kISA_GrBlendCoeff),
    /* dst-over */   COEFF_FORMULA(   kIDA_GrBlendCoeff,    kOne_GrBlendCoeff),
    /* src-in */     COVERAGE_DST_COEFF_ZERO_FORMULA(kDA_GrBlendCoeff),
    /* dst-in */     COVERAGE_SRC_COEFF_ZERO_FORMULA(BlendFormula::kISAModulate_OutputType),
    /* src-out */    COVERAGE_DST_COEFF_ZERO_FORMULA(kIDA_GrBlendCoeff),
    /* dst-out */    COEFF_FORMULA(   kZero_GrBlendCoeff,   kISA_GrBlendCoeff),
    /* src-atop */   COEFF_FORMULA(   kDA_GrBlendCoeff,     kISA_GrBlendCoeff),
    /* dst-atop */   COVERAGE_FORMULA(BlendFormula::kISAModulate_OutputType, kIDA_GrBlendCoeff),
    /* xor */        COEFF_FORMULA(   kIDA_GrBlendCoeff,    kISA_GrBlendCoeff),
    /* plus */       COEFF_FORMULA(   kOne_GrBlendCoeff,    kOne_GrBlendCoeff),
    /* modulate */   COVERAGE_SRC_COEFF_ZERO_FORMULA(BlendFormula::kISCModulate_OutputType),
    /* screen */     COEFF_FORMULA(   kOne_GrBlendCoeff,    kISC_GrBlendCoeff),

                     }}, /*>> No coverage, input color opaque <<*/ {{

    /* clear */      DST_CLEAR_FORMULA,
    /* src */        COEFF_FORMULA(   kOne_GrBlendCoeff,    kZero_GrBlendCoeff),
    /* dst */        NO_DST_WRITE_FORMULA,
    /* src-over */   COEFF_FORMULA(   kOne_GrBlendCoeff,    kZero_GrBlendCoeff),
    /* dst-over */   COEFF_FORMULA(   kIDA_GrBlendCoeff,    kOne_GrBlendCoeff),
    /* src-in */     COEFF_FORMULA(   kDA_GrBlendCoeff,     kZero_GrBlendCoeff),
    /* dst-in */     NO_DST_WRITE_FORMULA,
    /* src-out */    COEFF_FORMULA(   kIDA_GrBlendCoeff,    kZero_GrBlendCoeff),
    /* dst-out */    DST_CLEAR_FORMULA,
    /* src-atop */   COEFF_FORMULA(   kDA_GrBlendCoeff,     kZero_GrBlendCoeff),
    /* dst-atop */   COEFF_FORMULA(   kIDA_GrBlendCoeff,    kOne_GrBlendCoeff),
    /* xor */        COEFF_FORMULA(   kIDA_GrBlendCoeff,    kZero_GrBlendCoeff),
    /* plus */       COEFF_FORMULA(   kOne_GrBlendCoeff,    kOne_GrBlendCoeff),
    /* modulate */   COEFF_FORMULA(   kZero_GrBlendCoeff,   kSC_GrBlendCoeff),
    /* screen */     COEFF_FORMULA(   kOne_GrBlendCoeff,    kISC_GrBlendCoeff),

                     }, /*>> Has coverage, input color opaque <<*/ {

    /* clear */      COVERAGE_SRC_COEFF_ZERO_FORMULA(BlendFormula::kCoverage_OutputType),
    /* src */        COEFF_FORMULA(   kOne_GrBlendCoeff,    kISA_GrBlendCoeff),
    /* dst */        NO_DST_WRITE_FORMULA,
    /* src-over */   COEFF_FORMULA(   kOne_GrBlendCoeff,    kISA_GrBlendCoeff),
    /* dst-over */   COEFF_FORMULA(   kIDA_GrBlendCoeff,    kOne_GrBlendCoeff),
    /* src-in */     COEFF_FORMULA(   kDA_GrBlendCoeff,     kISA_GrBlendCoeff),
    /* dst-in */     NO_DST_WRITE_FORMULA,
    /* src-out */    COEFF_FORMULA(   kIDA_GrBlendCoeff,    kISA_GrBlendCoeff),
    /* dst-out */    COVERAGE_SRC_COEFF_ZERO_FORMULA(BlendFormula::kCoverage_OutputType),
    /* src-atop */   COEFF_FORMULA(   kDA_GrBlendCoeff,     kISA_GrBlendCoeff),
    /* dst-atop */   COEFF_FORMULA(   kIDA_GrBlendCoeff,    kOne_GrBlendCoeff),
    /* xor */        COEFF_FORMULA(   kIDA_GrBlendCoeff,    kISA_GrBlendCoeff),
    /* plus */       COEFF_FORMULA(   kOne_GrBlendCoeff,    kOne_GrBlendCoeff),
    /* modulate */   COVERAGE_SRC_COEFF_ZERO_FORMULA(BlendFormula::kISCModulate_OutputType),
    /* screen */     COEFF_FORMULA(   kOne_GrBlendCoeff,    kISC_GrBlendCoeff),
}}};

static const BlendFormula gLCDBlendTable[SkXfermode::kLastCoeffMode + 1] = {
    /* clear */      COVERAGE_SRC_COEFF_ZERO_FORMULA(BlendFormula::kCoverage_OutputType),
    /* src */        COVERAGE_FORMULA(BlendFormula::kCoverage_OutputType, kOne_GrBlendCoeff),
    /* dst */        NO_DST_WRITE_FORMULA,
    /* src-over */   COVERAGE_FORMULA(BlendFormula::kSAModulate_OutputType, kOne_GrBlendCoeff),
    /* dst-over */   COEFF_FORMULA(   kIDA_GrBlendCoeff,    kOne_GrBlendCoeff),
    /* src-in */     COVERAGE_FORMULA(BlendFormula::kCoverage_OutputType, kDA_GrBlendCoeff),
    /* dst-in */     COVERAGE_SRC_COEFF_ZERO_FORMULA(BlendFormula::kISAModulate_OutputType),
    /* src-out */    COVERAGE_FORMULA(BlendFormula::kCoverage_OutputType, kIDA_GrBlendCoeff),
    /* dst-out */    COEFF_FORMULA_SA_MODULATE(   kZero_GrBlendCoeff,   kISC_GrBlendCoeff),
    /* src-atop */   COVERAGE_FORMULA(BlendFormula::kSAModulate_OutputType, kDA_GrBlendCoeff),
    /* dst-atop */   COVERAGE_FORMULA(BlendFormula::kISAModulate_OutputType, kIDA_GrBlendCoeff),
    /* xor */        COVERAGE_FORMULA(BlendFormula::kSAModulate_OutputType, kIDA_GrBlendCoeff),
    /* plus */       COEFF_FORMULA(   kOne_GrBlendCoeff,    kOne_GrBlendCoeff),
    /* modulate */   COVERAGE_SRC_COEFF_ZERO_FORMULA(BlendFormula::kISCModulate_OutputType),
    /* screen */     COEFF_FORMULA(   kOne_GrBlendCoeff,    kISC_GrBlendCoeff),
};

static BlendFormula get_blend_formula(const GrProcOptInfo& colorPOI,
                                      const GrProcOptInfo& coveragePOI,
                                      bool hasMixedSamples,
                                      SkXfermode::Mode xfermode) {
    SkASSERT(xfermode >= 0 && xfermode <= SkXfermode::kLastCoeffMode);
    SkASSERT(!coveragePOI.isFourChannelOutput());

    bool conflatesCoverage = !coveragePOI.isSolidWhite() || hasMixedSamples;
    return gBlendTable[colorPOI.isOpaque()][conflatesCoverage][xfermode];
}

static BlendFormula get_lcd_blend_formula(const GrProcOptInfo& coveragePOI,
                                          SkXfermode::Mode xfermode) {
    SkASSERT(xfermode >= 0 && xfermode <= SkXfermode::kLastCoeffMode);
    SkASSERT(coveragePOI.isFourChannelOutput());

    return gLCDBlendTable[xfermode];
}

///////////////////////////////////////////////////////////////////////////////

class PorterDuffXferProcessor : public GrXferProcessor {
public:
    PorterDuffXferProcessor(BlendFormula blendFormula) : fBlendFormula(blendFormula) {
        this->initClassID<PorterDuffXferProcessor>();
    }

    const char* name() const override { return "Porter Duff"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

    BlendFormula getBlendFormula() const { return fBlendFormula; }

private:
    GrXferProcessor::OptFlags onGetOptimizations(const GrPipelineOptimizations& optimizations,
                                                 bool doesStencilWrite,
                                                 GrColor* overrideColor,
                                                 const GrCaps& caps) const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onHasSecondaryOutput() const override { return fBlendFormula.hasSecondaryOutput(); }

    void onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const override {
        blendInfo->fEquation = fBlendFormula.fBlendEquation;
        blendInfo->fSrcBlend = fBlendFormula.fSrcCoeff;
        blendInfo->fDstBlend = fBlendFormula.fDstCoeff;
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
    switch (outputType) {
        case BlendFormula::kNone_OutputType:
            fragBuilder->codeAppendf("%s = vec4(0.0);", output);
            break;
        case BlendFormula::kCoverage_OutputType:
            // We can have a coverage formula while not reading coverage if there are mixed samples.
            if (inCoverage) {
                fragBuilder->codeAppendf("%s = %s;", output, inCoverage);
            } else {
                fragBuilder->codeAppendf("%s = vec4(1.0);", output);
            }
            break;
        case BlendFormula::kModulate_OutputType:
            if (inCoverage) {
                fragBuilder->codeAppendf("%s = %s * %s;", output, inColor, inCoverage);
            } else {
                fragBuilder->codeAppendf("%s = %s;", output, inColor);
            }
            break;
        case BlendFormula::kSAModulate_OutputType:
            if (inCoverage) {
                fragBuilder->codeAppendf("%s = %s.a * %s;", output, inColor, inCoverage);
            } else {
                fragBuilder->codeAppendf("%s = %s;", output, inColor);
            }
            break;
        case BlendFormula::kISAModulate_OutputType:
            if (inCoverage) {
                fragBuilder->codeAppendf("%s = (1.0 - %s.a) * %s;", output, inColor, inCoverage);
            } else {
                fragBuilder->codeAppendf("%s = vec4(1.0 - %s.a);", output, inColor);
            }
            break;
        case BlendFormula::kISCModulate_OutputType:
            if (inCoverage) {
                fragBuilder->codeAppendf("%s = (vec4(1.0) - %s) * %s;", output, inColor, inCoverage);
            } else {
                fragBuilder->codeAppendf("%s = vec4(1.0) - %s;", output, inColor);
            }
            break;
        default:
            SkFAIL("Unsupported output type.");
            break;
    }
}

class GLPorterDuffXferProcessor : public GrGLSLXferProcessor {
public:
    static void GenKey(const GrProcessor& processor, GrProcessorKeyBuilder* b) {
        const PorterDuffXferProcessor& xp = processor.cast<PorterDuffXferProcessor>();
        b->add32(xp.getBlendFormula().fPrimaryOutputType |
                 (xp.getBlendFormula().fSecondaryOutputType << 3));
        GR_STATIC_ASSERT(BlendFormula::kLast_OutputType < 8);
    };

private:
    void emitOutputsForBlendState(const EmitArgs& args) override {
        const PorterDuffXferProcessor& xp = args.fXP.cast<PorterDuffXferProcessor>();
        GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;

        BlendFormula blendFormula = xp.getBlendFormula();
        if (blendFormula.hasSecondaryOutput()) {
            append_color_output(xp, fragBuilder, blendFormula.fSecondaryOutputType,
                                args.fOutputSecondary, args.fInputColor, args.fInputCoverage);
        }
        append_color_output(xp, fragBuilder, blendFormula.fPrimaryOutputType,
                            args.fOutputPrimary, args.fInputColor, args.fInputCoverage);
    }

    void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) override {}

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void PorterDuffXferProcessor::onGetGLSLProcessorKey(const GrGLSLCaps&,
                                                    GrProcessorKeyBuilder* b) const {
    GLPorterDuffXferProcessor::GenKey(*this, b);
}

GrGLSLXferProcessor* PorterDuffXferProcessor::createGLSLInstance() const {
    return new GLPorterDuffXferProcessor;
}

GrXferProcessor::OptFlags
PorterDuffXferProcessor::onGetOptimizations(const GrPipelineOptimizations& optimizations,
                                            bool doesStencilWrite,
                                            GrColor* overrideColor,
                                            const GrCaps& caps) const {
    GrXferProcessor::OptFlags optFlags = GrXferProcessor::kNone_OptFlags;
    if (!fBlendFormula.modifiesDst()) {
        if (!doesStencilWrite) {
            optFlags |= GrXferProcessor::kSkipDraw_OptFlag;
        }
        optFlags |= (GrXferProcessor::kIgnoreColor_OptFlag |
                     GrXferProcessor::kIgnoreCoverage_OptFlag |
                     GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag);
    } else {
        if (!fBlendFormula.usesInputColor()) {
            optFlags |= GrXferProcessor::kIgnoreColor_OptFlag;
        }
        if (optimizations.fCoveragePOI.isSolidWhite()) {
            optFlags |= GrXferProcessor::kIgnoreCoverage_OptFlag;
        }
        if (optimizations.fColorPOI.allStagesMultiplyInput() &&
            fBlendFormula.canTweakAlphaForCoverage() &&
            !optimizations.fCoveragePOI.isFourChannelOutput()) {
            optFlags |= GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag;
        }
    }
    return optFlags;
}

///////////////////////////////////////////////////////////////////////////////

class ShaderPDXferProcessor : public GrXferProcessor {
public:
    ShaderPDXferProcessor(const DstTexture* dstTexture,
                          bool hasMixedSamples,
                          SkXfermode::Mode xfermode)
        : INHERITED(dstTexture, true, hasMixedSamples)
        , fXfermode(xfermode) {
        this->initClassID<ShaderPDXferProcessor>();
    }

    const char* name() const override { return "Porter Duff Shader"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

    SkXfermode::Mode getXfermode() const { return fXfermode; }

private:
    GrXferProcessor::OptFlags onGetOptimizations(const GrPipelineOptimizations&, bool, GrColor*,
                                                 const GrCaps&) const override {
        return kNone_OptFlags;
    }

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const ShaderPDXferProcessor& xp = xpBase.cast<ShaderPDXferProcessor>();
        return fXfermode == xp.fXfermode;
    }

    const SkXfermode::Mode fXfermode;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLShaderPDXferProcessor : public GrGLSLXferProcessor {
public:
    static void GenKey(const GrProcessor& processor, GrProcessorKeyBuilder* b) {
        const ShaderPDXferProcessor& xp = processor.cast<ShaderPDXferProcessor>();
        b->add32(xp.getXfermode());
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

void ShaderPDXferProcessor::onGetGLSLProcessorKey(const GrGLSLCaps&,
                                                  GrProcessorKeyBuilder* b) const {
    GLShaderPDXferProcessor::GenKey(*this, b);
}

GrGLSLXferProcessor* ShaderPDXferProcessor::createGLSLInstance() const {
    return new GLShaderPDXferProcessor;
}

///////////////////////////////////////////////////////////////////////////////

class PDLCDXferProcessor : public GrXferProcessor {
public:
    static GrXferProcessor* Create(SkXfermode::Mode xfermode, const GrProcOptInfo& colorPOI);

    ~PDLCDXferProcessor() override;

    const char* name() const override { return "Porter Duff LCD"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

private:
    PDLCDXferProcessor(GrColor blendConstant, uint8_t alpha);

    GrXferProcessor::OptFlags onGetOptimizations(const GrPipelineOptimizations& optimizations,
                                                 bool doesStencilWrite,
                                                 GrColor* overrideColor,
                                                 const GrCaps& caps) const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    void onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const override {
        blendInfo->fSrcBlend = kConstC_GrBlendCoeff;
        blendInfo->fDstBlend = kISC_GrBlendCoeff;
        blendInfo->fBlendConstant = fBlendConstant;
    }

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const PDLCDXferProcessor& xp = xpBase.cast<PDLCDXferProcessor>();
        if (fBlendConstant != xp.fBlendConstant ||
            fAlpha != xp.fAlpha) {
            return false;
        }
        return true;
    }

    GrColor      fBlendConstant;
    uint8_t      fAlpha;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLPDLCDXferProcessor : public GrGLSLXferProcessor {
public:
    GLPDLCDXferProcessor(const GrProcessor&) {}

    virtual ~GLPDLCDXferProcessor() {}

    static void GenKey(const GrProcessor& processor, const GrGLSLCaps& caps,
                       GrProcessorKeyBuilder* b) {}

private:
    void emitOutputsForBlendState(const EmitArgs& args) override {
        GrGLSLXPFragmentBuilder* fragBuilder = args.fXPFragBuilder;
        SkASSERT(args.fInputCoverage);
        fragBuilder->codeAppendf("%s = %s * %s;", args.fOutputPrimary, args.fInputColor,
                                 args.fInputCoverage);
    }

    void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) override {};

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

PDLCDXferProcessor::PDLCDXferProcessor(GrColor blendConstant, uint8_t alpha)
    : fBlendConstant(blendConstant)
    , fAlpha(alpha) {
    this->initClassID<PDLCDXferProcessor>();
}

GrXferProcessor* PDLCDXferProcessor::Create(SkXfermode::Mode xfermode,
                                            const GrProcOptInfo& colorPOI) {
    if (SkXfermode::kSrcOver_Mode != xfermode) {
        return nullptr;
    }

    if (kRGBA_GrColorComponentFlags != colorPOI.validFlags()) {
        return nullptr;
    }

    GrColor blendConstant = GrUnpremulColor(colorPOI.color());
    uint8_t alpha = GrColorUnpackA(blendConstant);
    blendConstant |= (0xff << GrColor_SHIFT_A);

    return new PDLCDXferProcessor(blendConstant, alpha);
}

PDLCDXferProcessor::~PDLCDXferProcessor() {
}

void PDLCDXferProcessor::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                               GrProcessorKeyBuilder* b) const {
    GLPDLCDXferProcessor::GenKey(*this, caps, b);
}

GrGLSLXferProcessor* PDLCDXferProcessor::createGLSLInstance() const {
    return new GLPDLCDXferProcessor(*this);
}

GrXferProcessor::OptFlags
PDLCDXferProcessor::onGetOptimizations(const GrPipelineOptimizations& optimizations,
                                       bool doesStencilWrite,
                                       GrColor* overrideColor,
                                       const GrCaps& caps) const {
        // We want to force our primary output to be alpha * Coverage, where alpha is the alpha
        // value of the blend the constant. We should already have valid blend coeff's if we are at
        // a point where we have RGB coverage. We don't need any color stages since the known color
        // output is already baked into the blendConstant.
        *overrideColor = GrColorPackRGBA(fAlpha, fAlpha, fAlpha, fAlpha);
        return GrXferProcessor::kOverrideColor_OptFlag;
}

///////////////////////////////////////////////////////////////////////////////

GrPorterDuffXPFactory::GrPorterDuffXPFactory(SkXfermode::Mode xfermode)
    : fXfermode(xfermode) {
    SkASSERT(fXfermode <= SkXfermode::kLastCoeffMode);
    this->initClassID<GrPorterDuffXPFactory>();
}

sk_sp<GrXPFactory> GrPorterDuffXPFactory::Make(SkXfermode::Mode xfermode) {
    static GrPorterDuffXPFactory gClearPDXPF(SkXfermode::kClear_Mode);
    static GrPorterDuffXPFactory gSrcPDXPF(SkXfermode::kSrc_Mode);
    static GrPorterDuffXPFactory gDstPDXPF(SkXfermode::kDst_Mode);
    static GrPorterDuffXPFactory gSrcOverPDXPF(SkXfermode::kSrcOver_Mode);
    static GrPorterDuffXPFactory gDstOverPDXPF(SkXfermode::kDstOver_Mode);
    static GrPorterDuffXPFactory gSrcInPDXPF(SkXfermode::kSrcIn_Mode);
    static GrPorterDuffXPFactory gDstInPDXPF(SkXfermode::kDstIn_Mode);
    static GrPorterDuffXPFactory gSrcOutPDXPF(SkXfermode::kSrcOut_Mode);
    static GrPorterDuffXPFactory gDstOutPDXPF(SkXfermode::kDstOut_Mode);
    static GrPorterDuffXPFactory gSrcATopPDXPF(SkXfermode::kSrcATop_Mode);
    static GrPorterDuffXPFactory gDstATopPDXPF(SkXfermode::kDstATop_Mode);
    static GrPorterDuffXPFactory gXorPDXPF(SkXfermode::kXor_Mode);
    static GrPorterDuffXPFactory gPlusPDXPF(SkXfermode::kPlus_Mode);
    static GrPorterDuffXPFactory gModulatePDXPF(SkXfermode::kModulate_Mode);
    static GrPorterDuffXPFactory gScreenPDXPF(SkXfermode::kScreen_Mode);

    static GrPorterDuffXPFactory* gFactories[] = {
        &gClearPDXPF, &gSrcPDXPF, &gDstPDXPF, &gSrcOverPDXPF, &gDstOverPDXPF, &gSrcInPDXPF,
        &gDstInPDXPF, &gSrcOutPDXPF, &gDstOutPDXPF, &gSrcATopPDXPF, &gDstATopPDXPF, &gXorPDXPF,
        &gPlusPDXPF, &gModulatePDXPF, &gScreenPDXPF
    };
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gFactories) == SkXfermode::kLastCoeffMode + 1);

    if (xfermode < 0 || xfermode > SkXfermode::kLastCoeffMode) {
        return nullptr;
    }
    return sk_sp<GrXPFactory>(SkRef(gFactories[xfermode]));
}

GrXferProcessor*
GrPorterDuffXPFactory::onCreateXferProcessor(const GrCaps& caps,
                                             const GrPipelineOptimizations& optimizations,
                                             bool hasMixedSamples,
                                             const DstTexture* dstTexture) const {
    if (optimizations.fOverrides.fUsePLSDstRead) {
        return new ShaderPDXferProcessor(dstTexture, hasMixedSamples, fXfermode);
    }
    BlendFormula blendFormula;
    if (optimizations.fCoveragePOI.isFourChannelOutput()) {
        if (SkXfermode::kSrcOver_Mode == fXfermode &&
            kRGBA_GrColorComponentFlags == optimizations.fColorPOI.validFlags() &&
            !caps.shaderCaps()->dualSourceBlendingSupport() &&
            !caps.shaderCaps()->dstReadInShaderSupport()) {
            // If we don't have dual source blending or in shader dst reads, we fall back to this
            // trick for rendering SrcOver LCD text instead of doing a dst copy.
            SkASSERT(!dstTexture || !dstTexture->texture());
            return PDLCDXferProcessor::Create(fXfermode, optimizations.fColorPOI);
        }
        blendFormula = get_lcd_blend_formula(optimizations.fCoveragePOI, fXfermode);
    } else {
        blendFormula = get_blend_formula(optimizations.fColorPOI, optimizations.fCoveragePOI,
                                         hasMixedSamples, fXfermode);
    }

    if (blendFormula.hasSecondaryOutput() && !caps.shaderCaps()->dualSourceBlendingSupport()) {
        return new ShaderPDXferProcessor(dstTexture, hasMixedSamples, fXfermode);
    }

    SkASSERT(!dstTexture || !dstTexture->texture());
    return new PorterDuffXferProcessor(blendFormula);
}

void GrPorterDuffXPFactory::getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                                     InvariantBlendedColor* blendedColor) const {
    // Find the blended color info based on the formula that does not have coverage.
    BlendFormula colorFormula = gBlendTable[colorPOI.isOpaque()][0][fXfermode];
    if (colorFormula.usesDstColor()) {
        blendedColor->fWillBlendWithDst = true;
        blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
        return;
    }

    blendedColor->fWillBlendWithDst = false;

    SkASSERT(kAdd_GrBlendEquation == colorFormula.fBlendEquation);

    switch (colorFormula.fSrcCoeff) {
        case kZero_GrBlendCoeff:
            blendedColor->fKnownColor = 0;
            blendedColor->fKnownColorFlags = kRGBA_GrColorComponentFlags;
            return;

        case kOne_GrBlendCoeff:
            blendedColor->fKnownColor = colorPOI.color();
            blendedColor->fKnownColorFlags = colorPOI.validFlags();
            return;

        default:
            blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
            return;
    }
}

bool GrPorterDuffXPFactory::onWillReadDstColor(const GrCaps& caps,
                                               const GrPipelineOptimizations& optimizations) const {
    if (caps.shaderCaps()->dualSourceBlendingSupport()) {
        return false;
    }

    // When we have four channel coverage we always need to read the dst in order to correctly
    // blend. The one exception is when we are using srcover mode and we know the input color into
    // the XP.
    if (optimizations.fCoveragePOI.isFourChannelOutput()) {
        if (SkXfermode::kSrcOver_Mode == fXfermode &&
            kRGBA_GrColorComponentFlags == optimizations.fColorPOI.validFlags() &&
            !caps.shaderCaps()->dstReadInShaderSupport()) {
            return false;
        }
        return get_lcd_blend_formula(optimizations.fCoveragePOI, fXfermode).hasSecondaryOutput();
    }

    // We fallback on the shader XP when the blend formula would use dual source blending but we
    // don't have support for it.
    static const bool kHasMixedSamples = false;
    SkASSERT(!caps.usesMixedSamples()); // We never use mixed samples without dual source blending.
    return get_blend_formula(optimizations.fColorPOI, optimizations.fCoveragePOI, kHasMixedSamples,
                             fXfermode).hasSecondaryOutput();
}

GR_DEFINE_XP_FACTORY_TEST(GrPorterDuffXPFactory);

sk_sp<GrXPFactory> GrPorterDuffXPFactory::TestCreate(GrProcessorTestData* d) {
    SkXfermode::Mode mode = SkXfermode::Mode(d->fRandom->nextULessThan(SkXfermode::kLastCoeffMode));
    return GrPorterDuffXPFactory::Make(mode);
}

void GrPorterDuffXPFactory::TestGetXPOutputTypes(const GrXferProcessor* xp,
                                                 int* outPrimary,
                                                 int* outSecondary) {
    if (!!strcmp(xp->name(), "Porter Duff")) {
        *outPrimary = *outSecondary = -1;
        return;
    }
    BlendFormula blendFormula = static_cast<const PorterDuffXferProcessor*>(xp)->getBlendFormula();
    *outPrimary = blendFormula.fPrimaryOutputType;
    *outSecondary = blendFormula.fSecondaryOutputType;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// SrcOver Global functions
////////////////////////////////////////////////////////////////////////////////////////////////
const GrXferProcessor& GrPorterDuffXPFactory::SimpleSrcOverXP() {
    static BlendFormula gSrcOverBlendFormula = COEFF_FORMULA(kOne_GrBlendCoeff,
                                                             kISA_GrBlendCoeff);
    static PorterDuffXferProcessor gSrcOverXP(gSrcOverBlendFormula);
    return gSrcOverXP;
}

GrXferProcessor* GrPorterDuffXPFactory::CreateSrcOverXferProcessor(
        const GrCaps& caps,
        const GrPipelineOptimizations& optimizations,
        bool hasMixedSamples,
        const GrXferProcessor::DstTexture* dstTexture) {
    if (optimizations.fOverrides.fUsePLSDstRead) {
        return new ShaderPDXferProcessor(dstTexture, hasMixedSamples, SkXfermode::kSrcOver_Mode);
    }
    if (!optimizations.fCoveragePOI.isFourChannelOutput() &&
        !(optimizations.fCoveragePOI.isSolidWhite() &&
          !hasMixedSamples &&
          optimizations.fColorPOI.isOpaque())) {
        // We return nullptr here, which our caller interprets as meaning "use SimpleSrcOverXP".
        // We don't simply return the address of that XP here because our caller would have to unref
        // it and since it is a global object and GrProgramElement's ref-cnting system is not thread
        // safe.
        return nullptr;
    }

    BlendFormula blendFormula;
    if (optimizations.fCoveragePOI.isFourChannelOutput()) {
        if (kRGBA_GrColorComponentFlags == optimizations.fColorPOI.validFlags() &&
            !caps.shaderCaps()->dualSourceBlendingSupport() &&
            !caps.shaderCaps()->dstReadInShaderSupport()) {
            // If we don't have dual source blending or in shader dst reads, we fall
            // back to this trick for rendering SrcOver LCD text instead of doing a
            // dst copy.
            SkASSERT(!dstTexture || !dstTexture->texture());
            return PDLCDXferProcessor::Create(SkXfermode::kSrcOver_Mode, optimizations.fColorPOI);
        }
        blendFormula = get_lcd_blend_formula(optimizations.fCoveragePOI, SkXfermode::kSrcOver_Mode);
    } else {
        blendFormula = get_blend_formula(optimizations.fColorPOI, optimizations.fCoveragePOI,
                                         hasMixedSamples, SkXfermode::kSrcOver_Mode);
    }

    if (blendFormula.hasSecondaryOutput() && !caps.shaderCaps()->dualSourceBlendingSupport()) {
        return new ShaderPDXferProcessor(dstTexture, hasMixedSamples, SkXfermode::kSrcOver_Mode);
    }

    SkASSERT(!dstTexture || !dstTexture->texture());
    return new PorterDuffXferProcessor(blendFormula);
}

bool GrPorterDuffXPFactory::SrcOverWillNeedDstTexture(const GrCaps& caps,
                                                     const GrPipelineOptimizations& optimizations) {
    if (caps.shaderCaps()->dstReadInShaderSupport() ||
        caps.shaderCaps()->dualSourceBlendingSupport()) {
        return false;
    }

    // When we have four channel coverage we always need to read the dst in order to correctly
    // blend. The one exception is when we are using srcover mode and we know the input color
    // into the XP.
    if (optimizations.fCoveragePOI.isFourChannelOutput()) {
        if (kRGBA_GrColorComponentFlags == optimizations.fColorPOI.validFlags() &&
            !caps.shaderCaps()->dstReadInShaderSupport()) {
            return false;
        }
        return get_lcd_blend_formula(optimizations.fCoveragePOI,
                                     SkXfermode::kSrcOver_Mode).hasSecondaryOutput();
    }

    // We fallback on the shader XP when the blend formula would use dual source blending but we
    // don't have support for it.
    static const bool kHasMixedSamples = false;
    SkASSERT(!caps.usesMixedSamples()); // We never use mixed samples without dual source blending.
    return get_blend_formula(optimizations.fColorPOI, optimizations.fCoveragePOI,
                             kHasMixedSamples, SkXfermode::kSrcOver_Mode).hasSecondaryOutput();
}
