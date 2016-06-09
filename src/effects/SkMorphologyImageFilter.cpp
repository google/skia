/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMorphologyImageFilter.h"

#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkOpts.h"
#include "SkReadBuffer.h"
#include "SkRect.h"
#include "SkSpecialImage.h"
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
                                                    MapDirection) const {
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

    static sk_sp<GrFragmentProcessor> Make(GrTexture* tex, Direction dir, int radius,
                                           MorphologyType type) {
        return sk_sp<GrFragmentProcessor>(new GrMorphologyEffect(tex, dir, radius, type));
    }

    static sk_sp<GrFragmentProcessor> Make(GrTexture* tex, Direction dir, int radius,
                                           MorphologyType type, float bounds[2]) {
        return sk_sp<GrFragmentProcessor>(new GrMorphologyEffect(tex, dir, radius, type, bounds));
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
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fPixelSizeUni;
    GrGLSLProgramDataManager::UniformHandle fRangeUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLMorphologyEffect::emitCode(EmitArgs& args) {
    const GrMorphologyEffect& me = args.fFp.cast<GrMorphologyEffect>();

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fPixelSizeUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                               kFloat_GrSLType, kDefault_GrSLPrecision,
                                               "PixelSize");
    const char* pixelSizeInc = uniformHandler->getUniformCStr(fPixelSizeUni);
    fRangeUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                           kVec2f_GrSLType, kDefault_GrSLPrecision,
                                           "Range");
    const char* range = uniformHandler->getUniformCStr(fRangeUni);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureFSCoords2D(args.fCoords, 0);
    const char* func;
    switch (me.type()) {
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
    switch (me.direction()) {
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

    int width = GrMorphologyEffect::WidthFromRadius(me.radius());

    // vec2 coord = coord2D;
    fragBuilder->codeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
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
    if (m.useRange()) {
        key |= 1 << 10;
    }
    b->add32(key);
}

void GrGLMorphologyEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                     const GrProcessor& proc) {
    const GrMorphologyEffect& m = proc.cast<GrMorphologyEffect>();
    GrTexture& texture = *m.texture(0);

    float pixelSize = 0.0f;
    switch (m.direction()) {
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

    if (m.useRange()) {
        const float* range = m.range();
        if (m.direction() && texture.origin() == kBottomLeft_GrSurfaceOrigin) {
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
    , fType(type)
    , fUseRange(false) {
    this->initClassID<GrMorphologyEffect>();
}

GrMorphologyEffect::GrMorphologyEffect(GrTexture* texture,
                                       Direction direction,
                                       int radius,
                                       MorphologyType type,
                                       float range[2])
    : INHERITED(texture, direction, radius)
    , fType(type)
    , fUseRange(true) {
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
    return new GrGLMorphologyEffect;
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

sk_sp<GrFragmentProcessor> GrMorphologyEffect::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                          GrProcessorUnitTest::kAlphaTextureIdx;
    Direction dir = d->fRandom->nextBool() ? kX_Direction : kY_Direction;
    static const int kMaxRadius = 10;
    int radius = d->fRandom->nextRangeU(1, kMaxRadius);
    MorphologyType type = d->fRandom->nextBool() ? GrMorphologyEffect::kErode_MorphologyType :
                                               GrMorphologyEffect::kDilate_MorphologyType;

    return GrMorphologyEffect::Make(d->fTextures[texIdx], dir, radius, type);
}


static void apply_morphology_rect(GrDrawContext* drawContext,
                                  const GrClip& clip,
                                  GrTexture* texture,
                                  const SkIRect& srcRect,
                                  const SkIRect& dstRect,
                                  int radius,
                                  GrMorphologyEffect::MorphologyType morphType,
                                  float bounds[2],
                                  Gr1DKernelEffect::Direction direction) {
    GrPaint paint;
    // SRGBTODO: AllowSRGBInputs?
    paint.addColorFragmentProcessor(GrMorphologyEffect::Make(texture,
                                                             direction,
                                                             radius,
                                                             morphType,
                                                             bounds));
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    drawContext->fillRectToRect(clip, paint, SkMatrix::I(), SkRect::Make(dstRect),
                                     SkRect::Make(srcRect));
}

static void apply_morphology_rect_no_bounds(GrDrawContext* drawContext,
                                            const GrClip& clip,
                                            GrTexture* texture,
                                            const SkIRect& srcRect,
                                            const SkIRect& dstRect,
                                            int radius,
                                            GrMorphologyEffect::MorphologyType morphType,
                                            Gr1DKernelEffect::Direction direction) {
    GrPaint paint;
    // SRGBTODO: AllowSRGBInputs?
    paint.addColorFragmentProcessor(GrMorphologyEffect::Make(texture, direction, radius,
                                                             morphType));
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    drawContext->fillRectToRect(clip, paint, SkMatrix::I(), SkRect::Make(dstRect),
                                SkRect::Make(srcRect));
}

static void apply_morphology_pass(GrDrawContext* drawContext,
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

static sk_sp<SkSpecialImage> apply_morphology(GrContext* context,
                                              SkSpecialImage* input,
                                              const SkIRect& rect,
                                              GrMorphologyEffect::MorphologyType morphType,
                                              SkISize radius) {
    sk_sp<GrTexture> srcTexture(input->asTextureRef(context));
    SkASSERT(srcTexture);

    // setup new clip
    const GrFixedClip clip(SkIRect::MakeWH(srcTexture->width(), srcTexture->height()));

    const SkIRect dstRect = SkIRect::MakeWH(rect.width(), rect.height());
    SkIRect srcRect = rect;

    SkASSERT(radius.width() > 0 || radius.height() > 0);

    if (radius.fWidth > 0) {
        sk_sp<GrDrawContext> dstDrawContext(context->newDrawContext(SkBackingFit::kApprox,
                                                                    rect.width(), rect.height(),
                                                                    kSkia8888_GrPixelConfig));
        if (!dstDrawContext) {
            return nullptr;
        }

        apply_morphology_pass(dstDrawContext.get(), clip, srcTexture.get(),
                              srcRect, dstRect, radius.fWidth, morphType,
                              Gr1DKernelEffect::kX_Direction);
        SkIRect clearRect = SkIRect::MakeXYWH(dstRect.fLeft, dstRect.fBottom,
                                              dstRect.width(), radius.fHeight);
        GrColor clearColor = GrMorphologyEffect::kErode_MorphologyType == morphType
                                ? SK_ColorWHITE
                                : SK_ColorTRANSPARENT;
        dstDrawContext->clear(&clearRect, clearColor, false);

        srcTexture = dstDrawContext->asTexture();
        srcRect = dstRect;
    }
    if (radius.fHeight > 0) {
        sk_sp<GrDrawContext> dstDrawContext(context->newDrawContext(SkBackingFit::kApprox,
                                                                    rect.width(), rect.height(),
                                                                    kSkia8888_GrPixelConfig));
        if (!dstDrawContext) {
            return nullptr;
        }

        apply_morphology_pass(dstDrawContext.get(), clip, srcTexture.get(),
                              srcRect, dstRect, radius.fHeight, morphType,
                              Gr1DKernelEffect::kY_Direction);

        srcTexture = dstDrawContext->asTexture();
    }

    return SkSpecialImage::MakeFromGpu(SkIRect::MakeWH(rect.width(), rect.height()),
                                       kNeedNewImageUniqueID_SpecialImage,
                                       std::move(srcTexture), &input->props());
}
#endif

sk_sp<SkSpecialImage> SkMorphologyImageFilter::onFilterImage(SkSpecialImage* source,
                                                             const Context& ctx,
                                                             SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    SkIRect bounds;
    input = this->applyCropRect(this->mapContext(ctx), input.get(), &inputOffset, &bounds);
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
        GrContext* context = source->getContext();

        auto type = (kDilate_Op == this->op()) ? GrMorphologyEffect::kDilate_MorphologyType
                                               : GrMorphologyEffect::kErode_MorphologyType;
        sk_sp<SkSpecialImage> result(apply_morphology(context, input.get(), srcBounds, type,
                                                      SkISize::Make(width, height)));
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

    SkAutoLockPixels inputLock(inputBM), dstLock(dst);

    SkMorphologyImageFilter::Proc procX, procY;

    if (kDilate_Op == this->op()) {
        procX = SkOpts::dilate_x;
        procY = SkOpts::dilate_y;
    } else {
        procX = SkOpts::erode_x;
        procY = SkOpts::erode_y;
    }

    if (width > 0 && height > 0) {
        SkBitmap tmp;
        if (!tmp.tryAllocPixels(info)) {
            return nullptr;
        }

        SkAutoLockPixels tmpLock(tmp);

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
