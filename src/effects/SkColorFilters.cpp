
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBlitRow.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#include "SkUtils.h"
#include "SkString.h"
#include "SkValidationUtils.h"

#define ILLEGAL_XFERMODE_MODE   ((SkXfermode::Mode)-1)

// baseclass for filters that store a color and mode
class SkModeColorFilter : public SkColorFilter {
public:
    SkModeColorFilter(SkColor color) {
        fColor = color;
        fMode = ILLEGAL_XFERMODE_MODE;
        this->updateCache();
    }

    SkModeColorFilter(SkColor color, SkXfermode::Mode mode) {
        fColor = color;
        fMode = mode;
        this->updateCache();
    };

    SkColor getColor() const { return fColor; }
    SkXfermode::Mode getMode() const { return fMode; }
    bool isModeValid() const { return ILLEGAL_XFERMODE_MODE != fMode; }
    SkPMColor getPMColor() const { return fPMColor; }

    virtual bool asColorMode(SkColor* color, SkXfermode::Mode* mode) const SK_OVERRIDE {
        if (ILLEGAL_XFERMODE_MODE == fMode) {
            return false;
        }

        if (color) {
            *color = fColor;
        }
        if (mode) {
            *mode = fMode;
        }
        return true;
    }

    virtual uint32_t getFlags() const SK_OVERRIDE {
        return fProc16 ? (kAlphaUnchanged_Flag | kHasFilter16_Flag) : 0;
    }

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) const SK_OVERRIDE {
        SkPMColor       color = fPMColor;
        SkXfermodeProc  proc = fProc;

        for (int i = 0; i < count; i++) {
            result[i] = proc(color, shader[i]);
        }
    }

    virtual void filterSpan16(const uint16_t shader[], int count,
                              uint16_t result[]) const SK_OVERRIDE {
        SkASSERT(this->getFlags() & kHasFilter16_Flag);

        SkPMColor        color = fPMColor;
        SkXfermodeProc16 proc16 = fProc16;

        for (int i = 0; i < count; i++) {
            result[i] = proc16(color, shader[i]);
        }
    }

#ifdef SK_DEVELOPER
    virtual void toString(SkString* str) const SK_OVERRIDE {
        str->append("SkModeColorFilter: color: 0x");
        str->appendHex(fColor);
        str->append(" mode: ");
        str->append(SkXfermode::ModeName(fMode));
    }
#endif

#if SK_SUPPORT_GPU
    virtual GrEffectRef* asNewEffect(GrContext*) const SK_OVERRIDE;
#endif
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkModeColorFilter)

protected:
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {
        this->INHERITED::flatten(buffer);
        buffer.writeColor(fColor);
        buffer.writeUInt(fMode);
    }

    SkModeColorFilter(SkFlattenableReadBuffer& buffer) {
        fColor = buffer.readColor();
        fMode = (SkXfermode::Mode)buffer.readUInt();
        this->updateCache();
        buffer.validate(SkIsValidMode(fMode));
    }

private:
    SkColor             fColor;
    SkXfermode::Mode    fMode;
    // cache
    SkPMColor           fPMColor;
    SkXfermodeProc      fProc;
    SkXfermodeProc16    fProc16;

    void updateCache() {
        fPMColor = SkPreMultiplyColor(fColor);
        fProc = SkXfermode::GetProc(fMode);
        fProc16 = SkXfermode::GetProc16(fMode, fColor);
    }

    typedef SkColorFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "GrBlend.h"
#include "GrEffect.h"
#include "GrEffectUnitTest.h"
#include "GrTBackendEffectFactory.h"
#include "gl/GrGLEffect.h"
#include "SkGr.h"

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
        GrCrash("Unexpected xfer coeff.");
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

class ModeColorFilterEffect : public GrEffect {
public:
    static GrEffectRef* Create(const GrColor& c, SkXfermode::Mode mode) {
        // TODO: Make the effect take the coeffs rather than mode since we already do the
        // conversion here.
        SkXfermode::Coeff srcCoeff, dstCoeff;
        if (!SkXfermode::ModeAsCoeff(mode, &srcCoeff, &dstCoeff)) {
            SkDebugf("Failing to create color filter for mode %d\n", mode);
            return NULL;
        }
        AutoEffectUnref effect(SkNEW_ARGS(ModeColorFilterEffect, (c, mode)));
        return CreateEffectRef(effect);
    }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    bool willUseFilterColor() const {
        SkXfermode::Coeff dstCoeff;
        SkXfermode::Coeff srcCoeff;
        SkAssertResult(SkXfermode::ModeAsCoeff(fMode, &srcCoeff, &dstCoeff));
        if (SkXfermode::kZero_Coeff == srcCoeff) {
            return GrBlendCoeffRefsSrc(sk_blend_to_grblend(dstCoeff));
        }
        return true;
    }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<ModeColorFilterEffect>::getInstance();
    }

    static const char* Name() { return "ModeColorFilterEffect"; }

    SkXfermode::Mode mode() const { return fMode; }
    GrColor color() const { return fColor; }

    class GLEffect : public GrGLEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
            : INHERITED(factory) {
        }

        virtual void emitCode(GrGLShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray& coords,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            SkXfermode::Mode mode = drawEffect.castEffect<ModeColorFilterEffect>().mode();

            SkASSERT(SkXfermode::kDst_Mode != mode);
            const char* colorFilterColorUniName = NULL;
            if (drawEffect.castEffect<ModeColorFilterEffect>().willUseFilterColor()) {
                fFilterColorUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                                      kVec4f_GrSLType, "FilterColor",
                                                      &colorFilterColorUniName);
            }

            GrGLSLExpr4 filter =
                color_filter_expression(mode, GrGLSLExpr4(colorFilterColorUniName), GrGLSLExpr4(inputColor));

            builder->fsCodeAppendf("\t%s = %s;\n", outputColor, filter.c_str());
        }

        static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
            const ModeColorFilterEffect& colorModeFilter = drawEffect.castEffect<ModeColorFilterEffect>();
            // The SL code does not depend on filter color at the moment, so no need to represent it
            // in the key.
            EffectKey modeKey = colorModeFilter.mode();
            return modeKey;
        }

        virtual void setData(const GrGLUniformManager& uman, const GrDrawEffect& drawEffect) SK_OVERRIDE {
            if (fFilterColorUni.isValid()) {
                const ModeColorFilterEffect& colorModeFilter = drawEffect.castEffect<ModeColorFilterEffect>();
                GrGLfloat c[4];
                GrColorToRGBAFloat(colorModeFilter.color(), c);
                uman.set4fv(fFilterColorUni, 1, c);
            }
        }

    private:

        GrGLUniformManager::UniformHandle fFilterColorUni;
        typedef GrGLEffect INHERITED;
    };

    GR_DECLARE_EFFECT_TEST;

private:
    ModeColorFilterEffect(GrColor color, SkXfermode::Mode mode)
        : fMode(mode),
          fColor(color) {

        SkXfermode::Coeff dstCoeff;
        SkXfermode::Coeff srcCoeff;
        SkAssertResult(SkXfermode::ModeAsCoeff(fMode, &srcCoeff, &dstCoeff));
        // These could be calculated from the blend equation with template trickery..
        if (SkXfermode::kZero_Coeff == dstCoeff && !GrBlendCoeffRefsDst(sk_blend_to_grblend(srcCoeff))) {
            this->setWillNotUseInputColor();
        }
    }

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        const ModeColorFilterEffect& s = CastEffect<ModeColorFilterEffect>(other);
        return fMode == s.fMode && fColor == s.fColor;
    }

    SkXfermode::Mode fMode;
    GrColor fColor;

    typedef GrEffect INHERITED;
};

namespace {

/** Function color_component_to_int tries to reproduce the GLSL rounding. The spec doesn't specify
 * to which direction the 0.5 goes.
 */
static inline int color_component_to_int(float value) {
    return sk_float_round2int(GrMax(0.f, GrMin(1.f, value)) * 255.f);
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

void ModeColorFilterEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    float inputColor[4];
    GrColorToRGBAFloat(*color, inputColor);
    float filterColor[4];
    GrColorToRGBAFloat(fColor, filterColor);
    MaskedColorExpr result =
        color_filter_expression(fMode,
                                MaskedColorExpr(filterColor, kRGBA_GrColorComponentFlags),
                                MaskedColorExpr(inputColor, *validFlags));

    *color = result.getColor();
    *validFlags = result.getValidComponents();
}

GR_DEFINE_EFFECT_TEST(ModeColorFilterEffect);
GrEffectRef* ModeColorFilterEffect::TestCreate(SkRandom* rand,
                                    GrContext*,
                                    const GrDrawTargetCaps&,
                                    GrTexture*[]) {
    SkXfermode::Mode mode = SkXfermode::kDst_Mode;
    while (SkXfermode::kDst_Mode == mode) {
        mode = static_cast<SkXfermode::Mode>(rand->nextRangeU(0, SkXfermode::kLastCoeffMode));
    }
    GrColor color = rand->nextU();
    return ModeColorFilterEffect::Create(color, mode);
}

GrEffectRef* SkModeColorFilter::asNewEffect(GrContext*) const {
    if (SkXfermode::kDst_Mode != fMode) {
        return ModeColorFilterEffect::Create(SkColor2GrColor(fColor), fMode);
    }
    return NULL;
}

#endif

///////////////////////////////////////////////////////////////////////////////

class Src_SkModeColorFilter : public SkModeColorFilter {
public:
    Src_SkModeColorFilter(SkColor color) : INHERITED(color, SkXfermode::kSrc_Mode) {}

    virtual uint32_t getFlags() const SK_OVERRIDE {
        if (SkGetPackedA32(this->getPMColor()) == 0xFF) {
            return kAlphaUnchanged_Flag | kHasFilter16_Flag;
        } else {
            return 0;
        }
    }

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) const SK_OVERRIDE {
        sk_memset32(result, this->getPMColor(), count);
    }

    virtual void filterSpan16(const uint16_t shader[], int count,
                              uint16_t result[]) const SK_OVERRIDE {
        SkASSERT(this->getFlags() & kHasFilter16_Flag);
        sk_memset16(result, SkPixel32ToPixel16(this->getPMColor()), count);
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(Src_SkModeColorFilter)

protected:
    Src_SkModeColorFilter(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
    typedef SkModeColorFilter INHERITED;
};

class SrcOver_SkModeColorFilter : public SkModeColorFilter {
public:
    SrcOver_SkModeColorFilter(SkColor color)
            : INHERITED(color, SkXfermode::kSrcOver_Mode) {
        fColor32Proc = SkBlitRow::ColorProcFactory();
    }

    virtual uint32_t getFlags() const SK_OVERRIDE {
        if (SkGetPackedA32(this->getPMColor()) == 0xFF) {
            return kAlphaUnchanged_Flag | kHasFilter16_Flag;
        } else {
            return 0;
        }
    }

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) const SK_OVERRIDE {
        fColor32Proc(result, shader, count, this->getPMColor());
    }

    virtual void filterSpan16(const uint16_t shader[], int count,
                              uint16_t result[]) const SK_OVERRIDE {
        SkASSERT(this->getFlags() & kHasFilter16_Flag);
        sk_memset16(result, SkPixel32ToPixel16(this->getPMColor()), count);
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SrcOver_SkModeColorFilter)

protected:
    SrcOver_SkModeColorFilter(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {
            fColor32Proc = SkBlitRow::ColorProcFactory();
        }

private:

    SkBlitRow::ColorProc fColor32Proc;

    typedef SkModeColorFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkColorFilter* SkColorFilter::CreateModeFilter(SkColor color,
                                               SkXfermode::Mode mode) {
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

static inline unsigned pin(unsigned value, unsigned max) {
    if (value > max) {
        value = max;
    }
    return value;
}

class SkLightingColorFilter : public SkColorFilter {
public:
    SkLightingColorFilter(SkColor mul, SkColor add) : fMul(mul), fAdd(add) {}

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) const SK_OVERRIDE {
        unsigned scaleR = SkAlpha255To256(SkColorGetR(fMul));
        unsigned scaleG = SkAlpha255To256(SkColorGetG(fMul));
        unsigned scaleB = SkAlpha255To256(SkColorGetB(fMul));

        unsigned addR = SkColorGetR(fAdd);
        unsigned addG = SkColorGetG(fAdd);
        unsigned addB = SkColorGetB(fAdd);

        for (int i = 0; i < count; i++) {
            SkPMColor c = shader[i];
            if (c) {
                unsigned a = SkGetPackedA32(c);
                unsigned scaleA = SkAlpha255To256(a);
                unsigned r = pin(SkAlphaMul(SkGetPackedR32(c), scaleR) + SkAlphaMul(addR, scaleA), a);
                unsigned g = pin(SkAlphaMul(SkGetPackedG32(c), scaleG) + SkAlphaMul(addG, scaleA), a);
                unsigned b = pin(SkAlphaMul(SkGetPackedB32(c), scaleB) + SkAlphaMul(addB, scaleA), a);
                c = SkPackARGB32(a, r, g, b);
            }
            result[i] = c;
        }
    }

#ifdef SK_DEVELOPER
    virtual void toString(SkString* str) const SK_OVERRIDE {
        str->append("SkLightingColorFilter: mul: 0x");
        str->appendHex(fMul);
        str->append(" add: 0x");
        str->appendHex(fAdd);
    }
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingColorFilter)

protected:
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {
        this->INHERITED::flatten(buffer);
        buffer.writeColor(fMul);
        buffer.writeColor(fAdd);
    }

    SkLightingColorFilter(SkFlattenableReadBuffer& buffer) {
        fMul = buffer.readColor();
        fAdd = buffer.readColor();
    }

    SkColor fMul, fAdd;

private:
    typedef SkColorFilter INHERITED;
};

class SkLightingColorFilter_JustAdd : public SkLightingColorFilter {
public:
    SkLightingColorFilter_JustAdd(SkColor mul, SkColor add)
        : INHERITED(mul, add) {}

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) const SK_OVERRIDE {
        unsigned addR = SkColorGetR(fAdd);
        unsigned addG = SkColorGetG(fAdd);
        unsigned addB = SkColorGetB(fAdd);

        for (int i = 0; i < count; i++) {
            SkPMColor c = shader[i];
            if (c) {
                unsigned a = SkGetPackedA32(c);
                unsigned scaleA = SkAlpha255To256(a);
                unsigned r = pin(SkGetPackedR32(c) + SkAlphaMul(addR, scaleA), a);
                unsigned g = pin(SkGetPackedG32(c) + SkAlphaMul(addG, scaleA), a);
                unsigned b = pin(SkGetPackedB32(c) + SkAlphaMul(addB, scaleA), a);
                c = SkPackARGB32(a, r, g, b);
            }
            result[i] = c;
        }
    }

#ifdef SK_DEVELOPER
    virtual void toString(SkString* str) const SK_OVERRIDE {
        str->append("SkLightingColorFilter_JustAdd: add: 0x");
        str->appendHex(fAdd);
    }
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingColorFilter_JustAdd)

protected:
    SkLightingColorFilter_JustAdd(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
    typedef SkLightingColorFilter INHERITED;
};

class SkLightingColorFilter_JustMul : public SkLightingColorFilter {
public:
    SkLightingColorFilter_JustMul(SkColor mul, SkColor add)
        : INHERITED(mul, add) {}

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) const SK_OVERRIDE {
        unsigned scaleR = SkAlpha255To256(SkColorGetR(fMul));
        unsigned scaleG = SkAlpha255To256(SkColorGetG(fMul));
        unsigned scaleB = SkAlpha255To256(SkColorGetB(fMul));

        for (int i = 0; i < count; i++) {
            SkPMColor c = shader[i];
            if (c) {
                unsigned a = SkGetPackedA32(c);
                unsigned r = SkAlphaMul(SkGetPackedR32(c), scaleR);
                unsigned g = SkAlphaMul(SkGetPackedG32(c), scaleG);
                unsigned b = SkAlphaMul(SkGetPackedB32(c), scaleB);
                c = SkPackARGB32(a, r, g, b);
            }
            result[i] = c;
        }
    }

#ifdef SK_DEVELOPER
    virtual void toString(SkString* str) const SK_OVERRIDE {
        str->append("SkLightingColorFilter_JustMul: mul: 0x");
        str->appendHex(fMul);
    }
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingColorFilter_JustMul)

protected:
    SkLightingColorFilter_JustMul(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
    typedef SkLightingColorFilter INHERITED;
};

class SkLightingColorFilter_SingleMul : public SkLightingColorFilter {
public:
    SkLightingColorFilter_SingleMul(SkColor mul, SkColor add)
            : INHERITED(mul, add) {
        SkASSERT(SkColorGetR(add) == 0);
        SkASSERT(SkColorGetG(add) == 0);
        SkASSERT(SkColorGetB(add) == 0);
        SkASSERT(SkColorGetR(mul) == SkColorGetG(mul));
        SkASSERT(SkColorGetR(mul) == SkColorGetB(mul));
    }

    virtual uint32_t getFlags() const SK_OVERRIDE {
        return this->INHERITED::getFlags() | (kAlphaUnchanged_Flag | kHasFilter16_Flag);
    }

    virtual void filterSpan16(const uint16_t shader[], int count,
                              uint16_t result[]) const SK_OVERRIDE {
        // all mul components are the same
        unsigned scale = SkAlpha255To256(SkColorGetR(fMul));

        if (count > 0) {
            do {
                *result++ = SkAlphaMulRGB16(*shader++, scale);
            } while (--count > 0);
        }
    }

#ifdef SK_DEVELOPER
    virtual void toString(SkString* str) const SK_OVERRIDE {
        str->append("SkLightingColorFilter_SingleMul: mul: 0x");
        str->appendHex(fMul);
    }
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingColorFilter_SingleMul)

protected:
    SkLightingColorFilter_SingleMul(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
    typedef SkLightingColorFilter INHERITED;
};

class SkLightingColorFilter_NoPin : public SkLightingColorFilter {
public:
    SkLightingColorFilter_NoPin(SkColor mul, SkColor add)
    : INHERITED(mul, add) {}

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) const SK_OVERRIDE {
        unsigned scaleR = SkAlpha255To256(SkColorGetR(fMul));
        unsigned scaleG = SkAlpha255To256(SkColorGetG(fMul));
        unsigned scaleB = SkAlpha255To256(SkColorGetB(fMul));

        unsigned addR = SkColorGetR(fAdd);
        unsigned addG = SkColorGetG(fAdd);
        unsigned addB = SkColorGetB(fAdd);

        for (int i = 0; i < count; i++) {
            SkPMColor c = shader[i];
            if (c) {
                unsigned a = SkGetPackedA32(c);
                unsigned scaleA = SkAlpha255To256(a);
                unsigned r = SkAlphaMul(SkGetPackedR32(c), scaleR) + SkAlphaMul(addR, scaleA);
                unsigned g = SkAlphaMul(SkGetPackedG32(c), scaleG) + SkAlphaMul(addG, scaleA);
                unsigned b = SkAlphaMul(SkGetPackedB32(c), scaleB) + SkAlphaMul(addB, scaleA);
                c = SkPackARGB32(a, r, g, b);
            }
            result[i] = c;
        }
    }

#ifdef SK_DEVELOPER
    virtual void toString(SkString* str) const SK_OVERRIDE {
        str->append("SkLightingColorFilter_NoPin: mul: 0x");
        str->appendHex(fMul);
        str->append(" add: 0x");
        str->appendHex(fAdd);
    }
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingColorFilter_NoPin)

protected:
    SkLightingColorFilter_NoPin(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
    typedef SkLightingColorFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SkSimpleColorFilter : public SkColorFilter {
public:
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW(SkSimpleColorFilter);
    }

#ifdef SK_DEVELOPER
    virtual void toString(SkString* str) const SK_OVERRIDE {
        str->append("SkSimpleColorFilter");
    }
#endif

protected:
    void filterSpan(const SkPMColor src[], int count, SkPMColor
                    result[]) const SK_OVERRIDE {
        if (result != src) {
            memcpy(result, src, count * sizeof(SkPMColor));
        }
    }

    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {}

    virtual Factory getFactory() const {
        return CreateProc;
    }

};

SkColorFilter* SkColorFilter::CreateLightingFilter(SkColor mul, SkColor add) {
    mul &= 0x00FFFFFF;
    add &= 0x00FFFFFF;

    if (0xFFFFFF == mul) {
        if (0 == add) {
            return SkNEW(SkSimpleColorFilter);   // no change to the colors
        } else {
            return SkNEW_ARGS(SkLightingColorFilter_JustAdd, (mul, add));
        }
    }

    if (0 == add) {
        if (SkColorGetR(mul) == SkColorGetG(mul) &&
                SkColorGetR(mul) == SkColorGetB(mul)) {
            return SkNEW_ARGS(SkLightingColorFilter_SingleMul, (mul, add));
        } else {
            return SkNEW_ARGS(SkLightingColorFilter_JustMul, (mul, add));
        }
    }

    if (SkColorGetR(mul) + SkColorGetR(add) <= 255 &&
        SkColorGetG(mul) + SkColorGetG(add) <= 255 &&
        SkColorGetB(mul) + SkColorGetB(add) <= 255) {
            return SkNEW_ARGS(SkLightingColorFilter_NoPin, (mul, add));
    }

    return SkNEW_ARGS(SkLightingColorFilter, (mul, add));
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkModeColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(Src_SkModeColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SrcOver_SkModeColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingColorFilter_JustAdd)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingColorFilter_JustMul)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingColorFilter_SingleMul)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingColorFilter_NoPin)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSimpleColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
