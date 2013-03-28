/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlendImageFilter.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "GrTBackendEffectFactory.h"
#include "SkImageFilterUtils.h"
#endif

namespace {

SkXfermode::Mode modeToXfermode(SkBlendImageFilter::Mode mode) {
    switch (mode) {
        case SkBlendImageFilter::kNormal_Mode:
            return SkXfermode::kSrcOver_Mode;
        case SkBlendImageFilter::kMultiply_Mode:
            return SkXfermode::kMultiply_Mode;
        case SkBlendImageFilter::kScreen_Mode:
            return SkXfermode::kScreen_Mode;
        case SkBlendImageFilter::kDarken_Mode:
            return SkXfermode::kDarken_Mode;
        case SkBlendImageFilter::kLighten_Mode:
            return SkXfermode::kLighten_Mode;
    }
    SkASSERT(0);
    return SkXfermode::kSrcOver_Mode;
}

};

///////////////////////////////////////////////////////////////////////////////

SkBlendImageFilter::SkBlendImageFilter(SkBlendImageFilter::Mode mode, SkImageFilter* background, SkImageFilter* foreground)
  : INHERITED(background, foreground), fMode(mode)
{
}

SkBlendImageFilter::~SkBlendImageFilter() {
}

SkBlendImageFilter::SkBlendImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer)
{
    fMode = (SkBlendImageFilter::Mode) buffer.readInt();
}

void SkBlendImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt((int) fMode);
}

bool SkBlendImageFilter::onFilterImage(Proxy* proxy,
                                       const SkBitmap& src,
                                       const SkMatrix& ctm,
                                       SkBitmap* dst,
                                       SkIPoint* offset) {
    SkBitmap background, foreground = src;
    SkImageFilter* backgroundInput = getBackgroundInput();
    SkImageFilter* foregroundInput = getForegroundInput();
    SkASSERT(NULL != backgroundInput);
    if (!backgroundInput->filterImage(proxy, src, ctm, &background, offset)) {
        return false;
    }
    if (foregroundInput && !foregroundInput->filterImage(proxy, src, ctm, &foreground, offset)) {
        return false;
    }
    SkAutoLockPixels alp_foreground(foreground), alp_background(background);
    if (!foreground.getPixels() || !background.getPixels()) {
        return false;
    }
    dst->setConfig(background.config(), background.width(), background.height());
    dst->allocPixels();
    SkCanvas canvas(*dst);
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    canvas.drawBitmap(background, 0, 0, &paint);
    paint.setXfermodeMode(modeToXfermode(fMode));
    canvas.drawBitmap(foreground, 0, 0, &paint);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
class GrGLBlendEffect : public GrGLEffect {
public:
    GrGLBlendEffect(const GrBackendEffectFactory&, const GrDrawEffect&);
    virtual ~GrGLBlendEffect();

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    static const GrEffect::CoordsType kCoordsType = GrEffect::kLocal_CoordsType;

    SkBlendImageFilter::Mode    fMode;
    GrGLEffectMatrix            fForegroundEffectMatrix;
    GrGLEffectMatrix            fBackgroundEffectMatrix;

    typedef GrGLEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrBlendEffect : public GrEffect {
public:
    static GrEffectRef* Create(SkBlendImageFilter::Mode mode,
                               GrTexture* foreground,
                               GrTexture* background) {
        AutoEffectUnref effect(SkNEW_ARGS(GrBlendEffect, (mode, foreground, background)));
        return CreateEffectRef(effect);
    }

    virtual ~GrBlendEffect();

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    SkBlendImageFilter::Mode mode() const { return fMode; }

    typedef GrGLBlendEffect GLEffect;
    static const char* Name() { return "Blend"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

private:
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    GrBlendEffect(SkBlendImageFilter::Mode mode, GrTexture* foreground, GrTexture* background);
    GrTextureAccess             fForegroundAccess;
    GrTextureAccess             fBackgroundAccess;
    SkBlendImageFilter::Mode    fMode;

    typedef GrEffect INHERITED;
};

bool SkBlendImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, SkBitmap* result) {
    SkBitmap backgroundBM;
    if (!SkImageFilterUtils::GetInputResultGPU(getBackgroundInput(), proxy, src, &backgroundBM)) {
        return false;
    }
    GrTexture* background = (GrTexture*) backgroundBM.getTexture();
    SkBitmap foregroundBM;
    if (!SkImageFilterUtils::GetInputResultGPU(getForegroundInput(), proxy, src, &foregroundBM)) {
        return false;
    }
    GrTexture* foreground = (GrTexture*) foregroundBM.getTexture();
    GrContext* context = foreground->getContext();

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fWidth = src.width();
    desc.fHeight = src.height();
    desc.fConfig = kSkia8888_GrPixelConfig;

    GrAutoScratchTexture ast(context, desc);
    SkAutoTUnref<GrTexture> dst(ast.detach());

    GrContext::AutoRenderTarget art(context, dst->asRenderTarget());

    GrPaint paint;
    paint.colorStage(0)->setEffect(
        GrBlendEffect::Create(fMode, foreground, background))->unref();
    SkRect srcRect;
    src.getBounds(&srcRect);
    context->drawRect(paint, srcRect);
    return SkImageFilterUtils::WrapTexture(dst, src.width(), src.height(), result);
}

///////////////////////////////////////////////////////////////////////////////

GrBlendEffect::GrBlendEffect(SkBlendImageFilter::Mode mode,
                             GrTexture* foreground,
                             GrTexture* background)
    : fForegroundAccess(foreground)
    , fBackgroundAccess(background)
    , fMode(mode) {
    this->addTextureAccess(&fForegroundAccess);
    this->addTextureAccess(&fBackgroundAccess);
}

GrBlendEffect::~GrBlendEffect() {
}

bool GrBlendEffect::onIsEqual(const GrEffect& sBase) const {
    const GrBlendEffect& s = CastEffect<GrBlendEffect>(sBase);
    return fForegroundAccess.getTexture() == s.fForegroundAccess.getTexture() &&
           fBackgroundAccess.getTexture() == s.fBackgroundAccess.getTexture() &&
           fMode == s.fMode;
}

const GrBackendEffectFactory& GrBlendEffect::getFactory() const {
    return GrTBackendEffectFactory<GrBlendEffect>::getInstance();
}

void GrBlendEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    // The output alpha is always 1 - (1 - FGa) * (1 - BGa). So if either FGa or BGa is known to
    // be one then the output alpha is one. (This effect ignores its input. We should have a way to
    // communicate this.)
    if (GrPixelConfigIsOpaque(fForegroundAccess.getTexture()->config()) ||
        GrPixelConfigIsOpaque(fBackgroundAccess.getTexture()->config())) {
        *validFlags = kA_GrColorComponentFlag;
        *color = GrColorPackRGBA(0, 0, 0, 0xff);
    } else {
        *validFlags = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

GrGLBlendEffect::GrGLBlendEffect(const GrBackendEffectFactory& factory,
                                 const GrDrawEffect& drawEffect)
    : INHERITED(factory)
    , fMode(drawEffect.castEffect<GrBlendEffect>().mode())
    , fForegroundEffectMatrix(kCoordsType)
    , fBackgroundEffectMatrix(kCoordsType) {
}

GrGLBlendEffect::~GrGLBlendEffect() {
}

void GrGLBlendEffect::emitCode(GrGLShaderBuilder* builder,
                               const GrDrawEffect&,
                               EffectKey key,
                               const char* outputColor,
                               const char* inputColor,
                               const TextureSamplerArray& samplers) {
    const char* fgCoords;
    const char* bgCoords;
    GrSLType fgCoordsType = fForegroundEffectMatrix.emitCode(builder, key, &fgCoords, NULL, "FG");
    GrSLType bgCoordsType = fBackgroundEffectMatrix.emitCode(builder, key, &bgCoords, NULL, "BG");

    const char* bgColor = "bgColor";
    const char* fgColor = "fgColor";

    builder->fsCodeAppendf("\t\tvec4 %s = ", fgColor);
    builder->appendTextureLookup(GrGLShaderBuilder::kFragment_ShaderType,
                                 samplers[0],
                                 fgCoords,
                                 fgCoordsType);
    builder->fsCodeAppend(";\n");

    builder->fsCodeAppendf("\t\tvec4 %s = ", bgColor);
    builder->appendTextureLookup(GrGLShaderBuilder::kFragment_ShaderType,
                                 samplers[1],
                                 bgCoords,
                                 bgCoordsType);
    builder->fsCodeAppendf(";\n");

    builder->fsCodeAppendf("\t\t%s.a = 1.0 - (1.0 - %s.a) * (1.0 - %s.b);\n", outputColor, bgColor, fgColor);
    switch (fMode) {
      case SkBlendImageFilter::kNormal_Mode:
        builder->fsCodeAppendf("\t\t%s.rgb = (1.0 - %s.a) * %s.rgb + %s.rgb;\n", outputColor, fgColor, bgColor, fgColor);
        break;
      case SkBlendImageFilter::kMultiply_Mode:
        builder->fsCodeAppendf("\t\t%s.rgb = (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb + %s.rgb * %s.rgb;\n", outputColor, fgColor, bgColor, bgColor, fgColor, fgColor, bgColor);
        break;
      case SkBlendImageFilter::kScreen_Mode:
        builder->fsCodeAppendf("\t\t%s.rgb = %s.rgb + %s.rgb - %s.rgb * %s.rgb;\n", outputColor, bgColor, fgColor, fgColor, bgColor);
        break;
      case SkBlendImageFilter::kDarken_Mode:
        builder->fsCodeAppendf("\t\t%s.rgb = min((1.0 - %s.a) * %s.rgb + %s.rgb, (1.0 - %s.a) * %s.rgb + %s.rgb);\n", outputColor, fgColor, bgColor, fgColor, bgColor, fgColor, bgColor);
        break;
      case SkBlendImageFilter::kLighten_Mode:
        builder->fsCodeAppendf("\t\t%s.rgb = max((1.0 - %s.a) * %s.rgb + %s.rgb, (1.0 - %s.a) * %s.rgb + %s.rgb);\n", outputColor, fgColor, bgColor, fgColor, bgColor, fgColor, bgColor);
        break;
    }
}

void GrGLBlendEffect::setData(const GrGLUniformManager& uman, const GrDrawEffect& drawEffect) {
    const GrBlendEffect& blend = drawEffect.castEffect<GrBlendEffect>();
    GrTexture* fgTex = blend.texture(0);
    GrTexture* bgTex = blend.texture(1);
    fForegroundEffectMatrix.setData(uman,
                                    GrEffect::MakeDivByTextureWHMatrix(fgTex),
                                    drawEffect,
                                    fgTex);
    fBackgroundEffectMatrix.setData(uman,
                                    GrEffect::MakeDivByTextureWHMatrix(bgTex),
                                    drawEffect,
                                    bgTex);

}

GrGLEffect::EffectKey GrGLBlendEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
    const GrBlendEffect& blend = drawEffect.castEffect<GrBlendEffect>();

    GrTexture* fgTex = blend.texture(0);
    GrTexture* bgTex = blend.texture(1);

    EffectKey fgKey = GrGLEffectMatrix::GenKey(GrEffect::MakeDivByTextureWHMatrix(fgTex),
                                               drawEffect,
                                               kCoordsType,
                                               fgTex);

    EffectKey bgKey = GrGLEffectMatrix::GenKey(GrEffect::MakeDivByTextureWHMatrix(bgTex),
                                               drawEffect,
                                               kCoordsType,
                                               bgTex);
    bgKey <<= GrGLEffectMatrix::kKeyBits;
    EffectKey modeKey = blend.mode() << (2 * GrGLEffectMatrix::kKeyBits);

    return  modeKey | bgKey | fgKey;
}
#endif
