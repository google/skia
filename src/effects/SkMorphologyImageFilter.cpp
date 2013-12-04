/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMorphologyImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#include "SkRect.h"
#include "SkMorphology_opts.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTexture.h"
#include "GrTBackendEffectFactory.h"
#include "gl/GrGLEffect.h"
#include "effects/Gr1DKernelEffect.h"
#include "SkImageFilterUtils.h"
#endif

SkMorphologyImageFilter::SkMorphologyImageFilter(SkFlattenableReadBuffer& buffer)
  : INHERITED(1, buffer) {
    fRadius.fWidth = buffer.readInt();
    fRadius.fHeight = buffer.readInt();
    buffer.validate((fRadius.fWidth >= 0) &&
                    (fRadius.fHeight >= 0));
}

SkMorphologyImageFilter::SkMorphologyImageFilter(int radiusX, int radiusY, SkImageFilter* input, const CropRect* cropRect)
    : INHERITED(input, cropRect), fRadius(SkISize::Make(radiusX, radiusY)) {
}


void SkMorphologyImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt(fRadius.fWidth);
    buffer.writeInt(fRadius.fHeight);
}

enum MorphDirection {
    kX, kY
};

template<MorphDirection direction>
static void erode(const SkPMColor* src, SkPMColor* dst,
                  int radius, int width, int height,
                  int srcStride, int dstStride)
{
    const int srcStrideX = direction == kX ? 1 : srcStride;
    const int dstStrideX = direction == kX ? 1 : dstStride;
    const int srcStrideY = direction == kX ? srcStride : 1;
    const int dstStrideY = direction == kX ? dstStride : 1;
    radius = SkMin32(radius, width - 1);
    const SkPMColor* upperSrc = src + radius * srcStrideX;
    for (int x = 0; x < width; ++x) {
        const SkPMColor* lp = src;
        const SkPMColor* up = upperSrc;
        SkPMColor* dptr = dst;
        for (int y = 0; y < height; ++y) {
            int minB = 255, minG = 255, minR = 255, minA = 255;
            for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                int b = SkGetPackedB32(*p);
                int g = SkGetPackedG32(*p);
                int r = SkGetPackedR32(*p);
                int a = SkGetPackedA32(*p);
                if (b < minB) minB = b;
                if (g < minG) minG = g;
                if (r < minR) minR = r;
                if (a < minA) minA = a;
            }
            *dptr = SkPackARGB32(minA, minR, minG, minB);
            dptr += dstStrideY;
            lp += srcStrideY;
            up += srcStrideY;
        }
        if (x >= radius) src += srcStrideX;
        if (x + radius < width - 1) upperSrc += srcStrideX;
        dst += dstStrideX;
    }
}

static void erodeX(const SkBitmap& src, SkBitmap* dst, int radiusX, const SkIRect& bounds)
{
    SkMorphologyProc erodeXProc = SkMorphologyGetPlatformProc(kErodeX_SkMorphologyProcType);
    if (!erodeXProc) {
        erodeXProc = erode<kX>;
    }
    erodeXProc(src.getAddr32(bounds.left(), bounds.top()), dst->getAddr32(0, 0),
               radiusX, bounds.width(), bounds.height(),
               src.rowBytesAsPixels(), dst->rowBytesAsPixels());
}

static void erodeY(const SkBitmap& src, SkBitmap* dst, int radiusY, const SkIRect& bounds)
{
    SkMorphologyProc erodeYProc = SkMorphologyGetPlatformProc(kErodeY_SkMorphologyProcType);
    if (!erodeYProc) {
        erodeYProc = erode<kY>;
    }
    erodeYProc(src.getAddr32(bounds.left(), bounds.top()), dst->getAddr32(0, 0),
               radiusY, bounds.height(), bounds.width(),
               src.rowBytesAsPixels(), dst->rowBytesAsPixels());
}

template<MorphDirection direction>
static void dilate(const SkPMColor* src, SkPMColor* dst,
                   int radius, int width, int height,
                   int srcStride, int dstStride)
{
    const int srcStrideX = direction == kX ? 1 : srcStride;
    const int dstStrideX = direction == kX ? 1 : dstStride;
    const int srcStrideY = direction == kX ? srcStride : 1;
    const int dstStrideY = direction == kX ? dstStride : 1;
    radius = SkMin32(radius, width - 1);
    const SkPMColor* upperSrc = src + radius * srcStrideX;
    for (int x = 0; x < width; ++x) {
        const SkPMColor* lp = src;
        const SkPMColor* up = upperSrc;
        SkPMColor* dptr = dst;
        for (int y = 0; y < height; ++y) {
            int maxB = 0, maxG = 0, maxR = 0, maxA = 0;
            for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                int b = SkGetPackedB32(*p);
                int g = SkGetPackedG32(*p);
                int r = SkGetPackedR32(*p);
                int a = SkGetPackedA32(*p);
                if (b > maxB) maxB = b;
                if (g > maxG) maxG = g;
                if (r > maxR) maxR = r;
                if (a > maxA) maxA = a;
            }
            *dptr = SkPackARGB32(maxA, maxR, maxG, maxB);
            dptr += dstStrideY;
            lp += srcStrideY;
            up += srcStrideY;
        }
        if (x >= radius) src += srcStrideX;
        if (x + radius < width - 1) upperSrc += srcStrideX;
        dst += dstStrideX;
    }
}

static void dilateX(const SkBitmap& src, SkBitmap* dst, int radiusX, const SkIRect& bounds)
{
    SkMorphologyProc dilateXProc = SkMorphologyGetPlatformProc(kDilateX_SkMorphologyProcType);
    if (!dilateXProc) {
        dilateXProc = dilate<kX>;
    }
    dilateXProc(src.getAddr32(bounds.left(), bounds.top()), dst->getAddr32(0, 0),
                radiusX, bounds.width(), bounds.height(),
                src.rowBytesAsPixels(), dst->rowBytesAsPixels());
}

static void dilateY(const SkBitmap& src, SkBitmap* dst, int radiusY, const SkIRect& bounds)
{
    SkMorphologyProc dilateYProc = SkMorphologyGetPlatformProc(kDilateY_SkMorphologyProcType);
    if (!dilateYProc) {
        dilateYProc = dilate<kY>;
    }
    dilateYProc(src.getAddr32(bounds.left(), bounds.top()), dst->getAddr32(0, 0),
                radiusY, bounds.height(), bounds.width(),
                src.rowBytesAsPixels(), dst->rowBytesAsPixels());
}

bool SkErodeImageFilter::onFilterImage(Proxy* proxy,
                                       const SkBitmap& source, const SkMatrix& ctm,
                                       SkBitmap* dst, SkIPoint* offset) {
    SkBitmap src = source;
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, ctm, &src, offset)) {
        return false;
    }

    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }

    SkIRect bounds;
    src.getBounds(&bounds);
    if (!this->applyCropRect(&bounds, ctm)) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    dst->setConfig(src.config(), bounds.width(), bounds.height());
    dst->allocPixels();
    if (!dst->getPixels()) {
        return false;
    }

    int width = radius().width();
    int height = radius().height();

    if (width < 0 || height < 0) {
        return false;
    }

    if (width == 0 && height == 0) {
        src.extractSubset(dst, bounds);
        offset->fX += bounds.left();
        offset->fY += bounds.top();
        return true;
    }

    SkBitmap temp;
    temp.setConfig(dst->config(), dst->width(), dst->height());
    if (!temp.allocPixels()) {
        return false;
    }

    if (width > 0 && height > 0) {
        erodeX(src, &temp, width, bounds);
        SkIRect tmpBounds = SkIRect::MakeWH(bounds.width(), bounds.height());
        erodeY(temp, dst, height, tmpBounds);
    } else if (width > 0) {
        erodeX(src, dst, width, bounds);
    } else if (height > 0) {
        erodeY(src, dst, height, bounds);
    }
    offset->fX += bounds.left();
    offset->fY += bounds.top();
    return true;
}

bool SkDilateImageFilter::onFilterImage(Proxy* proxy,
                                        const SkBitmap& source, const SkMatrix& ctm,
                                        SkBitmap* dst, SkIPoint* offset) {
    SkBitmap src = source;
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, ctm, &src, offset)) {
        return false;
    }
    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }

    SkIRect bounds;
    src.getBounds(&bounds);
    if (!this->applyCropRect(&bounds, ctm)) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    dst->setConfig(src.config(), bounds.width(), bounds.height());
    dst->allocPixels();
    if (!dst->getPixels()) {
        return false;
    }

    int width = radius().width();
    int height = radius().height();

    if (width < 0 || height < 0) {
        return false;
    }

    if (width == 0 && height == 0) {
        src.extractSubset(dst, bounds);
        offset->fX += bounds.left();
        offset->fY += bounds.top();
        return true;
    }

    SkBitmap temp;
    temp.setConfig(dst->config(), dst->width(), dst->height());
    if (!temp.allocPixels()) {
        return false;
    }

    if (width > 0 && height > 0) {
        dilateX(src, &temp, width, bounds);
        SkIRect tmpBounds = SkIRect::MakeWH(bounds.width(), bounds.height());
        dilateY(temp, dst, height, tmpBounds);
    } else if (width > 0) {
        dilateX(src, dst, width, bounds);
    } else if (height > 0) {
        dilateY(src, dst, height, bounds);
    }
    offset->fX += bounds.left();
    offset->fY += bounds.top();
    return true;
}

#if SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////

class GrGLMorphologyEffect;

/**
 * Morphology effects. Depending upon the type of morphology, either the
 * component-wise min (Erode_Type) or max (Dilate_Type) of all pixels in the
 * kernel is selected as the new color. The new color is modulated by the input
 * color.
 */
class GrMorphologyEffect : public Gr1DKernelEffect {

public:

    enum MorphologyType {
        kErode_MorphologyType,
        kDilate_MorphologyType,
    };

    static GrEffectRef* Create(GrTexture* tex, Direction dir, int radius, MorphologyType type) {
        AutoEffectUnref effect(SkNEW_ARGS(GrMorphologyEffect, (tex, dir, radius, type)));
        return CreateEffectRef(effect);
    }

    virtual ~GrMorphologyEffect();

    MorphologyType type() const { return fType; }

    static const char* Name() { return "Morphology"; }

    typedef GrGLMorphologyEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

protected:

    MorphologyType fType;

private:
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    GrMorphologyEffect(GrTexture*, Direction, int radius, MorphologyType);

    GR_DECLARE_EFFECT_TEST;

    typedef Gr1DKernelEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLMorphologyEffect : public GrGLEffect {
public:
    GrGLMorphologyEffect (const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    int width() const { return GrMorphologyEffect::WidthFromRadius(fRadius); }

    int                                 fRadius;
    GrMorphologyEffect::MorphologyType  fType;
    GrGLUniformManager::UniformHandle   fImageIncrementUni;

    typedef GrGLEffect INHERITED;
};

GrGLMorphologyEffect::GrGLMorphologyEffect(const GrBackendEffectFactory& factory,
                                           const GrDrawEffect& drawEffect)
    : INHERITED(factory) {
    const GrMorphologyEffect& m = drawEffect.castEffect<GrMorphologyEffect>();
    fRadius = m.radius();
    fType = m.type();
}

void GrGLMorphologyEffect::emitCode(GrGLShaderBuilder* builder,
                                    const GrDrawEffect&,
                                    EffectKey key,
                                    const char* outputColor,
                                    const char* inputColor,
                                    const TransformedCoordsArray& coords,
                                    const TextureSamplerArray& samplers) {
    SkString coords2D = builder->ensureFSCoords2D(coords, 0);
    fImageIncrementUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                             kVec2f_GrSLType, "ImageIncrement");

    const char* func;
    switch (fType) {
        case GrMorphologyEffect::kErode_MorphologyType:
            builder->fsCodeAppendf("\t\t%s = vec4(1, 1, 1, 1);\n", outputColor);
            func = "min";
            break;
        case GrMorphologyEffect::kDilate_MorphologyType:
            builder->fsCodeAppendf("\t\t%s = vec4(0, 0, 0, 0);\n", outputColor);
            func = "max";
            break;
        default:
            GrCrash("Unexpected type");
            func = ""; // suppress warning
            break;
    }
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);

    builder->fsCodeAppendf("\t\tvec2 coord = %s - %d.0 * %s;\n", coords2D.c_str(), fRadius, imgInc);
    builder->fsCodeAppendf("\t\tfor (int i = 0; i < %d; i++) {\n", this->width());
    builder->fsCodeAppendf("\t\t\t%s = %s(%s, ", outputColor, func, outputColor);
    builder->fsAppendTextureLookup(samplers[0], "coord");
    builder->fsCodeAppend(");\n");
    builder->fsCodeAppendf("\t\t\tcoord += %s;\n", imgInc);
    builder->fsCodeAppend("\t\t}\n");
    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, 2, outputColor, inputColor);
    builder->fsCodeAppend(modulate.c_str());
}

GrGLEffect::EffectKey GrGLMorphologyEffect::GenKey(const GrDrawEffect& drawEffect,
                                                   const GrGLCaps&) {
    const GrMorphologyEffect& m = drawEffect.castEffect<GrMorphologyEffect>();
    EffectKey key = static_cast<EffectKey>(m.radius());
    key |= (m.type() << 8);
    return key;
}

void GrGLMorphologyEffect::setData(const GrGLUniformManager& uman,
                                   const GrDrawEffect& drawEffect) {
    const Gr1DKernelEffect& kern = drawEffect.castEffect<Gr1DKernelEffect>();
    GrTexture& texture = *kern.texture(0);
    // the code we generated was for a specific kernel radius
    SkASSERT(kern.radius() == fRadius);
    float imageIncrement[2] = { 0 };
    switch (kern.direction()) {
        case Gr1DKernelEffect::kX_Direction:
            imageIncrement[0] = 1.0f / texture.width();
            break;
        case Gr1DKernelEffect::kY_Direction:
            imageIncrement[1] = 1.0f / texture.height();
            break;
        default:
            GrCrash("Unknown filter direction.");
    }
    uman.set2fv(fImageIncrementUni, 1, imageIncrement);
}

///////////////////////////////////////////////////////////////////////////////

GrMorphologyEffect::GrMorphologyEffect(GrTexture* texture,
                                       Direction direction,
                                       int radius,
                                       MorphologyType type)
    : Gr1DKernelEffect(texture, direction, radius)
    , fType(type) {
}

GrMorphologyEffect::~GrMorphologyEffect() {
}

const GrBackendEffectFactory& GrMorphologyEffect::getFactory() const {
    return GrTBackendEffectFactory<GrMorphologyEffect>::getInstance();
}

bool GrMorphologyEffect::onIsEqual(const GrEffect& sBase) const {
    const GrMorphologyEffect& s = CastEffect<GrMorphologyEffect>(sBase);
    return (this->texture(0) == s.texture(0) &&
            this->radius() == s.radius() &&
            this->direction() == s.direction() &&
            this->type() == s.type());
}

void GrMorphologyEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    // This is valid because the color components of the result of the kernel all come
    // exactly from existing values in the source texture.
    this->updateConstantColorComponentsForModulation(color, validFlags);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrMorphologyEffect);

GrEffectRef* GrMorphologyEffect::TestCreate(SkRandom* random,
                                            GrContext*,
                                            const GrDrawTargetCaps&,
                                            GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
    Direction dir = random->nextBool() ? kX_Direction : kY_Direction;
    static const int kMaxRadius = 10;
    int radius = random->nextRangeU(1, kMaxRadius);
    MorphologyType type = random->nextBool() ? GrMorphologyEffect::kErode_MorphologyType :
                                               GrMorphologyEffect::kDilate_MorphologyType;

    return GrMorphologyEffect::Create(textures[texIdx], dir, radius, type);
}

namespace {

void apply_morphology_pass(GrContext* context,
                           GrTexture* texture,
                           const SkIRect& srcRect,
                           const SkIRect& dstRect,
                           int radius,
                           GrMorphologyEffect::MorphologyType morphType,
                           Gr1DKernelEffect::Direction direction) {
    GrPaint paint;
    paint.addColorEffect(GrMorphologyEffect::Create(texture,
                                                    direction,
                                                    radius,
                                                    morphType))->unref();
    context->drawRectToRect(paint, SkRect::Make(dstRect), SkRect::Make(srcRect));
}

bool apply_morphology(const SkBitmap& input,
                      const SkIRect& rect,
                      GrMorphologyEffect::MorphologyType morphType,
                      SkISize radius,
                      SkBitmap* dst) {
    GrTexture* srcTexture = input.getTexture();
    SkASSERT(NULL != srcTexture);
    GrContext* context = srcTexture->getContext();
    srcTexture->ref();
    SkAutoTUnref<GrTexture> src(srcTexture);

    GrContext::AutoMatrix am;
    am.setIdentity(context);

    GrContext::AutoClip acs(context, SkRect::MakeWH(SkIntToScalar(srcTexture->width()),
                                                    SkIntToScalar(srcTexture->height())));

    SkIRect dstRect = SkIRect::MakeWH(rect.width(), rect.height());
    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fWidth = rect.width();
    desc.fHeight = rect.height();
    desc.fConfig = kSkia8888_GrPixelConfig;
    SkIRect srcRect = rect;

    if (radius.fWidth > 0) {
        GrAutoScratchTexture ast(context, desc);
        GrContext::AutoRenderTarget art(context, ast.texture()->asRenderTarget());
        apply_morphology_pass(context, src, srcRect, dstRect, radius.fWidth,
                              morphType, Gr1DKernelEffect::kX_Direction);
        SkIRect clearRect = SkIRect::MakeXYWH(dstRect.fLeft, dstRect.fBottom,
                                              dstRect.width(), radius.fHeight);
        context->clear(&clearRect, GrMorphologyEffect::kErode_MorphologyType == morphType ?
                                   SK_ColorWHITE :
                                   SK_ColorTRANSPARENT, false);
        src.reset(ast.detach());
        srcRect = dstRect;
    }
    if (radius.fHeight > 0) {
        GrAutoScratchTexture ast(context, desc);
        GrContext::AutoRenderTarget art(context, ast.texture()->asRenderTarget());
        apply_morphology_pass(context, src, srcRect, dstRect, radius.fHeight,
                              morphType, Gr1DKernelEffect::kY_Direction);
        src.reset(ast.detach());
    }
    return SkImageFilterUtils::WrapTexture(src, rect.width(), rect.height(), dst);
}

};

bool SkDilateImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                                         SkBitmap* result, SkIPoint* offset) {
    SkBitmap input;
    if (!SkImageFilterUtils::GetInputResultGPU(getInput(0), proxy, src, ctm, &input, offset)) {
        return false;
    }
    SkIRect bounds;
    src.getBounds(&bounds);
    if (!this->applyCropRect(&bounds, ctm)) {
        return false;
    }
    int width = radius().width();
    int height = radius().height();

    if (width < 0 || height < 0) {
        return false;
    }

    if (width == 0 && height == 0) {
        src.extractSubset(result, bounds);
        offset->fX += bounds.left();
        offset->fY += bounds.top();
        return true;
    }

    if (!apply_morphology(input, bounds, GrMorphologyEffect::kDilate_MorphologyType, radius(), result)) {
        return false;
    }
    offset->fX += bounds.left();
    offset->fY += bounds.top();
    return true;
}

bool SkErodeImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                                        SkBitmap* result, SkIPoint* offset) {
    SkBitmap input;
    if (!SkImageFilterUtils::GetInputResultGPU(getInput(0), proxy, src, ctm, &input, offset)) {
        return false;
    }
    SkIRect bounds;
    src.getBounds(&bounds);
    if (!this->applyCropRect(&bounds, ctm)) {
        return false;
    }
    int width = radius().width();
    int height = radius().height();

    if (width < 0 || height < 0) {
        return false;
    }

    if (width == 0 && height == 0) {
        src.extractSubset(result, bounds);
        offset->fX += bounds.left();
        offset->fY += bounds.top();
        return true;
    }

    if (!apply_morphology(input, bounds, GrMorphologyEffect::kErode_MorphologyType, radius(), result)) {
        return false;
    }
    offset->fX += bounds.left();
    offset->fY += bounds.top();
    return true;
}

#endif
