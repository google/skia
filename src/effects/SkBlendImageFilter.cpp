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
#include "SkGr.h"
#include "SkGrPixelRef.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "effects/GrSingleTextureEffect.h"
#include "GrTBackendEffectFactory.h"
#endif

namespace {

SkXfermode::Mode modeToXfermode(SkBlendImageFilter::Mode mode)
{
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

SkPMColor multiply_proc(SkPMColor src, SkPMColor dst) {
    int omsa = 255 - SkGetPackedA32(src);
    int sr = SkGetPackedR32(src), sg = SkGetPackedG32(src), sb = SkGetPackedB32(src);
    int omda = 255 - SkGetPackedA32(dst);
    int dr = SkGetPackedR32(dst), dg = SkGetPackedG32(dst), db = SkGetPackedB32(dst);
    int a = 255 - SkMulDiv255Round(omsa, omda);
    int r = SkMulDiv255Round(omsa, dr) + SkMulDiv255Round(omda, sr) + SkMulDiv255Round(sr, dr);
    int g = SkMulDiv255Round(omsa, dg) + SkMulDiv255Round(omda, sg) + SkMulDiv255Round(sg, dg);
    int b = SkMulDiv255Round(omsa, db) + SkMulDiv255Round(omda, sb) + SkMulDiv255Round(sb, db);
    return SkPackARGB32(a, r, g, b);
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
    // FEBlend's multiply mode is (1 - Sa) * Da + (1 - Da) * Sc + Sc * Dc
    // Skia's is just Sc * Dc.  So we use a custom proc to implement FEBlend's
    // version.
    if (fMode == SkBlendImageFilter::kMultiply_Mode) {
        paint.setXfermode(new SkProcXfermode(multiply_proc))->unref();
    } else {
        paint.setXfermodeMode(modeToXfermode(fMode));
    }
    canvas.drawBitmap(foreground, 0, 0, &paint);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
class GrGLBlendEffect : public GrGLEffect {
public:
    GrGLBlendEffect(const GrBackendEffectFactory& factory,
                    const GrEffect& effect);
    virtual ~GrGLBlendEffect();

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrEffectStage&,
                          EffectKey,
                          const char* vertexCoords,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrEffectStage&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrEffectStage&);

private:
    typedef GrGLEffect INHERITED;
    SkBlendImageFilter::Mode fMode;
    GrGLEffectMatrix fEffectMatrix;
};

///////////////////////////////////////////////////////////////////////////////

class GrBlendEffect : public GrSingleTextureEffect {
public:
    GrBlendEffect(SkBlendImageFilter::Mode mode, GrTexture* foreground, const SkMatrix&);
    virtual ~GrBlendEffect();

    virtual bool isEqual(const GrEffect&) const SK_OVERRIDE;
    const GrBackendEffectFactory& getFactory() const;
    SkBlendImageFilter::Mode mode() const { return fMode; }

    typedef GrGLBlendEffect GLEffect;
    static const char* Name() { return "Blend"; }

private:
    typedef GrSingleTextureEffect INHERITED;
    SkBlendImageFilter::Mode fMode;
};

// FIXME:  This should be refactored with SkSingleInputImageFilter's version.
static GrTexture* getInputResultAsTexture(SkImageFilter::Proxy* proxy,
                                          SkImageFilter* input,
                                          GrTexture* src,
                                          const SkRect& rect) {
    GrTexture* resultTex;
    if (!input) {
        resultTex = src;
    } else if (input->canFilterImageGPU()) {
        // onFilterImageGPU() already refs the result, so just return it here.
        return input->onFilterImageGPU(proxy, src, rect);
    } else {
        SkBitmap srcBitmap, result;
        srcBitmap.setConfig(SkBitmap::kARGB_8888_Config, src->width(), src->height());
        srcBitmap.setPixelRef(new SkGrPixelRef(src))->unref();
        SkIPoint offset;
        if (input->filterImage(proxy, srcBitmap, SkMatrix(), &result, &offset)) {
            if (result.getTexture()) {
                resultTex = (GrTexture*) result.getTexture();
            } else {
                resultTex = GrLockCachedBitmapTexture(src->getContext(), result, NULL);
                SkSafeRef(resultTex);
                GrUnlockCachedBitmapTexture(resultTex);
                return resultTex;
            }
        } else {
            resultTex = src;
        }
    }
    SkSafeRef(resultTex);
    return resultTex;
}

GrTexture* SkBlendImageFilter::onFilterImageGPU(Proxy* proxy, GrTexture* src, const SkRect& rect) {
    SkAutoTUnref<GrTexture> background(getInputResultAsTexture(proxy, getBackgroundInput(), src, rect));
    SkAutoTUnref<GrTexture> foreground(getInputResultAsTexture(proxy, getForegroundInput(), src, rect));
    GrContext* context = src->getContext();

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fWidth = SkScalarCeilToInt(rect.width());
    desc.fHeight = SkScalarCeilToInt(rect.height());
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    GrAutoScratchTexture ast(context, desc);
    GrTexture* dst = ast.detach();

    GrContext::AutoMatrix am;
    am.setIdentity(context);

    GrContext::AutoRenderTarget art(context, dst->asRenderTarget());
    GrContext::AutoClip ac(context, rect);

    SkMatrix backgroundTexMatrix, foregroundTexMatrix;
    backgroundTexMatrix.setIDiv(background->width(), background->height());
    foregroundTexMatrix.setIDiv(foreground->width(), foreground->height());
    GrPaint paint;
    paint.colorStage(0)->setEffect(
        SkNEW_ARGS(GrSingleTextureEffect, (background.get(), backgroundTexMatrix)))->unref();
    paint.colorStage(1)->setEffect(
        SkNEW_ARGS(GrBlendEffect, (fMode, foreground.get(), foregroundTexMatrix)))->unref();
    context->drawRect(paint, rect);
    return dst;
}

///////////////////////////////////////////////////////////////////////////////

GrBlendEffect::GrBlendEffect(SkBlendImageFilter::Mode mode,
                             GrTexture* foreground,
                             const SkMatrix& matrix)
    : INHERITED(foreground, matrix), fMode(mode) {
}

GrBlendEffect::~GrBlendEffect() {
}

bool GrBlendEffect::isEqual(const GrEffect& sBase) const {
    const GrBlendEffect& s = static_cast<const GrBlendEffect&>(sBase);
    return INHERITED::isEqual(sBase) && fMode == s.fMode;
}

const GrBackendEffectFactory& GrBlendEffect::getFactory() const {
    return GrTBackendEffectFactory<GrBlendEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GrGLBlendEffect::GrGLBlendEffect(const GrBackendEffectFactory& factory,
                                 const GrEffect& effect)
    : INHERITED(factory),
      fMode(static_cast<const GrBlendEffect&>(effect).mode()) {
}

GrGLBlendEffect::~GrGLBlendEffect() {
}

void GrGLBlendEffect::emitCode(GrGLShaderBuilder* builder,
                               const GrEffectStage&,
                               EffectKey key,
                               const char* vertexCoords,
                               const char* outputColor,
                               const char* inputColor,
                               const TextureSamplerArray& samplers) {
    const char* coords;
    GrSLType coordsType =  fEffectMatrix.emitCode(builder, key, vertexCoords, &coords);

    SkString* code = &builder->fFSCode;
    const char* bgColor = inputColor;
    const char* fgColor = "fgColor";
    code->appendf("\t\tvec4 %s = ", fgColor);
    builder->appendTextureLookup(code, samplers[0], coords, coordsType);
    code->append(";\n");
    code->appendf("\t\t%s.a = 1.0 - (1.0 - %s.a) * (1.0 - %s.b);\n", outputColor, bgColor, fgColor);
    switch (fMode) {
      case SkBlendImageFilter::kNormal_Mode:
        code->appendf("\t\t%s.rgb = (1.0 - %s.a) * %s.rgb + %s.rgb;\n", outputColor, fgColor, bgColor, fgColor);
        break;
      case SkBlendImageFilter::kMultiply_Mode:
        code->appendf("\t\t%s.rgb = (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb + %s.rgb * %s.rgb;\n", outputColor, fgColor, bgColor, bgColor, fgColor, fgColor, bgColor);
        break;
      case SkBlendImageFilter::kScreen_Mode:
        code->appendf("\t\t%s.rgb = %s.rgb + %s.rgb - %s.rgb * %s.rgb;\n", outputColor, bgColor, fgColor, fgColor, bgColor);
        break;
      case SkBlendImageFilter::kDarken_Mode:
        code->appendf("\t\t%s.rgb = min((1.0 - %s.a) * %s.rgb + %s.rgb, (1.0 - %s.a) * %s.rgb + %s.rgb);\n", outputColor, fgColor, bgColor, fgColor, bgColor, fgColor, bgColor);
        break;
      case SkBlendImageFilter::kLighten_Mode:
        code->appendf("\t\t%s.rgb = max((1.0 - %s.a) * %s.rgb + %s.rgb, (1.0 - %s.a) * %s.rgb + %s.rgb);\n", outputColor, fgColor, bgColor, fgColor, bgColor, fgColor, bgColor);
        break;
    }
}

void GrGLBlendEffect::setData(const GrGLUniformManager& uman, const GrEffectStage& stage) {
    const GrBlendEffect& blend = static_cast<const GrBlendEffect&>(*stage.getEffect());
    fEffectMatrix.setData(uman, blend.getMatrix(), stage.getCoordChangeMatrix(), blend.texture(0));
}

GrGLEffect::EffectKey GrGLBlendEffect::GenKey(const GrEffectStage& stage, const GrGLCaps&) {
    const GrBlendEffect& blend = static_cast<const GrBlendEffect&>(*stage.getEffect());
    EffectKey key =
        GrGLEffectMatrix::GenKey(blend.getMatrix(), stage.getCoordChangeMatrix(), blend.texture(0));
    key |= (blend.mode() << GrGLEffectMatrix::kKeyBits);
    return key;
}
#endif
