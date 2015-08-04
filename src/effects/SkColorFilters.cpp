/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkModeColorFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkUtils.h"
#include "SkString.h"
#include "SkValidationUtils.h"
#include "SkColorMatrixFilter.h"

bool SkModeColorFilter::asColorMode(SkColor* color, SkXfermode::Mode* mode) const {
    if (color) {
        *color = fColor;
    }
    if (mode) {
        *mode = fMode;
    }
    return true;
}

uint32_t SkModeColorFilter::getFlags() const {
    switch (fMode) {
        case SkXfermode::kDst_Mode:      //!< [Da, Dc]
        case SkXfermode::kSrcATop_Mode:  //!< [Da, Sc * Da + (1 - Sa) * Dc]
            return kAlphaUnchanged_Flag;
        default:
            break;
    }
    return 0;
}

void SkModeColorFilter::filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const {
    SkPMColor       color = fPMColor;
    SkXfermodeProc  proc = fProc;

    for (int i = 0; i < count; i++) {
        result[i] = proc(color, shader[i]);
    }
}

void SkModeColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor(fColor);
    buffer.writeUInt(fMode);
}

void SkModeColorFilter::updateCache() {
    fPMColor = SkPreMultiplyColor(fColor);
    fProc = SkXfermode::GetProc(fMode);
}

SkFlattenable* SkModeColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkColor color = buffer.readColor();
    SkXfermode::Mode mode = (SkXfermode::Mode)buffer.readUInt();
    return SkColorFilter::CreateModeFilter(color, mode);
}

///////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "GrBlend.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrProcessorUnitTest.h"
#include "SkGr.h"
#include "gl/GrGLFragmentProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

namespace {
/**
 * A definition of blend equation for one coefficient. Generates a
 * blend_coeff * value "expression".
 */
template<typename ColorExpr>
static inline ColorExpr blend_term(SkXfermode::Coeff coeff,
                                   const ColorExpr& src,
                                   const ColorExpr& dst,
                                   const ColorExpr& value) {
    switch (coeff) {
    default:
        SkFAIL("Unexpected xfer coeff.");
    case SkXfermode::kZero_Coeff:    /** 0 */
        return ColorExpr(0);
    case SkXfermode::kOne_Coeff:     /** 1 */
        return value;
    case SkXfermode::kSC_Coeff:
        return src * value;
    case SkXfermode::kISC_Coeff:
        return (ColorExpr(1) - src) * dst;
    case SkXfermode::kDC_Coeff:
        return dst * value;
    case SkXfermode::kIDC_Coeff:
        return (ColorExpr(1) - dst) * value;
    case SkXfermode::kSA_Coeff:      /** src alpha */
        return src.a() * value;
    case SkXfermode::kISA_Coeff:     /** inverse src alpha (i.e. 1 - sa) */
        return (typename ColorExpr::AExpr(1) - src.a()) * value;
    case SkXfermode::kDA_Coeff:      /** dst alpha */
        return dst.a() * value;
    case SkXfermode::kIDA_Coeff:     /** inverse dst alpha (i.e. 1 - da) */
        return (typename ColorExpr::AExpr(1) - dst.a()) *  value;
    }
}
/**
 * Creates a color filter expression which modifies the color by
 * the specified color filter.
 */
template <typename ColorExpr>
static inline ColorExpr color_filter_expression(const SkXfermode::Mode& mode,
                                                const ColorExpr& filterColor,
                                                const ColorExpr& inColor) {
    SkXfermode::Coeff colorCoeff;
    SkXfermode::Coeff filterColorCoeff;
    SkAssertResult(SkXfermode::ModeAsCoeff(mode, &filterColorCoeff, &colorCoeff));
    return blend_term(colorCoeff, filterColor, inColor, inColor) +
        blend_term(filterColorCoeff, filterColor, inColor, filterColor);
}

}

class ModeColorFilterEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(const GrColor& c, SkXfermode::Mode mode) {
        // TODO: Make the effect take the coeffs rather than mode since we already do the
        // conversion here.
        SkXfermode::Coeff srcCoeff, dstCoeff;
        if (!SkXfermode::ModeAsCoeff(mode, &srcCoeff, &dstCoeff)) {
//            SkDebugf("Failing to create color filter for mode %d\n", mode);
            return NULL;
        }
        return SkNEW_ARGS(ModeColorFilterEffect, (c, mode));
    }

    bool willUseFilterColor() const {
        SkXfermode::Coeff dstCoeff;
        SkXfermode::Coeff srcCoeff;
        SkAssertResult(SkXfermode::ModeAsCoeff(fMode, &srcCoeff, &dstCoeff));
        if (SkXfermode::kZero_Coeff == srcCoeff) {
            return GrBlendCoeffRefsSrc(sk_blend_to_grblend(dstCoeff));
        }
        return true;
    }

    GrGLFragmentProcessor* createGLInstance() const override {
        return SkNEW_ARGS(GLProcessor, (*this));
    }

    const char* name() const override { return "ModeColorFilterEffect"; }

    SkXfermode::Mode mode() const { return fMode; }
    GrColor color() const { return fColor; }

    class GLProcessor : public GrGLFragmentProcessor {
    public:
        GLProcessor(const GrProcessor&) {
        }

        virtual void emitCode(EmitArgs& args) override {
            SkXfermode::Mode mode = args.fFp.cast<ModeColorFilterEffect>().mode();

            SkASSERT(SkXfermode::kDst_Mode != mode);
            const char* colorFilterColorUniName = NULL;
            if (args.fFp.cast<ModeColorFilterEffect>().willUseFilterColor()) {
                fFilterColorUni = args.fBuilder->addUniform(
                                                      GrGLProgramBuilder::kFragment_Visibility,
                                                      kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                      "FilterColor",
                                                      &colorFilterColorUniName);
            }

            GrGLSLExpr4 filter =
                color_filter_expression(mode, GrGLSLExpr4(colorFilterColorUniName),
                                        GrGLSLExpr4(args.fInputColor));

            args.fBuilder->getFragmentShaderBuilder()->
                    codeAppendf("\t%s = %s;\n", args.fOutputColor, filter.c_str());
        }

        static void GenKey(const GrProcessor& fp, const GrGLSLCaps&,
                           GrProcessorKeyBuilder* b) {
            const ModeColorFilterEffect& colorModeFilter = fp.cast<ModeColorFilterEffect>();
            // The SL code does not depend on filter color at the moment, so no need to represent it
            // in the key.
            b->add32(colorModeFilter.mode());
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrProcessor& fp) override {
            if (fFilterColorUni.isValid()) {
                const ModeColorFilterEffect& colorModeFilter = fp.cast<ModeColorFilterEffect>();
                GrGLfloat c[4];
                GrColorToRGBAFloat(colorModeFilter.color(), c);
                pdman.set4fv(fFilterColorUni, 1, c);
            }
        }

    private:

        GrGLProgramDataManager::UniformHandle fFilterColorUni;
        typedef GrGLFragmentProcessor INHERITED;
    };

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

private:
    ModeColorFilterEffect(GrColor color, SkXfermode::Mode mode)
        : fMode(mode),
          fColor(color) {
        this->initClassID<ModeColorFilterEffect>();
    }

    virtual void onGetGLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const override {
        GLProcessor::GenKey(*this, caps, b);
    }

    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const ModeColorFilterEffect& s = other.cast<ModeColorFilterEffect>();
        return fMode == s.fMode && fColor == s.fColor;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    SkXfermode::Mode fMode;
    GrColor fColor;

    typedef GrFragmentProcessor INHERITED;
};

namespace {

/** Function color_component_to_int tries to reproduce the GLSL rounding. The spec doesn't specify
 * to which direction the 0.5 goes.
 */
static inline int color_component_to_int(float value) {
    return sk_float_round2int(SkTMax(0.f, SkTMin(1.f, value)) * 255.f);
}

/** MaskedColorExpr is used to evaluate the color and valid color component flags through the
 * blending equation. It has members similar to GrGLSLExpr so that it can be used with the
 * templated helpers above.
 */
class MaskedColorExpr {
public:
    MaskedColorExpr(const float color[], uint32_t flags)
        : fFlags(flags) {
        fColor[0] = color[0];
        fColor[1] = color[1];
        fColor[2] = color[2];
        fColor[3] = color[3];
    }

    MaskedColorExpr(float v, uint32_t flags = kRGBA_GrColorComponentFlags)
        : fFlags(flags) {
        fColor[0] = v;
        fColor[1] = v;
        fColor[2] = v;
        fColor[3] = v;
    }

    MaskedColorExpr operator*(const MaskedColorExpr& other) const {
        float tmp[4];
        tmp[0] = fColor[0] * other.fColor[0];
        tmp[1] = fColor[1] * other.fColor[1];
        tmp[2] = fColor[2] * other.fColor[2];
        tmp[3] = fColor[3] * other.fColor[3];

        return MaskedColorExpr(tmp, fFlags & other.fFlags);
    }

    MaskedColorExpr operator+(const MaskedColorExpr& other) const {
        float tmp[4];
        tmp[0] = fColor[0] + other.fColor[0];
        tmp[1] = fColor[1] + other.fColor[1];
        tmp[2] = fColor[2] + other.fColor[2];
        tmp[3] = fColor[3] + other.fColor[3];

        return MaskedColorExpr(tmp, fFlags & other.fFlags);
    }

    MaskedColorExpr operator-(const MaskedColorExpr& other) const {
        float tmp[4];
        tmp[0] = fColor[0] - other.fColor[0];
        tmp[1] = fColor[1] - other.fColor[1];
        tmp[2] = fColor[2] - other.fColor[2];
        tmp[3] = fColor[3] - other.fColor[3];

        return MaskedColorExpr(tmp, fFlags & other.fFlags);
    }

    MaskedColorExpr a() const {
        uint32_t flags = (fFlags & kA_GrColorComponentFlag) ? kRGBA_GrColorComponentFlags : 0;
        return MaskedColorExpr(fColor[3], flags);
    }

    GrColor getColor() const {
        return GrColorPackRGBA(color_component_to_int(fColor[0]),
                               color_component_to_int(fColor[1]),
                               color_component_to_int(fColor[2]),
                               color_component_to_int(fColor[3]));
    }

    uint32_t getValidComponents() const  { return fFlags; }

    typedef MaskedColorExpr AExpr;
private:
    float fColor[4];
    uint32_t fFlags;
};

}

void ModeColorFilterEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    float inputColor[4];
    GrColorToRGBAFloat(inout->color(), inputColor);
    float filterColor[4];
    GrColorToRGBAFloat(fColor, filterColor);
    MaskedColorExpr result =
        color_filter_expression(fMode,
                                MaskedColorExpr(filterColor, kRGBA_GrColorComponentFlags),
                                MaskedColorExpr(inputColor, inout->validFlags()));

    // Check if we will use the input color
    SkXfermode::Coeff dstCoeff;
    SkXfermode::Coeff srcCoeff;
    SkAssertResult(SkXfermode::ModeAsCoeff(fMode, &srcCoeff, &dstCoeff));
    GrInvariantOutput::ReadInput readInput = GrInvariantOutput::kWill_ReadInput;
    // These could be calculated from the blend equation with template trickery..
    if (SkXfermode::kZero_Coeff == dstCoeff &&
        !GrBlendCoeffRefsDst(sk_blend_to_grblend(srcCoeff))) {
        readInput = GrInvariantOutput::kWillNot_ReadInput;
    }
    inout->setToOther(result.getValidComponents(), result.getColor(), readInput);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ModeColorFilterEffect);
GrFragmentProcessor* ModeColorFilterEffect::TestCreate(GrProcessorTestData* d) {
    SkXfermode::Mode mode = SkXfermode::kDst_Mode;
    while (SkXfermode::kDst_Mode == mode) {
        mode = static_cast<SkXfermode::Mode>(d->fRandom->nextRangeU(0, SkXfermode::kLastCoeffMode));
    }

    // pick a random premul color
    uint8_t alpha = d->fRandom->nextULessThan(256);
    GrColor color = GrColorPackRGBA(d->fRandom->nextRangeU(0, alpha),
                                    d->fRandom->nextRangeU(0, alpha),
                                    d->fRandom->nextRangeU(0, alpha),
                                    alpha);
    return ModeColorFilterEffect::Create(color, mode);
}

bool SkModeColorFilter::asFragmentProcessors(GrContext*, GrProcessorDataManager*,
                                             SkTDArray<GrFragmentProcessor*>* array) const {
    if (SkXfermode::kDst_Mode != fMode) {
        GrFragmentProcessor* frag = ModeColorFilterEffect::Create(SkColor2GrColor(fColor), fMode);
        if (frag) {
            if (array) {
                *array->append() = frag;
            } else {
                frag->unref();
                SkDEBUGCODE(frag = NULL;)
            }
            return true;
        }
    }
    return false;
}

#endif

///////////////////////////////////////////////////////////////////////////////

class Src_SkModeColorFilter : public SkModeColorFilter {
public:
    Src_SkModeColorFilter(SkColor color) : INHERITED(color, SkXfermode::kSrc_Mode) {}

    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const override {
        sk_memset32(result, this->getPMColor(), count);
    }

private:
    typedef SkModeColorFilter INHERITED;
};

class SrcOver_SkModeColorFilter : public SkModeColorFilter {
public:
    SrcOver_SkModeColorFilter(SkColor color) : INHERITED(color, SkXfermode::kSrcOver_Mode) { }

    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const override {
        SkBlitRow::Color32(result, shader, count, this->getPMColor());
    }

private:
    typedef SkModeColorFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkColorFilter* SkColorFilter::CreateModeFilter(SkColor color, SkXfermode::Mode mode) {
    if (!SkIsValidMode(mode)) {
        return NULL;
    }

    unsigned alpha = SkColorGetA(color);

    // first collaps some modes if possible

    if (SkXfermode::kClear_Mode == mode) {
        color = 0;
        mode = SkXfermode::kSrc_Mode;
    } else if (SkXfermode::kSrcOver_Mode == mode) {
        if (0 == alpha) {
            mode = SkXfermode::kDst_Mode;
        } else if (255 == alpha) {
            mode = SkXfermode::kSrc_Mode;
        }
        // else just stay srcover
    }

    // weed out combinations that are noops, and just return null
    if (SkXfermode::kDst_Mode == mode ||
        (0 == alpha && (SkXfermode::kSrcOver_Mode == mode ||
                        SkXfermode::kDstOver_Mode == mode ||
                        SkXfermode::kDstOut_Mode == mode ||
                        SkXfermode::kSrcATop_Mode == mode ||
                        SkXfermode::kXor_Mode == mode ||
                        SkXfermode::kDarken_Mode == mode)) ||
            (0xFF == alpha && SkXfermode::kDstIn_Mode == mode)) {
        return NULL;
    }

    switch (mode) {
        case SkXfermode::kSrc_Mode:
            return SkNEW_ARGS(Src_SkModeColorFilter, (color));
        case SkXfermode::kSrcOver_Mode:
            return SkNEW_ARGS(SrcOver_SkModeColorFilter, (color));
        default:
            return SkNEW_ARGS(SkModeColorFilter, (color, mode));
    }
}

///////////////////////////////////////////////////////////////////////////////

static SkScalar byte_to_scale(U8CPU byte) {
    if (0xFF == byte) {
        // want to get this exact
        return 1;
    } else {
        return byte * 0.00392156862745f;
    }
}

SkColorFilter* SkColorFilter::CreateLightingFilter(SkColor mul, SkColor add) {
    SkColorMatrix matrix;
    matrix.setScale(byte_to_scale(SkColorGetR(mul)),
                    byte_to_scale(SkColorGetG(mul)),
                    byte_to_scale(SkColorGetB(mul)),
                    1);
    matrix.postTranslate(SkIntToScalar(SkColorGetR(add)),
                         SkIntToScalar(SkColorGetG(add)),
                         SkIntToScalar(SkColorGetB(add)),
                         0);
    return SkColorMatrixFilter::Create(matrix);
}

