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
#include "gl/GrGLProgramStage.h"
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
  : fMode(mode), fBackground(background), fForeground(foreground)
{
    SkASSERT(NULL != background);
    SkSafeRef(fBackground);
    SkSafeRef(fForeground);
}

SkBlendImageFilter::~SkBlendImageFilter() {
    SkSafeUnref(fBackground);
    SkSafeUnref(fForeground);
}

SkBlendImageFilter::SkBlendImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(buffer)
{
    fMode = (SkBlendImageFilter::Mode) buffer.readInt();
    fBackground = buffer.readFlattenableT<SkImageFilter>();
    if (buffer.readBool()) {
        fForeground = buffer.readFlattenableT<SkImageFilter>();
    } else {
        fForeground = NULL;
    }
}

void SkBlendImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt((int) fMode);
    buffer.writeFlattenable(fBackground);
    buffer.writeBool(NULL != fForeground);
    if (NULL != fForeground) {
        buffer.writeFlattenable(fForeground);
    }
}

bool SkBlendImageFilter::onFilterImage(Proxy* proxy,
                                       const SkBitmap& src,
                                       const SkMatrix& ctm,
                                       SkBitmap* dst,
                                       SkIPoint* offset) {
    SkBitmap background, foreground = src;
    SkASSERT(NULL != fBackground);
    if (!fBackground->filterImage(proxy, src, ctm, &background, offset)) {
        return false;
    }
    if (fForeground && !fForeground->filterImage(proxy, src, ctm, &foreground, offset)) {
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
class GrGLBlendEffect  : public GrGLProgramStage {
public:
    GrGLBlendEffect(const GrProgramStageFactory& factory,
                    const GrCustomStage& stage);
    virtual ~GrGLBlendEffect();

    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const TextureSamplerArray&) SK_OVERRIDE;

    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE {}

    static inline StageKey GenKey(const GrCustomStage& s, const GrGLCaps&);

private:
    typedef GrGLProgramStage INHERITED;
    SkBlendImageFilter::Mode fMode;
};

///////////////////////////////////////////////////////////////////////////////

class GrBlendEffect : public GrSingleTextureEffect {
public:
    GrBlendEffect(SkBlendImageFilter::Mode mode, GrTexture* foreground);
    virtual ~GrBlendEffect();

    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;
    const GrProgramStageFactory& getFactory() const;
    SkBlendImageFilter::Mode mode() const { return fMode; }

    typedef GrGLBlendEffect GLProgramStage;
    static const char* Name() { return "Blend"; }

private:
    typedef GrSingleTextureEffect INHERITED;
    SkBlendImageFilter::Mode fMode;
};

// FIXME:  This should be refactored with SkSingleInputImageFilter's version.
static GrTexture* getInputResultAsTexture(SkImageFilter* input,
                                          GrTexture* src,
                                          const SkRect& rect) {
    GrTexture* resultTex;
    if (!input) {
        resultTex = src;
    } else if (input->canFilterImageGPU()) {
        // onFilterImageGPU() already refs the result, so just return it here.
        return input->onFilterImageGPU(src, rect);
    } else {
        SkBitmap srcBitmap, result;
        srcBitmap.setConfig(SkBitmap::kARGB_8888_Config, src->width(), src->height());
        srcBitmap.setPixelRef(new SkGrPixelRef(src))->unref();
        SkIPoint offset;
        if (input->filterImage(NULL, srcBitmap, SkMatrix(), &result, &offset)) {
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

GrTexture* SkBlendImageFilter::onFilterImageGPU(GrTexture* src, const SkRect& rect) {
    SkAutoTUnref<GrTexture> background(getInputResultAsTexture(fBackground, src, rect));
    SkAutoTUnref<GrTexture> foreground(getInputResultAsTexture(fForeground, src, rect));
    GrContext* context = src->getContext();

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fWidth = SkScalarCeilToInt(rect.width());
    desc.fHeight = SkScalarCeilToInt(rect.height());
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    GrAutoScratchTexture ast(context, desc);
    GrTexture* dst = ast.detach();
    GrContext::AutoMatrix avm(context, GrMatrix::I());
    GrContext::AutoRenderTarget art(context, dst->asRenderTarget());
    GrContext::AutoClip ac(context, rect);

    GrMatrix sampleM;
    sampleM.setIDiv(background->width(), background->height());
    GrPaint paint;
    paint.reset();
    paint.textureSampler(0)->reset(sampleM);
    paint.textureSampler(0)->setCustomStage(SkNEW_ARGS(GrSingleTextureEffect, (background.get())))->unref();
    paint.textureSampler(1)->reset(sampleM);
    paint.textureSampler(1)->setCustomStage(SkNEW_ARGS(GrBlendEffect, (fMode, foreground.get())))->unref();
    context->drawRect(paint, rect);
    return dst;
}

///////////////////////////////////////////////////////////////////////////////

GrBlendEffect::GrBlendEffect(SkBlendImageFilter::Mode mode, GrTexture* foreground)
    : INHERITED(foreground), fMode(mode) {
}

GrBlendEffect::~GrBlendEffect() {
}

bool GrBlendEffect::isEqual(const GrCustomStage& sBase) const {
    const GrBlendEffect& s = static_cast<const GrBlendEffect&>(sBase);
    return INHERITED::isEqual(sBase) &&
           fMode == s.fMode;
}

const GrProgramStageFactory& GrBlendEffect::getFactory() const {
    return GrTProgramStageFactory<GrBlendEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GrGLBlendEffect::GrGLBlendEffect(const GrProgramStageFactory& factory,
                                 const GrCustomStage& stage)
    : GrGLProgramStage(factory),
      fMode(static_cast<const GrBlendEffect&>(stage).mode()) {
}

GrGLBlendEffect::~GrGLBlendEffect() {
}

void GrGLBlendEffect::emitFS(GrGLShaderBuilder* builder,
                             const char* outputColor,
                             const char* inputColor,
                             const TextureSamplerArray& samplers) {
    SkString* code = &builder->fFSCode;
    const char* bgColor = inputColor;
    const char* fgColor = "fgColor";
    code->appendf("\t\tvec4 %s = ", fgColor);
    builder->appendTextureLookup(code, samplers[0]);
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

GrGLProgramStage::StageKey GrGLBlendEffect::GenKey(const GrCustomStage& s, const GrGLCaps&) {
    return static_cast<const GrBlendEffect&>(s).mode();
}
#endif

SK_DEFINE_FLATTENABLE_REGISTRAR(SkBlendImageFilter)
