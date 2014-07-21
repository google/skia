/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMorphologyImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkRect.h"
#include "SkMorphology_opts.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTexture.h"
#include "GrTBackendEffectFactory.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLShaderBuilder.h"
#include "effects/Gr1DKernelEffect.h"
#endif

SkMorphologyImageFilter::SkMorphologyImageFilter(SkReadBuffer& buffer)
  : INHERITED(1, buffer) {
    fRadius.fWidth = buffer.readInt();
    fRadius.fHeight = buffer.readInt();
    buffer.validate((fRadius.fWidth >= 0) &&
                    (fRadius.fHeight >= 0));
}

SkMorphologyImageFilter::SkMorphologyImageFilter(int radiusX,
                                                 int radiusY,
                                                 SkImageFilter* input,
                                                 const CropRect* cropRect)
    : INHERITED(1, &input, cropRect), fRadius(SkISize::Make(radiusX, radiusY)) {
}


void SkMorphologyImageFilter::flatten(SkWriteBuffer& buffer) const {
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

static void callProcX(SkMorphologyImageFilter::Proc procX, const SkBitmap& src, SkBitmap* dst, int radiusX, const SkIRect& bounds)
{
    procX(src.getAddr32(bounds.left(), bounds.top()), dst->getAddr32(0, 0),
          radiusX, bounds.width(), bounds.height(),
          src.rowBytesAsPixels(), dst->rowBytesAsPixels());
}

static void callProcY(SkMorphologyImageFilter::Proc procY, const SkBitmap& src, SkBitmap* dst, int radiusY, const SkIRect& bounds)
{
    procY(src.getAddr32(bounds.left(), bounds.top()), dst->getAddr32(0, 0),
          radiusY, bounds.height(), bounds.width(),
          src.rowBytesAsPixels(), dst->rowBytesAsPixels());
}

bool SkMorphologyImageFilter::filterImageGeneric(SkMorphologyImageFilter::Proc procX,
                                                 SkMorphologyImageFilter::Proc procY,
                                                 Proxy* proxy,
                                                 const SkBitmap& source,
                                                 const Context& ctx,
                                                 SkBitmap* dst,
                                                 SkIPoint* offset) const {
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, ctx, &src, &srcOffset)) {
        return false;
    }

    if (src.colorType() != kN32_SkColorType) {
        return false;
    }

    SkIRect bounds;
    if (!this->applyCropRect(ctx, proxy, src, &srcOffset, &bounds, &src)) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    if (!dst->allocPixels(src.info().makeWH(bounds.width(), bounds.height()))) {
        return false;
    }

    SkVector radius = SkVector::Make(SkIntToScalar(this->radius().width()),
                                     SkIntToScalar(this->radius().height()));
    ctx.ctm().mapVectors(&radius, 1);
    int width = SkScalarFloorToInt(radius.fX);
    int height = SkScalarFloorToInt(radius.fY);

    if (width < 0 || height < 0) {
        return false;
    }

    SkIRect srcBounds = bounds;
    srcBounds.offset(-srcOffset);

    if (width == 0 && height == 0) {
        src.extractSubset(dst, srcBounds);
        offset->fX = bounds.left();
        offset->fY = bounds.top();
        return true;
    }

    SkBitmap temp;
    if (!temp.allocPixels(dst->info())) {
        return false;
    }

    if (width > 0 && height > 0) {
        callProcX(procX, src, &temp, width, srcBounds);
        SkIRect tmpBounds = SkIRect::MakeWH(srcBounds.width(), srcBounds.height());
        callProcY(procY, temp, dst, height, tmpBounds);
    } else if (width > 0) {
        callProcX(procX, src, dst, width, srcBounds);
    } else if (height > 0) {
        callProcY(procY, src, dst, height, srcBounds);
    }
    offset->fX = bounds.left();
    offset->fY = bounds.top();
    return true;
}

bool SkErodeImageFilter::onFilterImage(Proxy* proxy,
                                       const SkBitmap& source, const Context& ctx,
                                       SkBitmap* dst, SkIPoint* offset) const {
    Proc erodeXProc = SkMorphologyGetPlatformProc(kErodeX_SkMorphologyProcType);
    if (!erodeXProc) {
        erodeXProc = erode<kX>;
    }
    Proc erodeYProc = SkMorphologyGetPlatformProc(kErodeY_SkMorphologyProcType);
    if (!erodeYProc) {
        erodeYProc = erode<kY>;
    }
    return this->filterImageGeneric(erodeXProc, erodeYProc, proxy, source, ctx, dst, offset);
}

bool SkDilateImageFilter::onFilterImage(Proxy* proxy,
                                        const SkBitmap& source, const Context& ctx,
                                        SkBitmap* dst, SkIPoint* offset) const {
    Proc dilateXProc = SkMorphologyGetPlatformProc(kDilateX_SkMorphologyProcType);
    if (!dilateXProc) {
        dilateXProc = dilate<kX>;
    }
    Proc dilateYProc = SkMorphologyGetPlatformProc(kDilateY_SkMorphologyProcType);
    if (!dilateYProc) {
        dilateYProc = dilate<kY>;
    }
    return this->filterImageGeneric(dilateXProc, dilateYProc, proxy, source, ctx, dst, offset);
}

void SkMorphologyImageFilter::computeFastBounds(const SkRect& src, SkRect* dst) const {
    if (getInput(0)) {
        getInput(0)->computeFastBounds(src, dst);
    } else {
        *dst = src;
    }
    dst->outset(SkIntToScalar(fRadius.width()), SkIntToScalar(fRadius.height()));
}

bool SkMorphologyImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                             SkIRect* dst) const {
    SkIRect bounds = src;
    if (getInput(0) && !getInput(0)->filterBounds(src, ctm, &bounds)) {
        return false;
    }
    SkVector radius = SkVector::Make(SkIntToScalar(this->radius().width()),
                                     SkIntToScalar(this->radius().height()));
    ctm.mapVectors(&radius, 1);
    bounds.outset(SkScalarCeilToInt(radius.x()), SkScalarCeilToInt(radius.y()));
    *dst = bounds;
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

    static GrEffect* Create(GrTexture* tex, Direction dir, int radius, MorphologyType type) {
        return SkNEW_ARGS(GrMorphologyEffect, (tex, dir, radius, type));
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
                          const GrEffectKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder* b);

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
                                    const GrEffectKey& key,
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
            SkFAIL("Unexpected type");
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

void GrGLMorphologyEffect::GenKey(const GrDrawEffect& drawEffect,
                                  const GrGLCaps&, GrEffectKeyBuilder* b) {
    const GrMorphologyEffect& m = drawEffect.castEffect<GrMorphologyEffect>();
    uint32_t key = static_cast<uint32_t>(m.radius());
    key |= (m.type() << 8);
    b->add32(key);
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
            SkFAIL("Unknown filter direction.");
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

GrEffect* GrMorphologyEffect::TestCreate(SkRandom* random,
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
    SkImageFilter::WrapTexture(src, rect.width(), rect.height(), dst);
    return true;
}

};

bool SkMorphologyImageFilter::filterImageGPUGeneric(bool dilate,
                                                    Proxy* proxy,
                                                    const SkBitmap& src,
                                                    const Context& ctx,
                                                    SkBitmap* result,
                                                    SkIPoint* offset) const {
    SkBitmap input = src;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (getInput(0) && !getInput(0)->getInputResultGPU(proxy, src, ctx, &input, &srcOffset)) {
        return false;
    }
    SkIRect bounds;
    if (!this->applyCropRect(ctx, proxy, input, &srcOffset, &bounds, &input)) {
        return false;
    }
    SkVector radius = SkVector::Make(SkIntToScalar(this->radius().width()),
                                     SkIntToScalar(this->radius().height()));
    ctx.ctm().mapVectors(&radius, 1);
    int width = SkScalarFloorToInt(radius.fX);
    int height = SkScalarFloorToInt(radius.fY);

    if (width < 0 || height < 0) {
        return false;
    }

    SkIRect srcBounds = bounds;
    srcBounds.offset(-srcOffset);
    if (width == 0 && height == 0) {
        input.extractSubset(result, srcBounds);
        offset->fX = bounds.left();
        offset->fY = bounds.top();
        return true;
    }

    GrMorphologyEffect::MorphologyType type = dilate ? GrMorphologyEffect::kDilate_MorphologyType : GrMorphologyEffect::kErode_MorphologyType;
    if (!apply_morphology(input, srcBounds, type,
                          SkISize::Make(width, height), result)) {
        return false;
    }
    offset->fX = bounds.left();
    offset->fY = bounds.top();
    return true;
}

bool SkDilateImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                                         SkBitmap* result, SkIPoint* offset) const {
    return this->filterImageGPUGeneric(true, proxy, src, ctx, result, offset);
}

bool SkErodeImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                                        SkBitmap* result, SkIPoint* offset) const {
    return this->filterImageGPUGeneric(false, proxy, src, ctx, result, offset);
}

#endif
