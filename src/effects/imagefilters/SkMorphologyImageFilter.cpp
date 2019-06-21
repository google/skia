/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkMorphologyImageFilter.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkRect.h"
#include "include/private/SkColorData.h"
#include "src/core/SkImageFilterPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "include/gpu/GrTexture.h"
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#endif

sk_sp<SkImageFilter> SkDilateImageFilter::Make(int radiusX, int radiusY,
                                               sk_sp<SkImageFilter> input,
                                               const CropRect* cropRect) {
    if (radiusX < 0 || radiusY < 0) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkDilateImageFilter(radiusX, radiusY,
                                                        std::move(input),
                                                        cropRect));
}


sk_sp<SkImageFilter> SkErodeImageFilter::Make(int radiusX, int radiusY,
                                              sk_sp<SkImageFilter> input,
                                              const CropRect* cropRect) {
    if (radiusX < 0 || radiusY < 0) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkErodeImageFilter(radiusX, radiusY,
                                                       std::move(input),
                                                       cropRect));
}

SkMorphologyImageFilter::SkMorphologyImageFilter(int radiusX,
                                                 int radiusY,
                                                 sk_sp<SkImageFilter> input,
                                                 const CropRect* cropRect)
    : INHERITED(&input, 1, cropRect)
    , fRadius(SkISize::Make(radiusX, radiusY)) {
}

void SkMorphologyImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt(fRadius.fWidth);
    buffer.writeInt(fRadius.fHeight);
}

static void call_proc_X(SkMorphologyImageFilter::Proc procX,
                        const SkBitmap& src, SkBitmap* dst,
                        int radiusX, const SkIRect& bounds) {
    procX(src.getAddr32(bounds.left(), bounds.top()), dst->getAddr32(0, 0),
          radiusX, bounds.width(), bounds.height(),
          src.rowBytesAsPixels(), dst->rowBytesAsPixels());
}

static void call_proc_Y(SkMorphologyImageFilter::Proc procY,
                        const SkPMColor* src, int srcRowBytesAsPixels, SkBitmap* dst,
                        int radiusY, const SkIRect& bounds) {
    procY(src, dst->getAddr32(0, 0),
          radiusY, bounds.height(), bounds.width(),
          srcRowBytesAsPixels, dst->rowBytesAsPixels());
}

SkRect SkMorphologyImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    bounds.outset(SkIntToScalar(fRadius.width()), SkIntToScalar(fRadius.height()));
    return bounds;
}

SkIRect SkMorphologyImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                                    MapDirection, const SkIRect* inputRect) const {
    SkVector radius = SkVector::Make(SkIntToScalar(this->radius().width()),
                                     SkIntToScalar(this->radius().height()));
    ctm.mapVectors(&radius, 1);
    return src.makeOutset(SkScalarCeilToInt(radius.x()), SkScalarCeilToInt(radius.y()));
}

sk_sp<SkFlattenable> SkErodeImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    const int width = buffer.readInt();
    const int height = buffer.readInt();
    return Make(width, height, common.getInput(0), &common.cropRect());
}

sk_sp<SkFlattenable> SkDilateImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    const int width = buffer.readInt();
    const int height = buffer.readInt();
    return Make(width, height, common.getInput(0), &common.cropRect());
}

#if SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////
/**
 * Morphology effects. Depending upon the type of morphology, either the
 * component-wise min (Erode_Type) or max (Dilate_Type) of all pixels in the
 * kernel is selected as the new color. The new color is modulated by the input
 * color.
 */
class GrMorphologyEffect : public GrFragmentProcessor {
public:
    enum class Direction { kX, kY };
    enum class Type { kErode, kDilate };

    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy, Direction dir,
                                                     int radius, Type type) {
        return std::unique_ptr<GrFragmentProcessor>(
                new GrMorphologyEffect(std::move(proxy), dir, radius, type, nullptr));
    }

    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy, Direction dir,
                                                     int radius, Type type, const float bounds[2]) {
        return std::unique_ptr<GrFragmentProcessor>(
                new GrMorphologyEffect(std::move(proxy), dir, radius, type, bounds));
    }

    Type type() const { return fType; }
    bool useRange() const { return fUseRange; }
    const float* range() const { return fRange; }
    Direction direction() const { return fDirection; }
    int radius() const { return fRadius; }
    int width() const { return 2 * fRadius + 1; }

    const char* name() const override { return "Morphology"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new GrMorphologyEffect(*this));
    }

private:
    GrCoordTransform fCoordTransform;
    TextureSampler fTextureSampler;
    Direction fDirection;
    int fRadius;
    Type fType;
    bool fUseRange;
    float fRange[2];

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    const TextureSampler& onTextureSampler(int i) const override { return fTextureSampler; }

    GrMorphologyEffect(sk_sp<GrTextureProxy>, Direction, int radius, Type, const float range[2]);
    explicit GrMorphologyEffect(const GrMorphologyEffect&);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLMorphologyEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fPixelSizeUni;
    GrGLSLProgramDataManager::UniformHandle fRangeUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLMorphologyEffect::emitCode(EmitArgs& args) {
    const GrMorphologyEffect& me = args.fFp.cast<GrMorphologyEffect>();

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fPixelSizeUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType, "PixelSize");
    const char* pixelSizeInc = uniformHandler->getUniformCStr(fPixelSizeUni);
    fRangeUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kFloat2_GrSLType, "Range");
    const char* range = uniformHandler->getUniformCStr(fRangeUni);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
    const char* func;
    switch (me.type()) {
        case GrMorphologyEffect::Type::kErode:
            fragBuilder->codeAppendf("\t\t%s = half4(1, 1, 1, 1);\n", args.fOutputColor);
            func = "min";
            break;
        case GrMorphologyEffect::Type::kDilate:
            fragBuilder->codeAppendf("\t\t%s = half4(0, 0, 0, 0);\n", args.fOutputColor);
            func = "max";
            break;
        default:
            SK_ABORT("Unexpected type");
            func = ""; // suppress warning
            break;
    }

    const char* dir;
    switch (me.direction()) {
        case GrMorphologyEffect::Direction::kX:
            dir = "x";
            break;
        case GrMorphologyEffect::Direction::kY:
            dir = "y";
            break;
        default:
            SK_ABORT("Unknown filter direction.");
            dir = ""; // suppress warning
    }

    int width = me.width();

    // float2 coord = coord2D;
    fragBuilder->codeAppendf("\t\tfloat2 coord = %s;\n", coords2D.c_str());
    // coord.x -= radius * pixelSize;
    fragBuilder->codeAppendf("\t\tcoord.%s -= %d.0 * %s; \n", dir, me.radius(), pixelSizeInc);
    if (me.useRange()) {
        // highBound = min(highBound, coord.x + (width-1) * pixelSize);
        fragBuilder->codeAppendf("\t\tfloat highBound = min(%s.y, coord.%s + %f * %s);",
                                 range, dir, float(width - 1), pixelSizeInc);
        // coord.x = max(lowBound, coord.x);
        fragBuilder->codeAppendf("\t\tcoord.%s = max(%s.x, coord.%s);", dir, range, dir);
    }
    fragBuilder->codeAppendf("\t\tfor (int i = 0; i < %d; i++) {\n", width);
    fragBuilder->codeAppendf("\t\t\t%s = %s(%s, ", args.fOutputColor, func, args.fOutputColor);
    fragBuilder->appendTextureLookup(args.fTexSamplers[0], "coord");
    fragBuilder->codeAppend(");\n");
    // coord.x += pixelSize;
    fragBuilder->codeAppendf("\t\t\tcoord.%s += %s;\n", dir, pixelSizeInc);
    if (me.useRange()) {
        // coord.x = min(highBound, coord.x);
        fragBuilder->codeAppendf("\t\t\tcoord.%s = min(highBound, coord.%s);", dir, dir);
    }
    fragBuilder->codeAppend("\t\t}\n");
    fragBuilder->codeAppendf("%s *= %s;\n", args.fOutputColor, args.fInputColor);
}

void GrGLMorphologyEffect::GenKey(const GrProcessor& proc,
                                  const GrShaderCaps&, GrProcessorKeyBuilder* b) {
    const GrMorphologyEffect& m = proc.cast<GrMorphologyEffect>();
    uint32_t key = static_cast<uint32_t>(m.radius());
    key |= (static_cast<uint32_t>(m.type()) << 8);
    key |= (static_cast<uint32_t>(m.direction()) << 9);
    if (m.useRange()) {
        key |= 1 << 10;
    }
    b->add32(key);
}

void GrGLMorphologyEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                     const GrFragmentProcessor& proc) {
    const GrMorphologyEffect& m = proc.cast<GrMorphologyEffect>();
    GrSurfaceProxy* proxy = m.textureSampler(0).proxy();
    GrTexture& texture = *proxy->peekTexture();

    float pixelSize = 0.0f;
    switch (m.direction()) {
        case GrMorphologyEffect::Direction::kX:
            pixelSize = 1.0f / texture.width();
            break;
        case GrMorphologyEffect::Direction::kY:
            pixelSize = 1.0f / texture.height();
            break;
        default:
            SK_ABORT("Unknown filter direction.");
    }
    pdman.set1f(fPixelSizeUni, pixelSize);

    if (m.useRange()) {
        const float* range = m.range();
        if (GrMorphologyEffect::Direction::kY == m.direction() &&
            proxy->origin() == kBottomLeft_GrSurfaceOrigin) {
            pdman.set2f(fRangeUni, 1.0f - (range[1]*pixelSize), 1.0f - (range[0]*pixelSize));
        } else {
            pdman.set2f(fRangeUni, range[0] * pixelSize, range[1] * pixelSize);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

GrMorphologyEffect::GrMorphologyEffect(sk_sp<GrTextureProxy> proxy,
                                       Direction direction,
                                       int radius,
                                       Type type,
                                       const float range[2])
        : INHERITED(kGrMorphologyEffect_ClassID,
                    ModulateForClampedSamplerOptFlags(proxy->config()))
        , fCoordTransform(proxy.get())
        , fTextureSampler(std::move(proxy))
        , fDirection(direction)
        , fRadius(radius)
        , fType(type)
        , fUseRange(SkToBool(range)) {
    // Make sure the sampler's ctor uses the clamp wrap mode
    SkASSERT(fTextureSampler.samplerState().wrapModeX() == GrSamplerState::WrapMode::kClamp &&
             fTextureSampler.samplerState().wrapModeY() == GrSamplerState::WrapMode::kClamp);
    this->addCoordTransform(&fCoordTransform);
    this->setTextureSamplerCnt(1);
    if (fUseRange) {
        fRange[0] = range[0];
        fRange[1] = range[1];
    }
}

GrMorphologyEffect::GrMorphologyEffect(const GrMorphologyEffect& that)
        : INHERITED(kGrMorphologyEffect_ClassID, that.optimizationFlags())
        , fCoordTransform(that.fCoordTransform)
        , fTextureSampler(that.fTextureSampler)
        , fDirection(that.fDirection)
        , fRadius(that.fRadius)
        , fType(that.fType)
        , fUseRange(that.fUseRange) {
    this->addCoordTransform(&fCoordTransform);
    this->setTextureSamplerCnt(1);
    if (that.fUseRange) {
        fRange[0] = that.fRange[0];
        fRange[1] = that.fRange[1];
    }
}

void GrMorphologyEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                               GrProcessorKeyBuilder* b) const {
    GrGLMorphologyEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrMorphologyEffect::onCreateGLSLInstance() const {
    return new GrGLMorphologyEffect;
}
bool GrMorphologyEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrMorphologyEffect& s = sBase.cast<GrMorphologyEffect>();
    return (this->radius() == s.radius() &&
            this->direction() == s.direction() &&
            this->useRange() == s.useRange() &&
            this->type() == s.type());
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrMorphologyEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrMorphologyEffect::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                        : GrProcessorUnitTest::kAlphaTextureIdx;
    sk_sp<GrTextureProxy> proxy = d->textureProxy(texIdx);

    Direction dir = d->fRandom->nextBool() ? Direction::kX : Direction::kY;
    static const int kMaxRadius = 10;
    int radius = d->fRandom->nextRangeU(1, kMaxRadius);
    Type type = d->fRandom->nextBool() ? GrMorphologyEffect::Type::kErode
                                       : GrMorphologyEffect::Type::kDilate;

    return GrMorphologyEffect::Make(std::move(proxy), dir, radius, type);
}
#endif

static void apply_morphology_rect(GrRenderTargetContext* renderTargetContext,
                                  const GrClip& clip,
                                  sk_sp<GrTextureProxy> proxy,
                                  const SkIRect& srcRect,
                                  const SkIRect& dstRect,
                                  int radius,
                                  GrMorphologyEffect::Type morphType,
                                  const float bounds[2],
                                  GrMorphologyEffect::Direction direction) {
    GrPaint paint;
    paint.addColorFragmentProcessor(GrMorphologyEffect::Make(std::move(proxy),
                                                             direction, radius, morphType,
                                                             bounds));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    renderTargetContext->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                        SkRect::Make(dstRect), SkRect::Make(srcRect));
}

static void apply_morphology_rect_no_bounds(GrRenderTargetContext* renderTargetContext,
                                            const GrClip& clip,
                                            sk_sp<GrTextureProxy> proxy,
                                            const SkIRect& srcRect,
                                            const SkIRect& dstRect,
                                            int radius,
                                            GrMorphologyEffect::Type morphType,
                                            GrMorphologyEffect::Direction direction) {
    GrPaint paint;
    paint.addColorFragmentProcessor(GrMorphologyEffect::Make(std::move(proxy),
                                                             direction, radius, morphType));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    renderTargetContext->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                        SkRect::Make(dstRect), SkRect::Make(srcRect));
}

static void apply_morphology_pass(GrRenderTargetContext* renderTargetContext,
                                  const GrClip& clip,
                                  sk_sp<GrTextureProxy> textureProxy,
                                  const SkIRect& srcRect,
                                  const SkIRect& dstRect,
                                  int radius,
                                  GrMorphologyEffect::Type morphType,
                                  GrMorphologyEffect::Direction direction) {
    float bounds[2] = { 0.0f, 1.0f };
    SkIRect lowerSrcRect = srcRect, lowerDstRect = dstRect;
    SkIRect middleSrcRect = srcRect, middleDstRect = dstRect;
    SkIRect upperSrcRect = srcRect, upperDstRect = dstRect;
    if (direction == GrMorphologyEffect::Direction::kX) {
        bounds[0] = SkIntToScalar(srcRect.left()) + 0.5f;
        bounds[1] = SkIntToScalar(srcRect.right()) - 0.5f;
        lowerSrcRect.fRight = srcRect.left() + radius;
        lowerDstRect.fRight = dstRect.left() + radius;
        upperSrcRect.fLeft = srcRect.right() - radius;
        upperDstRect.fLeft = dstRect.right() - radius;
        middleSrcRect.inset(radius, 0);
        middleDstRect.inset(radius, 0);
    } else {
        bounds[0] = SkIntToScalar(srcRect.top()) + 0.5f;
        bounds[1] = SkIntToScalar(srcRect.bottom()) - 0.5f;
        lowerSrcRect.fBottom = srcRect.top() + radius;
        lowerDstRect.fBottom = dstRect.top() + radius;
        upperSrcRect.fTop = srcRect.bottom() - radius;
        upperDstRect.fTop = dstRect.bottom() - radius;
        middleSrcRect.inset(0, radius);
        middleDstRect.inset(0, radius);
    }
    if (middleSrcRect.width() <= 0) {
        // radius covers srcRect; use bounds over entire draw
        apply_morphology_rect(renderTargetContext, clip, std::move(textureProxy),
                              srcRect, dstRect, radius, morphType, bounds, direction);
    } else {
        // Draw upper and lower margins with bounds; middle without.
        apply_morphology_rect(renderTargetContext, clip, textureProxy,
                              lowerSrcRect, lowerDstRect, radius, morphType, bounds, direction);
        apply_morphology_rect(renderTargetContext, clip, textureProxy,
                              upperSrcRect, upperDstRect, radius, morphType, bounds, direction);
        apply_morphology_rect_no_bounds(renderTargetContext, clip, std::move(textureProxy),
                                        middleSrcRect, middleDstRect, radius, morphType, direction);
    }
}

static sk_sp<SkSpecialImage> apply_morphology(
                                          GrRecordingContext* context,
                                          SkSpecialImage* input,
                                          const SkIRect& rect,
                                          GrMorphologyEffect::Type morphType,
                                          SkISize radius,
                                          const SkImageFilter::OutputProperties& outputProperties) {
    sk_sp<GrTextureProxy> srcTexture(input->asTextureProxyRef(context));
    SkASSERT(srcTexture);
    sk_sp<SkColorSpace> colorSpace = sk_ref_sp(outputProperties.colorSpace());
    SkColorType colorType = outputProperties.colorType();
    GrBackendFormat format =
            context->priv().caps()->getBackendFormatFromColorType(colorType);
    GrPixelConfig config = SkColorType2GrPixelConfig(colorType);

    // setup new clip
    const GrFixedClip clip(SkIRect::MakeWH(srcTexture->width(), srcTexture->height()));

    const SkIRect dstRect = SkIRect::MakeWH(rect.width(), rect.height());
    SkIRect srcRect = rect;

    SkASSERT(radius.width() > 0 || radius.height() > 0);

    if (radius.fWidth > 0) {
        sk_sp<GrRenderTargetContext> dstRTContext(context->priv().makeDeferredRenderTargetContext(
                format, SkBackingFit::kApprox, rect.width(), rect.height(), config, colorSpace, 1,
                GrMipMapped::kNo, kBottomLeft_GrSurfaceOrigin, nullptr, SkBudgeted::kYes,
                srcTexture->isProtected() ? GrProtected::kYes : GrProtected::kNo));
        if (!dstRTContext) {
            return nullptr;
        }

        apply_morphology_pass(dstRTContext.get(), clip, std::move(srcTexture), srcRect, dstRect,
                              radius.fWidth, morphType, GrMorphologyEffect::Direction::kX);
        SkIRect clearRect = SkIRect::MakeXYWH(dstRect.fLeft, dstRect.fBottom,
                                              dstRect.width(), radius.fHeight);
        SkPMColor4f clearColor = GrMorphologyEffect::Type::kErode == morphType
                ? SK_PMColor4fWHITE : SK_PMColor4fTRANSPARENT;
        dstRTContext->clear(&clearRect, clearColor, GrRenderTargetContext::CanClearFullscreen::kNo);

        srcTexture = dstRTContext->asTextureProxyRef();
        srcRect = dstRect;
    }
    if (radius.fHeight > 0) {
        sk_sp<GrRenderTargetContext> dstRTContext(context->priv().makeDeferredRenderTargetContext(
                format, SkBackingFit::kApprox, rect.width(), rect.height(), config, colorSpace, 1,
                GrMipMapped::kNo, kBottomLeft_GrSurfaceOrigin, nullptr, SkBudgeted::kYes,
                srcTexture->isProtected() ? GrProtected::kYes : GrProtected::kNo));
        if (!dstRTContext) {
            return nullptr;
        }

        apply_morphology_pass(dstRTContext.get(), clip, std::move(srcTexture), srcRect, dstRect,
                              radius.fHeight, morphType, GrMorphologyEffect::Direction::kY);

        srcTexture = dstRTContext->asTextureProxyRef();
    }

    return SkSpecialImage::MakeDeferredFromGpu(context,
                                               SkIRect::MakeWH(rect.width(), rect.height()),
                                               kNeedNewImageUniqueID_SpecialImage,
                                               std::move(srcTexture), std::move(colorSpace),
                                               &input->props());
}
#endif

namespace {
    enum MorphType { kDilate, kErode };
    enum class MorphDirection { kX, kY };

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    template<MorphType type, MorphDirection direction>
    static void morph(const SkPMColor* src, SkPMColor* dst,
                      int radius, int width, int height, int srcStride, int dstStride) {
        const int srcStrideX = direction == MorphDirection::kX ? 1 : srcStride;
        const int dstStrideX = direction == MorphDirection::kX ? 1 : dstStride;
        const int srcStrideY = direction == MorphDirection::kX ? srcStride : 1;
        const int dstStrideY = direction == MorphDirection::kX ? dstStride : 1;
        radius = SkMin32(radius, width - 1);
        const SkPMColor* upperSrc = src + radius * srcStrideX;
        for (int x = 0; x < width; ++x) {
            const SkPMColor* lp = src;
            const SkPMColor* up = upperSrc;
            SkPMColor* dptr = dst;
            for (int y = 0; y < height; ++y) {
                __m128i extreme = (type == kDilate) ? _mm_setzero_si128()
                                                    : _mm_set1_epi32(0xFFFFFFFF);
                for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                    __m128i src_pixel = _mm_cvtsi32_si128(*p);
                    extreme = (type == kDilate) ? _mm_max_epu8(src_pixel, extreme)
                                                : _mm_min_epu8(src_pixel, extreme);
                }
                *dptr = _mm_cvtsi128_si32(extreme);
                dptr += dstStrideY;
                lp += srcStrideY;
                up += srcStrideY;
            }
            if (x >= radius) { src += srcStrideX; }
            if (x + radius < width - 1) { upperSrc += srcStrideX; }
            dst += dstStrideX;
        }
    }

#elif defined(SK_ARM_HAS_NEON)
    template<MorphType type, MorphDirection direction>
    static void morph(const SkPMColor* src, SkPMColor* dst,
                      int radius, int width, int height, int srcStride, int dstStride) {
        const int srcStrideX = direction == MorphDirection::kX ? 1 : srcStride;
        const int dstStrideX = direction == MorphDirection::kX ? 1 : dstStride;
        const int srcStrideY = direction == MorphDirection::kX ? srcStride : 1;
        const int dstStrideY = direction == MorphDirection::kX ? dstStride : 1;
        radius = SkMin32(radius, width - 1);
        const SkPMColor* upperSrc = src + radius * srcStrideX;
        for (int x = 0; x < width; ++x) {
            const SkPMColor* lp = src;
            const SkPMColor* up = upperSrc;
            SkPMColor* dptr = dst;
            for (int y = 0; y < height; ++y) {
                uint8x8_t extreme = vdup_n_u8(type == kDilate ? 0 : 255);
                for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                    uint8x8_t src_pixel = vreinterpret_u8_u32(vdup_n_u32(*p));
                    extreme = (type == kDilate) ? vmax_u8(src_pixel, extreme)
                                                : vmin_u8(src_pixel, extreme);
                }
                *dptr = vget_lane_u32(vreinterpret_u32_u8(extreme), 0);
                dptr += dstStrideY;
                lp += srcStrideY;
                up += srcStrideY;
            }
            if (x >= radius) src += srcStrideX;
            if (x + radius < width - 1) upperSrc += srcStrideX;
            dst += dstStrideX;
        }
    }

#else
    template<MorphType type, MorphDirection direction>
    static void morph(const SkPMColor* src, SkPMColor* dst,
                      int radius, int width, int height, int srcStride, int dstStride) {
        const int srcStrideX = direction == MorphDirection::kX ? 1 : srcStride;
        const int dstStrideX = direction == MorphDirection::kX ? 1 : dstStride;
        const int srcStrideY = direction == MorphDirection::kX ? srcStride : 1;
        const int dstStrideY = direction == MorphDirection::kX ? dstStride : 1;
        radius = SkMin32(radius, width - 1);
        const SkPMColor* upperSrc = src + radius * srcStrideX;
        for (int x = 0; x < width; ++x) {
            const SkPMColor* lp = src;
            const SkPMColor* up = upperSrc;
            SkPMColor* dptr = dst;
            for (int y = 0; y < height; ++y) {
                // If we're maxing (dilate), start from 0; if minning (erode), start from 255.
                const int start = (type == kDilate) ? 0 : 255;
                int B = start, G = start, R = start, A = start;
                for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                    int b = SkGetPackedB32(*p),
                        g = SkGetPackedG32(*p),
                        r = SkGetPackedR32(*p),
                        a = SkGetPackedA32(*p);
                    if (type == kDilate) {
                        B = SkTMax(b, B);
                        G = SkTMax(g, G);
                        R = SkTMax(r, R);
                        A = SkTMax(a, A);
                    } else {
                        B = SkTMin(b, B);
                        G = SkTMin(g, G);
                        R = SkTMin(r, R);
                        A = SkTMin(a, A);
                    }
                }
                *dptr = SkPackARGB32(A, R, G, B);
                dptr += dstStrideY;
                lp += srcStrideY;
                up += srcStrideY;
            }
            if (x >= radius) { src += srcStrideX; }
            if (x + radius < width - 1) { upperSrc += srcStrideX; }
            dst += dstStrideX;
        }
    }
#endif
}  // namespace

sk_sp<SkSpecialImage> SkMorphologyImageFilter::onFilterImage(SkSpecialImage* source,
                                                             const Context& ctx,
                                                             SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    SkIRect bounds;
    input = this->applyCropRectAndPad(this->mapContext(ctx), input.get(), &inputOffset, &bounds);
    if (!input) {
        return nullptr;
    }

    SkVector radius = SkVector::Make(SkIntToScalar(this->radius().width()),
                                     SkIntToScalar(this->radius().height()));
    ctx.ctm().mapVectors(&radius, 1);
    int width = SkScalarFloorToInt(radius.fX);
    int height = SkScalarFloorToInt(radius.fY);

    if (width < 0 || height < 0) {
        return nullptr;
    }

    SkIRect srcBounds = bounds;
    srcBounds.offset(-inputOffset);

    if (0 == width && 0 == height) {
        offset->fX = bounds.left();
        offset->fY = bounds.top();
        return input->makeSubset(srcBounds);
    }

#if SK_SUPPORT_GPU
    if (source->isTextureBacked()) {
        auto context = source->getContext();

        // Ensure the input is in the destination color space. Typically applyCropRect will have
        // called pad_image to account for our dilation of bounds, so the result will already be
        // moved to the destination color space. If a filter DAG avoids that, then we use this
        // fall-back, which saves us from having to do the xform during the filter itself.
        input = ImageToColorSpace(input.get(), ctx.outputProperties());

        auto type = (kDilate_Op == this->op()) ? GrMorphologyEffect::Type::kDilate
                                               : GrMorphologyEffect::Type::kErode;
        sk_sp<SkSpecialImage> result(apply_morphology(context, input.get(), srcBounds, type,
                                                      SkISize::Make(width, height),
                                                      ctx.outputProperties()));
        if (result) {
            offset->fX = bounds.left();
            offset->fY = bounds.top();
        }
        return result;
    }
#endif

    SkBitmap inputBM;

    if (!input->getROPixels(&inputBM)) {
        return nullptr;
    }

    if (inputBM.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::Make(bounds.width(), bounds.height(),
                                         inputBM.colorType(), inputBM.alphaType());

    SkBitmap dst;
    if (!dst.tryAllocPixels(info)) {
        return nullptr;
    }

    SkMorphologyImageFilter::Proc procX, procY;

    if (kDilate_Op == this->op()) {
        procX = &morph<kDilate, MorphDirection::kX>;
        procY = &morph<kDilate, MorphDirection::kY>;
    } else {
        procX = &morph<kErode,  MorphDirection::kX>;
        procY = &morph<kErode,  MorphDirection::kY>;
    }

    if (width > 0 && height > 0) {
        SkBitmap tmp;
        if (!tmp.tryAllocPixels(info)) {
            return nullptr;
        }

        call_proc_X(procX, inputBM, &tmp, width, srcBounds);
        SkIRect tmpBounds = SkIRect::MakeWH(srcBounds.width(), srcBounds.height());
        call_proc_Y(procY,
                    tmp.getAddr32(tmpBounds.left(), tmpBounds.top()), tmp.rowBytesAsPixels(),
                    &dst, height, tmpBounds);
    } else if (width > 0) {
        call_proc_X(procX, inputBM, &dst, width, srcBounds);
    } else if (height > 0) {
        call_proc_Y(procY,
                    inputBM.getAddr32(srcBounds.left(), srcBounds.top()),
                    inputBM.rowBytesAsPixels(),
                    &dst, height, srcBounds);
    }
    offset->fX = bounds.left();
    offset->fY = bounds.top();

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(bounds.width(), bounds.height()),
                                          dst, &source->props());
}
