/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLumaXfermode.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#include "SkString.h"

#if SK_SUPPORT_GPU
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "GrContext.h"
#include "GrTBackendEffectFactory.h"
#endif

static inline SkPMColor luma_proc(const SkPMColor a, const SkPMColor b) {
    unsigned luma = SkComputeLuminance(SkGetPackedR32(b),
                                       SkGetPackedG32(b),
                                       SkGetPackedB32(b));
    return SkAlphaMulQ(a, SkAlpha255To256(luma));
}

template <typename T>
static inline const T* lumaOpA(SkXfermode::Mode mode,
                               const T* src, const T* dst) {
    return SkXfermode::kSrcIn_Mode == mode ? src : dst;
}

template <typename T>
static inline const T* lumaOpB(SkXfermode::Mode mode,
                               const T* src, const T* dst) {
    return SkXfermode::kSrcIn_Mode == mode ? dst : src;
}

SkXfermode* SkLumaMaskXfermode::Create(SkXfermode::Mode mode) {
    if (kSrcIn_Mode == mode || kDstIn_Mode == mode) {
        return SkNEW_ARGS(SkLumaMaskXfermode, (mode));
    }
    return NULL;
}

SkLumaMaskXfermode::SkLumaMaskXfermode(SkXfermode::Mode mode)
    : fMode(mode) {
    SkASSERT(kSrcIn_Mode == mode || kDstIn_Mode == mode);
}

SkLumaMaskXfermode::SkLumaMaskXfermode(SkFlattenableReadBuffer& buffer)
    : INHERITED(buffer)
    , fMode((SkXfermode::Mode)buffer.readUInt()) {
    SkASSERT(kSrcIn_Mode == fMode || kDstIn_Mode == fMode);
}

void SkLumaMaskXfermode::flatten(SkFlattenableWriteBuffer& buffer) const {
    INHERITED::flatten(buffer);
    buffer.writeUInt(fMode);
}

SkPMColor SkLumaMaskXfermode::xferColor(SkPMColor src, SkPMColor dst) const {
    const SkPMColor* a = lumaOpA<SkPMColor>(fMode, &src, &dst);
    const SkPMColor* b = lumaOpB<SkPMColor>(fMode, &src, &dst);
    return luma_proc(*a, *b);
}

void SkLumaMaskXfermode::xfer32(SkPMColor dst[], const SkPMColor src[],
                                int count, const SkAlpha aa[]) const {
    const SkPMColor* a = lumaOpA<SkPMColor>(fMode, src, dst);
    const SkPMColor* b = lumaOpB<SkPMColor>(fMode, src, dst);

    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned cov = aa[i];
            if (cov) {
                unsigned resC = luma_proc(a[i], b[i]);
                if (cov < 255) {
                    resC = SkFastFourByteInterp256(resC, dst[i],
                                                   SkAlpha255To256(cov));
                }
                dst[i] = resC;
            }
        }
    } else {
        for (int i = 0; i < count; ++i) {
            dst[i] = luma_proc(a[i], b[i]);
        }
    }
}

#ifdef SK_DEVELOPER
void SkLumaMaskXfermode::toString(SkString* str) const {
    str->printf("SkLumaMaskXfermode: mode: %s",
                fMode == kSrcIn_Mode ? "SRC_IN" : "DST_IN");
}
#endif

#if SK_SUPPORT_GPU
//////////////////////////////////////////////////////////////////////////////

class GrGLLumaMaskEffect : public GrGLEffect {
public:
    GrGLLumaMaskEffect(const GrBackendEffectFactory&, const GrDrawEffect&);
    virtual ~GrGLLumaMaskEffect();

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

private:
    typedef GrGLEffect INHERITED;
};

class GrLumaMaskEffect : public GrEffect {
public:
    static GrEffectRef* Create(SkXfermode::Mode mode) {
        AutoEffectUnref effect(SkNEW_ARGS(GrLumaMaskEffect, (mode)));
        return CreateEffectRef(effect);
    }

    virtual ~GrLumaMaskEffect();

    typedef GrGLLumaMaskEffect GLEffect;
    static const char* Name() { return "LumaMask"; }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    virtual void getConstantColorComponents(GrColor*, uint32_t*) const SK_OVERRIDE;

    SkXfermode::Mode getMode() const { return fMode; }

private:
    GrLumaMaskEffect(SkXfermode::Mode);

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    const SkXfermode::Mode fMode;
};

//////////////////////////////////////////////////////////////////////////////

GrGLLumaMaskEffect::GrGLLumaMaskEffect(const GrBackendEffectFactory& factory,
                                       const GrDrawEffect&)
   : INHERITED(factory) {
}

GrGLLumaMaskEffect::~GrGLLumaMaskEffect() {
}

void GrGLLumaMaskEffect::emitCode(GrGLShaderBuilder* builder,
                                    const GrDrawEffect& effect,
                                    EffectKey key,
                                    const char* outputColor,
                                    const char* inputColor,
                                    const TextureSamplerArray& samplers) {

    const GrLumaMaskEffect& lumaEffect = effect.castEffect<GrLumaMaskEffect>();
    const char* dstColor = builder->dstColor();
    SkASSERT(NULL != dstColor);
    if (NULL == inputColor) {
        inputColor = GrGLSLOnesVecf(4);
    }

    const char *opA = lumaOpA<char>(lumaEffect.getMode(), inputColor, dstColor);
    const char *opB = lumaOpB<char>(lumaEffect.getMode(), inputColor, dstColor);

    builder->fsCodeAppendf("\t\tfloat luma = dot(vec3(%f, %f, %f), %s.rgb); \n",
                           SK_ITU_BT709_LUM_COEFF_R,
                           SK_ITU_BT709_LUM_COEFF_G,
                           SK_ITU_BT709_LUM_COEFF_B,
                           opB);
    builder->fsCodeAppendf("\t\t%s = %s * luma;\n", outputColor, opA);
}

GrGLEffect::EffectKey GrGLLumaMaskEffect::GenKey(const GrDrawEffect& drawEffect,
                                                 const GrGLCaps&) {
    const GrLumaMaskEffect& effect = drawEffect.castEffect<GrLumaMaskEffect>();
    return (EffectKey)effect.getMode();
}

//////////////////////////////////////////////////////////////////////////////

GrLumaMaskEffect::GrLumaMaskEffect(SkXfermode::Mode mode)
    : fMode(mode) {
    this->setWillReadDstColor();
}

GrLumaMaskEffect::~GrLumaMaskEffect() {
}

const GrBackendEffectFactory& GrLumaMaskEffect::getFactory() const {
    return GrTBackendEffectFactory<GrLumaMaskEffect>::getInstance();
}

void GrLumaMaskEffect::getConstantColorComponents(GrColor*, uint32_t *validFlags) const {
    *validFlags = 0;
}

bool GrLumaMaskEffect::onIsEqual(const GrEffect& sBase) const {
    return fMode == CastEffect<GrLumaMaskEffect>(sBase).fMode;
}

//////////////////////////////////////////////////////////////////////////////

bool SkLumaMaskXfermode::asNewEffectOrCoeff(GrContext*, GrEffectRef** effect,
                                            Coeff*, Coeff*,
                                            GrTexture* background) const {
    // No background texture support.
    if (effect && !background) {
        *effect = GrLumaMaskEffect::Create(fMode);
        return true;
    }

    return false;
}

#endif // SK_SUPPORT_GPU
