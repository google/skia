/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMorphologyImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkOpts.h"
#include "SkReadBuffer.h"
#include "SkRect.h"
#include "SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrInvariantOutput.h"
#include "GrTexture.h"
#include "SkGr.h"
#include "effects/Gr1DKernelEffect.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#endif

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
    if (!this->filterInput(0, proxy, source, ctx, &src, &srcOffset)) {
        return false;
    }

    if (src.colorType() != kN32_SkColorType) {
        return false;
    }

    SkIRect bounds;
    if (!this->applyCropRect(this->mapContext(ctx), proxy, src, &srcOffset, &bounds, &src)) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
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

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (!device) {
        return false;
    }
    *dst = device->accessBitmap(false);
    SkAutoLockPixels alp_dst(*dst);

    if (width > 0 && height > 0) {
        SkAutoTUnref<SkBaseDevice> tempDevice(proxy->createDevice(dst->width(), dst->height()));
        if (!tempDevice) {
            return false;
        }
        SkBitmap temp = tempDevice->accessBitmap(false);
        SkAutoLockPixels alp_temp(temp);
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
    return this->filterImageGeneric(SkOpts::erode_x, SkOpts::erode_y,
                                    proxy, source, ctx, dst, offset);
}

bool SkDilateImageFilter::onFilterImage(Proxy* proxy,
                                        const SkBitmap& source, const Context& ctx,
                                        SkBitmap* dst, SkIPoint* offset) const {
    return this->filterImageGeneric(SkOpts::dilate_x, SkOpts::dilate_y,
                                    proxy, source, ctx, dst, offset);
}

void SkMorphologyImageFilter::computeFastBounds(const SkRect& src, SkRect* dst) const {
    if (this->getInput(0)) {
        this->getInput(0)->computeFastBounds(src, dst);
    } else {
        *dst = src;
    }
    dst->outset(SkIntToScalar(fRadius.width()), SkIntToScalar(fRadius.height()));
}

void SkMorphologyImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                                 SkIRect* dst, MapDirection) const {
    *dst = src;
    SkVector radius = SkVector::Make(SkIntToScalar(this->radius().width()),
                                     SkIntToScalar(this->radius().height()));
    ctm.mapVectors(&radius, 1);
    dst->outset(SkScalarCeilToInt(radius.x()), SkScalarCeilToInt(radius.y()));
}

SkFlattenable* SkErodeImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    const int width = buffer.readInt();
    const int height = buffer.readInt();
    return Create(width, height, common.getInput(0), &common.cropRect());
}

SkFlattenable* SkDilateImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    const int width = buffer.readInt();
    const int height = buffer.readInt();
    return Create(width, height, common.getInput(0), &common.cropRect());
}

#ifndef SK_IGNORE_TO_STRING
void SkErodeImageFilter::toString(SkString* str) const {
    str->appendf("SkErodeImageFilter: (");
    str->appendf("radius: (%d,%d)", this->radius().fWidth, this->radius().fHeight);
    str->append(")");
}
#endif

#ifndef SK_IGNORE_TO_STRING
void SkDilateImageFilter::toString(SkString* str) const {
    str->appendf("SkDilateImageFilter: (");
    str->appendf("radius: (%d,%d)", this->radius().fWidth, this->radius().fHeight);
    str->append(")");
}
#endif

#if SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////
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

    static GrFragmentProcessor* Create(GrTexture* tex, Direction dir, int radius,
                                       MorphologyType type) {
        return new GrMorphologyEffect(tex, dir, radius, type);
    }

    static GrFragmentProcessor* Create(GrTexture* tex, Direction dir, int radius,
                                       MorphologyType type, float bounds[2]) {
        return new GrMorphologyEffect(tex, dir, radius, type, bounds);
    }

    virtual ~GrMorphologyEffect();

    MorphologyType type() const { return fType; }
    bool useRange() const { return fUseRange; }
    const float* range() const { return fRange; }

    const char* name() const override { return "Morphology"; }

protected:

    MorphologyType fType;
    bool fUseRange;
    float fRange[2];

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GrMorphologyEffect(GrTexture*, Direction, int radius, MorphologyType);
    GrMorphologyEffect(GrTexture*, Direction, int radius, MorphologyType,
                       float bounds[2]);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef Gr1DKernelEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLMorphologyEffect : public GrGLSLFragmentProcessor {
public:
    GrGLMorphologyEffect(const GrProcessor&);

    virtual void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder* b);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    int width() const { return GrMorphologyEffect::WidthFromRadius(fRadius); }

    int                                   fRadius;
    Gr1DKernelEffect::Direction           fDirection;
    bool                                  fUseRange;
    GrMorphologyEffect::MorphologyType    fType;
    GrGLSLProgramDataManager::UniformHandle fPixelSizeUni;
    GrGLSLProgramDataManager::UniformHandle fRangeUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

GrGLMorphologyEffect::GrGLMorphologyEffect(const GrProcessor& proc) {
    const GrMorphologyEffect& m = proc.cast<GrMorphologyEffect>();
    fRadius = m.radius();
    fDirection = m.direction();
    fUseRange = m.useRange();
    fType = m.type();
}

void GrGLMorphologyEffect::emitCode(EmitArgs& args) {
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fPixelSizeUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                               kFloat_GrSLType, kDefault_GrSLPrecision,
                                               "PixelSize");
    const char* pixelSizeInc = uniformHandler->getUniformCStr(fPixelSizeUni);
    fRangeUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                           kVec2f_GrSLType, kDefault_GrSLPrecision,
                                           "Range");
    const char* range = uniformHandler->getUniformCStr(fRangeUni);

    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureFSCoords2D(args.fCoords, 0);
    const char* func;
    switch (fType) {
        case GrMorphologyEffect::kErode_MorphologyType:
            fragBuilder->codeAppendf("\t\t%s = vec4(1, 1, 1, 1);\n", args.fOutputColor);
            func = "min";
            break;
        case GrMorphologyEffect::kDilate_MorphologyType:
            fragBuilder->codeAppendf("\t\t%s = vec4(0, 0, 0, 0);\n", args.fOutputColor);
            func = "max";
            break;
        default:
            SkFAIL("Unexpected type");
            func = ""; // suppress warning
            break;
    }

    const char* dir;
    switch (fDirection) {
        case Gr1DKernelEffect::kX_Direction:
            dir = "x";
            break;
        case Gr1DKernelEffect::kY_Direction:
            dir = "y";
            break;
        default:
            SkFAIL("Unknown filter direction.");
            dir = ""; // suppress warning
    }

    // vec2 coord = coord2D;
    fragBuilder->codeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
    // coord.x -= radius * pixelSize;
    fragBuilder->codeAppendf("\t\tcoord.%s -= %d.0 * %s; \n", dir, fRadius, pixelSizeInc);
    if (fUseRange) {
        // highBound = min(highBound, coord.x + (width-1) * pixelSize);
        fragBuilder->codeAppendf("\t\tfloat highBound = min(%s.y, coord.%s + %f * %s);",
                                 range, dir, float(width() - 1), pixelSizeInc);
        // coord.x = max(lowBound, coord.x);
        fragBuilder->codeAppendf("\t\tcoord.%s = max(%s.x, coord.%s);", dir, range, dir);
    }
    fragBuilder->codeAppendf("\t\tfor (int i = 0; i < %d; i++) {\n", width());
    fragBuilder->codeAppendf("\t\t\t%s = %s(%s, ", args.fOutputColor, func, args.fOutputColor);
    fragBuilder->appendTextureLookup(args.fSamplers[0], "coord");
    fragBuilder->codeAppend(");\n");
    // coord.x += pixelSize;
    fragBuilder->codeAppendf("\t\t\tcoord.%s += %s;\n", dir, pixelSizeInc);
    if (fUseRange) {
        // coord.x = min(highBound, coord.x);
        fragBuilder->codeAppendf("\t\t\tcoord.%s = min(highBound, coord.%s);", dir, dir);
    }
    fragBuilder->codeAppend("\t\t}\n");
    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, args.fOutputColor, args.fInputColor);
    fragBuilder->codeAppend(modulate.c_str());
}

void GrGLMorphologyEffect::GenKey(const GrProcessor& proc,
                                  const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
    const GrMorphologyEffect& m = proc.cast<GrMorphologyEffect>();
    uint32_t key = static_cast<uint32_t>(m.radius());
    key |= (m.type() << 8);
    key |= (m.direction() << 9);
    if (m.useRange()) key |= 1 << 10;
    b->add32(key);
}

void GrGLMorphologyEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                   const GrProcessor& proc) {
    const GrMorphologyEffect& m = proc.cast<GrMorphologyEffect>();
    GrTexture& texture = *m.texture(0);
    // the code we generated was for a specific kernel radius, direction and bound usage
    SkASSERT(m.radius() == fRadius);
    SkASSERT(m.direction() == fDirection);
    SkASSERT(m.useRange() == fUseRange);

    float pixelSize = 0.0f;
    switch (fDirection) {
        case Gr1DKernelEffect::kX_Direction:
            pixelSize = 1.0f / texture.width();
            break;
        case Gr1DKernelEffect::kY_Direction:
            pixelSize = 1.0f / texture.height();
            break;
        default:
            SkFAIL("Unknown filter direction.");
    }
    pdman.set1f(fPixelSizeUni, pixelSize);

    if (fUseRange) {
        const float* range = m.range();
        if (fDirection && texture.origin() == kBottomLeft_GrSurfaceOrigin) {
            pdman.set2f(fRangeUni, 1.0f - range[1], 1.0f - range[0]);
        } else {
            pdman.set2f(fRangeUni, range[0], range[1]);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

GrMorphologyEffect::GrMorphologyEffect(GrTexture* texture,
                                       Direction direction,
                                       int radius,
                                       MorphologyType type)
    : INHERITED(texture, direction, radius)
    , fType(type), fUseRange(false) {
    this->initClassID<GrMorphologyEffect>();
}

GrMorphologyEffect::GrMorphologyEffect(GrTexture* texture,
                                       Direction direction,
                                       int radius,
                                       MorphologyType type,
                                       float range[2])
    : INHERITED(texture, direction, radius)
    , fType(type), fUseRange(true) {
    this->initClassID<GrMorphologyEffect>();
    fRange[0] = range[0];
    fRange[1] = range[1];
}

GrMorphologyEffect::~GrMorphologyEffect() {
}

void GrMorphologyEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                               GrProcessorKeyBuilder* b) const {
    GrGLMorphologyEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrMorphologyEffect::onCreateGLSLInstance() const {
    return new GrGLMorphologyEffect(*this);
}
bool GrMorphologyEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrMorphologyEffect& s = sBase.cast<GrMorphologyEffect>();
    return (this->radius() == s.radius() &&
            this->direction() == s.direction() &&
            this->useRange() == s.useRange() &&
            this->type() == s.type());
}

void GrMorphologyEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    // This is valid because the color components of the result of the kernel all come
    // exactly from existing values in the source texture.
    this->updateInvariantOutputForModulation(inout);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrMorphologyEffect);

const GrFragmentProcessor* GrMorphologyEffect::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                      GrProcessorUnitTest::kAlphaTextureIdx;
    Direction dir = d->fRandom->nextBool() ? kX_Direction : kY_Direction;
    static const int kMaxRadius = 10;
    int radius = d->fRandom->nextRangeU(1, kMaxRadius);
    MorphologyType type = d->fRandom->nextBool() ? GrMorphologyEffect::kErode_MorphologyType :
                                               GrMorphologyEffect::kDilate_MorphologyType;

    return GrMorphologyEffect::Create(d->fTextures[texIdx], dir, radius, type);
}

namespace {


void apply_morphology_rect(GrDrawContext* drawContext,
                           const GrClip& clip,
                           GrTexture* texture,
                           const SkIRect& srcRect,
                           const SkIRect& dstRect,
                           int radius,
                           GrMorphologyEffect::MorphologyType morphType,
                           float bounds[2],
                           Gr1DKernelEffect::Direction direction) {
    GrPaint paint;
    paint.addColorFragmentProcessor(GrMorphologyEffect::Create(texture,
                                                               direction,
                                                               radius,
                                                               morphType,
                                                               bounds))->unref();
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    drawContext->fillRectToRect(clip, paint, SkMatrix::I(), SkRect::Make(dstRect),
                                     SkRect::Make(srcRect));
}

void apply_morphology_rect_no_bounds(GrDrawContext* drawContext,
                                     const GrClip& clip,
                                     GrTexture* texture,
                                     const SkIRect& srcRect,
                                     const SkIRect& dstRect,
                                     int radius,
                                     GrMorphologyEffect::MorphologyType morphType,
                                     Gr1DKernelEffect::Direction direction) {
    GrPaint paint;
    paint.addColorFragmentProcessor(GrMorphologyEffect::Create(texture,
                                                               direction,
                                                               radius,
                                                               morphType))->unref();
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    drawContext->fillRectToRect(clip, paint, SkMatrix::I(), SkRect::Make(dstRect),
                                SkRect::Make(srcRect));
}

void apply_morphology_pass(GrDrawContext* drawContext,
                           const GrClip& clip,
                           GrTexture* texture,
                           const SkIRect& srcRect,
                           const SkIRect& dstRect,
                           int radius,
                           GrMorphologyEffect::MorphologyType morphType,
                           Gr1DKernelEffect::Direction direction) {
    float bounds[2] = { 0.0f, 1.0f };
    SkIRect lowerSrcRect = srcRect, lowerDstRect = dstRect;
    SkIRect middleSrcRect = srcRect, middleDstRect = dstRect;
    SkIRect upperSrcRect = srcRect, upperDstRect = dstRect;
    if (direction == Gr1DKernelEffect::kX_Direction) {
        bounds[0] = (SkIntToScalar(srcRect.left()) + 0.5f) / texture->width();
        bounds[1] = (SkIntToScalar(srcRect.right()) - 0.5f) / texture->width();
        lowerSrcRect.fRight = srcRect.left() + radius;
        lowerDstRect.fRight = dstRect.left() + radius;
        upperSrcRect.fLeft = srcRect.right() - radius;
        upperDstRect.fLeft = dstRect.right() - radius;
        middleSrcRect.inset(radius, 0);
        middleDstRect.inset(radius, 0);
    } else {
        bounds[0] = (SkIntToScalar(srcRect.top()) + 0.5f) / texture->height();
        bounds[1] = (SkIntToScalar(srcRect.bottom()) - 0.5f) / texture->height();
        lowerSrcRect.fBottom = srcRect.top() + radius;
        lowerDstRect.fBottom = dstRect.top() + radius;
        upperSrcRect.fTop = srcRect.bottom() - radius;
        upperDstRect.fTop = dstRect.bottom() - radius;
        middleSrcRect.inset(0, radius);
        middleDstRect.inset(0, radius);
    }
    if (middleSrcRect.fLeft - middleSrcRect.fRight >= 0) {
        // radius covers srcRect; use bounds over entire draw
        apply_morphology_rect(drawContext, clip, texture, srcRect, dstRect, radius,
                              morphType, bounds, direction);
    } else {
        // Draw upper and lower margins with bounds; middle without.
        apply_morphology_rect(drawContext, clip, texture, lowerSrcRect, lowerDstRect, radius,
                              morphType, bounds, direction);
        apply_morphology_rect(drawContext, clip, texture, upperSrcRect, upperDstRect, radius,
                              morphType, bounds, direction);
        apply_morphology_rect_no_bounds(drawContext, clip, texture, middleSrcRect, middleDstRect,
                                        radius, morphType, direction);
    }
}

bool apply_morphology(const SkBitmap& input,
                      const SkIRect& rect,
                      GrMorphologyEffect::MorphologyType morphType,
                      SkISize radius,
                      SkBitmap* dst) {
    SkAutoTUnref<GrTexture> srcTexture(SkRef(input.getTexture()));
    SkASSERT(srcTexture);
    GrContext* context = srcTexture->getContext();

    // setup new clip
    GrClip clip(SkRect::MakeWH(SkIntToScalar(srcTexture->width()),
                               SkIntToScalar(srcTexture->height())));

    SkIRect dstRect = SkIRect::MakeWH(rect.width(), rect.height());
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = rect.width();
    desc.fHeight = rect.height();
    desc.fConfig = kSkia8888_GrPixelConfig;
    SkIRect srcRect = rect;

    if (radius.fWidth > 0) {
        GrTexture* scratch = context->textureProvider()->createApproxTexture(desc);
        if (nullptr == scratch) {
            return false;
        }
        SkAutoTUnref<GrDrawContext> dstDrawContext(
                                                context->drawContext(scratch->asRenderTarget()));
        if (!dstDrawContext) {
            return false;
        }

        apply_morphology_pass(dstDrawContext, clip, srcTexture,
                              srcRect, dstRect, radius.fWidth, morphType,
                              Gr1DKernelEffect::kX_Direction);
        SkIRect clearRect = SkIRect::MakeXYWH(dstRect.fLeft, dstRect.fBottom,
                                              dstRect.width(), radius.fHeight);
        GrColor clearColor = GrMorphologyEffect::kErode_MorphologyType == morphType ?
                                SK_ColorWHITE :
                                SK_ColorTRANSPARENT;
        dstDrawContext->clear(&clearRect, clearColor, false);

        srcTexture.reset(scratch);
        srcRect = dstRect;
    }
    if (radius.fHeight > 0) {
        GrTexture* scratch = context->textureProvider()->createApproxTexture(desc);
        if (nullptr == scratch) {
            return false;
        }
        SkAutoTUnref<GrDrawContext> dstDrawContext(
                                                context->drawContext(scratch->asRenderTarget()));
        if (!dstDrawContext) {
            return false;
        }

        apply_morphology_pass(dstDrawContext, clip, srcTexture,
                              srcRect, dstRect, radius.fHeight, morphType,
                              Gr1DKernelEffect::kY_Direction);

        srcTexture.reset(scratch);
    }
    GrWrapTextureInBitmap(srcTexture, rect.width(), rect.height(), false, dst);
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
    if (!this->filterInputGPU(0, proxy, src, ctx, &input, &srcOffset)) {
        return false;
    }
    SkIRect bounds;
    if (!this->applyCropRect(this->mapContext(ctx), proxy, input, &srcOffset, &bounds, &input)) {
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

    GrMorphologyEffect::MorphologyType type = dilate ? GrMorphologyEffect::kDilate_MorphologyType
                                                     : GrMorphologyEffect::kErode_MorphologyType;
    if (!apply_morphology(input, srcBounds, type, SkISize::Make(width, height), result)) {
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
