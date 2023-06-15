/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#if defined(SK_USE_LEGACY_MORPHOLOGY_IMAGEFILTER)

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

#if defined(SK_GANESH)
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

struct GrShaderCaps;
#endif

#if GR_TEST_UTILS
#include "src/base/SkRandom.h"
#endif

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include <emmintrin.h>
#endif

#if defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>
#endif

namespace {

enum class MorphType {
    kErode,
    kDilate,
    kLastType = kDilate
};

enum class MorphDirection { kX, kY };

class SkMorphologyImageFilter final : public SkImageFilter_Base {
public:
    SkMorphologyImageFilter(MorphType type, SkScalar radiusX, SkScalar radiusY,
                            sk_sp<SkImageFilter> input, const SkRect* cropRect)
            : INHERITED(&input, 1, cropRect)
            , fType(type)
            , fRadius(SkSize::Make(radiusX, radiusY)) {}

    SkRect computeFastBounds(const SkRect& src) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

    /**
     * All morphology procs have the same signature: src is the source buffer, dst the
     * destination buffer, radius is the morphology radius, width and height are the bounds
     * of the destination buffer (in pixels), and srcStride and dstStride are the
     * number of pixels per row in each buffer. All buffers are 8888.
     */

    typedef void (*Proc)(const SkPMColor* src, SkPMColor* dst, int radius,
                         int width, int height, int srcStride, int dstStride);

protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;
    void flatten(SkWriteBuffer&) const override;

    SkSize mappedRadius(const SkMatrix& ctm) const {
      SkVector radiusVector = SkVector::Make(fRadius.width(), fRadius.height());
      ctm.mapVectors(&radiusVector, 1);
      radiusVector.setAbs(radiusVector);
      return SkSize::Make(radiusVector.x(), radiusVector.y());
    }

private:
    friend void ::SkRegisterMorphologyImageFilterFlattenables();

    SK_FLATTENABLE_HOOKS(SkMorphologyImageFilter)

    MorphType fType;
    SkSize    fRadius;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Dilate(SkScalar radiusX, SkScalar radiusY,
                                            sk_sp<SkImageFilter> input,
                                            const CropRect& cropRect) {
    if (radiusX < 0 || radiusY < 0) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkMorphologyImageFilter(
            MorphType::kDilate, radiusX, radiusY, std::move(input), cropRect));
}

sk_sp<SkImageFilter> SkImageFilters::Erode(SkScalar radiusX, SkScalar radiusY,
                                           sk_sp<SkImageFilter> input,
                                           const CropRect& cropRect) {
    if (radiusX < 0 || radiusY < 0) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkMorphologyImageFilter(
            MorphType::kErode, radiusX, radiusY,  std::move(input), cropRect));
}

void SkRegisterMorphologyImageFilterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkMorphologyImageFilter);
    // TODO (michaelludwig): Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkMorphologyImageFilterImpl", SkMorphologyImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkMorphologyImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    SkScalar width = buffer.readScalar();
    SkScalar height = buffer.readScalar();
    MorphType filterType = buffer.read32LE(MorphType::kLastType);

    if (filterType == MorphType::kDilate) {
        return SkImageFilters::Dilate(width, height, common.getInput(0), common.cropRect());
    } else if (filterType == MorphType::kErode) {
        return SkImageFilters::Erode(width, height, common.getInput(0), common.cropRect());
    } else {
        return nullptr;
    }
}

void SkMorphologyImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fRadius.fWidth);
    buffer.writeScalar(fRadius.fHeight);
    buffer.writeInt(static_cast<int>(fType));
}

///////////////////////////////////////////////////////////////////////////////

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
    bounds.outset(fRadius.width(), fRadius.height());
    return bounds;
}

SkIRect SkMorphologyImageFilter::onFilterNodeBounds(
        const SkIRect& src, const SkMatrix& ctm, MapDirection, const SkIRect* inputRect) const {
    SkSize radius = mappedRadius(ctm);
    return src.makeOutset(SkScalarCeilToInt(radius.width()), SkScalarCeilToInt(radius.height()));
}

#if defined(SK_GANESH)

///////////////////////////////////////////////////////////////////////////////
/**
 * Morphology effects. Depending upon the type of morphology, either the
 * component-wise min (Erode_Type) or max (Dilate_Type) of all pixels in the
 * kernel is selected as the new color. The new color is modulated by the input
 * color.
 */
class GrMorphologyEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(
            std::unique_ptr<GrFragmentProcessor> inputFP, GrSurfaceProxyView view,
            SkAlphaType srcAlphaType, MorphDirection dir, int radius, MorphType type) {
        return std::unique_ptr<GrFragmentProcessor>(
                new GrMorphologyEffect(std::move(inputFP), std::move(view), srcAlphaType, dir,
                                       radius, type, /*range=*/nullptr));
    }

    static std::unique_ptr<GrFragmentProcessor> Make(
            std::unique_ptr<GrFragmentProcessor> inputFP, GrSurfaceProxyView view,
            SkAlphaType srcAlphaType, MorphDirection dir, int radius, MorphType type,
            const float range[2]) {
        return std::unique_ptr<GrFragmentProcessor>(new GrMorphologyEffect(
                std::move(inputFP), std::move(view), srcAlphaType, dir, radius, type, range));
    }

    const char* name() const override { return "Morphology"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new GrMorphologyEffect(*this));
    }

private:
    MorphDirection fDirection;
    int fRadius;
    MorphType fType;
    bool fUseRange;
    float fRange[2];

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;
    GrMorphologyEffect(std::unique_ptr<GrFragmentProcessor> inputFP, GrSurfaceProxyView,
                       SkAlphaType srcAlphaType, MorphDirection, int radius, MorphType,
                       const float range[2]);
    explicit GrMorphologyEffect(const GrMorphologyEffect&);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    using INHERITED = GrFragmentProcessor;
};

std::unique_ptr<GrFragmentProcessor::ProgramImpl> GrMorphologyEffect::onMakeProgramImpl() const {
    class Impl : public ProgramImpl {
    public:
        void emitCode(EmitArgs& args) override {
            constexpr int kInputFPIndex = 0;
            constexpr int kTexEffectIndex = 1;

            const GrMorphologyEffect& me = args.fFp.cast<GrMorphologyEffect>();

            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
            fRangeUni = uniformHandler->addUniform(&me, kFragment_GrShaderFlag, SkSLType::kFloat2,
                                                   "Range");
            const char* range = uniformHandler->getUniformCStr(fRangeUni);

            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

            const char* func = me.fType == MorphType::kErode ? "min" : "max";

            char initialValue = me.fType == MorphType::kErode ? '1' : '0';
            fragBuilder->codeAppendf("half4 color = half4(%c);", initialValue);

            char dir = me.fDirection == MorphDirection::kX ? 'x' : 'y';

            int width = 2 * me.fRadius + 1;

            // float2 coord = coord2D;
            fragBuilder->codeAppendf("float2 coord = %s;", args.fSampleCoord);
            // coord.x -= radius;
            fragBuilder->codeAppendf("coord.%c -= %d;", dir, me.fRadius);
            if (me.fUseRange) {
                // highBound = min(highBound, coord.x + (width-1));
                fragBuilder->codeAppendf("float highBound = min(%s.y, coord.%c + %f);", range, dir,
                                         float(width - 1));
                // coord.x = max(lowBound, coord.x);
                fragBuilder->codeAppendf("coord.%c = max(%s.x, coord.%c);", dir, range, dir);
            }
            fragBuilder->codeAppendf("for (int i = 0; i < %d; i++) {", width);
            SkString sample = this->invokeChild(kTexEffectIndex, args, "coord");
            fragBuilder->codeAppendf("    color = %s(color, %s);", func, sample.c_str());
            // coord.x += 1;
            fragBuilder->codeAppendf("    coord.%c += 1;", dir);
            if (me.fUseRange) {
                // coord.x = min(highBound, coord.x);
                fragBuilder->codeAppendf("    coord.%c = min(highBound, coord.%c);", dir, dir);
            }
            fragBuilder->codeAppend("}");

            SkString inputColor = this->invokeChild(kInputFPIndex, args);
            fragBuilder->codeAppendf("return color * %s;", inputColor.c_str());
        }

    private:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& proc) override {
            const GrMorphologyEffect& m = proc.cast<GrMorphologyEffect>();
            if (m.fUseRange) {
                pdman.set2f(fRangeUni, m.fRange[0], m.fRange[1]);
            }
        }

        GrGLSLProgramDataManager::UniformHandle fRangeUni;
    };

    return std::make_unique<Impl>();
}

void GrMorphologyEffect::onAddToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const {
    uint32_t key = static_cast<uint32_t>(fRadius);
    key |= (static_cast<uint32_t>(fType) << 8);
    key |= (static_cast<uint32_t>(fDirection) << 9);
    if (fUseRange) {
        key |= 1 << 10;
    }
    b->add32(key);
}

GrMorphologyEffect::GrMorphologyEffect(std::unique_ptr<GrFragmentProcessor> inputFP,
                                       GrSurfaceProxyView view,
                                       SkAlphaType srcAlphaType,
                                       MorphDirection direction,
                                       int radius,
                                       MorphType type,
                                       const float range[2])
        : INHERITED(kGrMorphologyEffect_ClassID, ModulateForClampedSamplerOptFlags(srcAlphaType))
        , fDirection(direction)
        , fRadius(radius)
        , fType(type)
        , fUseRange(SkToBool(range)) {
    this->setUsesSampleCoordsDirectly();
    this->registerChild(std::move(inputFP));
    this->registerChild(GrTextureEffect::Make(std::move(view), srcAlphaType),
                        SkSL::SampleUsage::Explicit());
    if (fUseRange) {
        fRange[0] = range[0];
        fRange[1] = range[1];
    }
}

GrMorphologyEffect::GrMorphologyEffect(const GrMorphologyEffect& that)
        : INHERITED(that)
        , fDirection(that.fDirection)
        , fRadius(that.fRadius)
        , fType(that.fType)
        , fUseRange(that.fUseRange) {
    if (that.fUseRange) {
        fRange[0] = that.fRange[0];
        fRange[1] = that.fRange[1];
    }
}

bool GrMorphologyEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrMorphologyEffect& s = sBase.cast<GrMorphologyEffect>();
    return this->fRadius    == s.fRadius    &&
           this->fDirection == s.fDirection &&
           this->fUseRange  == s.fUseRange  &&
           this->fType      == s.fType;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrMorphologyEffect)

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrMorphologyEffect::TestCreate(GrProcessorTestData* d) {
    auto [view, ct, at] = d->randomView();

    MorphDirection dir = d->fRandom->nextBool() ? MorphDirection::kX : MorphDirection::kY;
    static const int kMaxRadius = 10;
    int radius = d->fRandom->nextRangeU(1, kMaxRadius);
    MorphType type = d->fRandom->nextBool() ? MorphType::kErode : MorphType::kDilate;
    return GrMorphologyEffect::Make(d->inputFP(), std::move(view), at, dir, radius, type);
}
#endif

static void apply_morphology_rect(skgpu::ganesh::SurfaceFillContext* sfc,
                                  GrSurfaceProxyView view,
                                  SkAlphaType srcAlphaType,
                                  const SkIRect& srcRect,
                                  const SkIRect& dstRect,
                                  int radius,
                                  MorphType morphType,
                                  const float range[2],
                                  MorphDirection direction) {
    auto fp = GrMorphologyEffect::Make(/*inputFP=*/nullptr,
                                       std::move(view),
                                       srcAlphaType,
                                       direction,
                                       radius,
                                       morphType,
                                       range);
    sfc->fillRectToRectWithFP(srcRect, dstRect, std::move(fp));
}

static void apply_morphology_rect_no_bounds(skgpu::ganesh::SurfaceFillContext* sfc,
                                            GrSurfaceProxyView view,
                                            SkAlphaType srcAlphaType,
                                            const SkIRect& srcRect,
                                            const SkIRect& dstRect,
                                            int radius,
                                            MorphType morphType,
                                            MorphDirection direction) {
    auto fp = GrMorphologyEffect::Make(
            /*inputFP=*/nullptr, std::move(view), srcAlphaType, direction, radius, morphType);
    sfc->fillRectToRectWithFP(srcRect, dstRect, std::move(fp));
}

static void apply_morphology_pass(skgpu::ganesh::SurfaceFillContext* sfc,
                                  GrSurfaceProxyView view,
                                  SkAlphaType srcAlphaType,
                                  const SkIRect& srcRect,
                                  const SkIRect& dstRect,
                                  int radius,
                                  MorphType morphType,
                                  MorphDirection direction) {
    float bounds[2] = { 0.0f, 1.0f };
    SkIRect lowerSrcRect = srcRect, lowerDstRect = dstRect;
    SkIRect middleSrcRect = srcRect, middleDstRect = dstRect;
    SkIRect upperSrcRect = srcRect, upperDstRect = dstRect;
    if (direction == MorphDirection::kX) {
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
        apply_morphology_rect(sfc, std::move(view), srcAlphaType, srcRect,
                              dstRect, radius, morphType, bounds, direction);
    } else {
        // Draw upper and lower margins with bounds; middle without.
        apply_morphology_rect(sfc, view, srcAlphaType, lowerSrcRect,
                              lowerDstRect, radius, morphType, bounds, direction);
        apply_morphology_rect(sfc, view, srcAlphaType, upperSrcRect,
                              upperDstRect, radius, morphType, bounds, direction);
        apply_morphology_rect_no_bounds(sfc, std::move(view), srcAlphaType,
                                        middleSrcRect, middleDstRect, radius, morphType, direction);
    }
}

static sk_sp<SkSpecialImage> apply_morphology(
        GrRecordingContext* rContext, SkSpecialImage* input, const SkIRect& rect,
        MorphType morphType, SkISize radius, const SkImageFilter_Base::Context& ctx) {
    GrSurfaceProxyView srcView = input->view(rContext);
    SkAlphaType srcAlphaType = input->alphaType();
    SkASSERT(srcView.asTextureProxy());

    GrSurfaceProxy* proxy = srcView.proxy();

    const SkIRect dstRect = SkIRect::MakeWH(rect.width(), rect.height());
    SkIRect srcRect = rect;
    // Map into proxy space
    srcRect.offset(input->subset().x(), input->subset().y());
    SkASSERT(radius.width() > 0 || radius.height() > 0);

    GrImageInfo info(ctx.grColorType(), kPremul_SkAlphaType, ctx.refColorSpace(), rect.size());

    if (radius.fWidth > 0) {
        auto dstFillContext =
                rContext->priv().makeSFC(info,
                                         "SpecialImage_ApplyMorphology_Width",
                                         SkBackingFit::kApprox,
                                         1,
                                         GrMipmapped::kNo,
                                         proxy->isProtected(),
                                         kBottomLeft_GrSurfaceOrigin);
        if (!dstFillContext) {
            return nullptr;
        }

        apply_morphology_pass(dstFillContext.get(), std::move(srcView), srcAlphaType,
                              srcRect, dstRect, radius.fWidth, morphType, MorphDirection::kX);
        SkIRect clearRect = SkIRect::MakeXYWH(dstRect.fLeft, dstRect.fBottom,
                                              dstRect.width(), radius.fHeight);
        SkPMColor4f clearColor = MorphType::kErode == morphType
                ? SK_PMColor4fWHITE : SK_PMColor4fTRANSPARENT;
        dstFillContext->clear(clearRect, clearColor);

        srcView = dstFillContext->readSurfaceView();
        srcAlphaType = dstFillContext->colorInfo().alphaType();
        srcRect = dstRect;
    }
    if (radius.fHeight > 0) {
        auto dstFillContext =
                rContext->priv().makeSFC(info,
                                         "SpecialImage_ApplyMorphology_Height",
                                         SkBackingFit::kApprox,
                                         1,
                                         GrMipmapped::kNo,
                                         srcView.proxy()->isProtected(),
                                         kBottomLeft_GrSurfaceOrigin);
        if (!dstFillContext) {
            return nullptr;
        }

        apply_morphology_pass(dstFillContext.get(), std::move(srcView), srcAlphaType,
                              srcRect, dstRect, radius.fHeight, morphType, MorphDirection::kY);

        srcView = dstFillContext->readSurfaceView();
    }

    return SkSpecialImage::MakeDeferredFromGpu(rContext,
                                               SkIRect::MakeWH(rect.width(), rect.height()),
                                               kNeedNewImageUniqueID_SpecialImage,
                                               std::move(srcView),
                                               info.colorInfo(),
                                               input->props());
}
#endif

namespace {

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    template<MorphType type, MorphDirection direction>
    static void morph(const SkPMColor* src, SkPMColor* dst,
                      int radius, int width, int height, int srcStride, int dstStride) {
        const int srcStrideX = direction == MorphDirection::kX ? 1 : srcStride;
        const int dstStrideX = direction == MorphDirection::kX ? 1 : dstStride;
        const int srcStrideY = direction == MorphDirection::kX ? srcStride : 1;
        const int dstStrideY = direction == MorphDirection::kX ? dstStride : 1;
        radius = std::min(radius, width - 1);
        const SkPMColor* upperSrc = src + radius * srcStrideX;
        for (int x = 0; x < width; ++x) {
            const SkPMColor* lp = src;
            const SkPMColor* up = upperSrc;
            SkPMColor* dptr = dst;
            for (int y = 0; y < height; ++y) {
                __m128i extreme = (type == MorphType::kDilate) ? _mm_setzero_si128()
                                                               : _mm_set1_epi32(0xFFFFFFFF);
                for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                    __m128i src_pixel = _mm_cvtsi32_si128(*p);
                    extreme = (type == MorphType::kDilate) ? _mm_max_epu8(src_pixel, extreme)
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
        radius = std::min(radius, width - 1);
        const SkPMColor* upperSrc = src + radius * srcStrideX;
        for (int x = 0; x < width; ++x) {
            const SkPMColor* lp = src;
            const SkPMColor* up = upperSrc;
            SkPMColor* dptr = dst;
            for (int y = 0; y < height; ++y) {
                uint8x8_t extreme = vdup_n_u8(type == MorphType::kDilate ? 0 : 255);
                for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                    uint8x8_t src_pixel = vreinterpret_u8_u32(vdup_n_u32(*p));
                    extreme = (type == MorphType::kDilate) ? vmax_u8(src_pixel, extreme)
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
        radius = std::min(radius, width - 1);
        const SkPMColor* upperSrc = src + radius * srcStrideX;
        for (int x = 0; x < width; ++x) {
            const SkPMColor* lp = src;
            const SkPMColor* up = upperSrc;
            SkPMColor* dptr = dst;
            for (int y = 0; y < height; ++y) {
                // If we're maxing (dilate), start from 0; if minning (erode), start from 255.
                const int start = (type == MorphType::kDilate) ? 0 : 255;
                int B = start, G = start, R = start, A = start;
                for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                    int b = SkGetPackedB32(*p),
                        g = SkGetPackedG32(*p),
                        r = SkGetPackedR32(*p),
                        a = SkGetPackedA32(*p);
                    if (type == MorphType::kDilate) {
                        B = std::max(b, B);
                        G = std::max(g, G);
                        R = std::max(r, R);
                        A = std::max(a, A);
                    } else {
                        B = std::min(b, B);
                        G = std::min(g, G);
                        R = std::min(r, R);
                        A = std::min(a, A);
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

sk_sp<SkSpecialImage> SkMorphologyImageFilter::onFilterImage(const Context& ctx,
                                                             SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    SkIRect bounds;
    input = this->applyCropRectAndPad(this->mapContext(ctx), input.get(), &inputOffset, &bounds);
    if (!input) {
        return nullptr;
    }

    SkSize radius = mappedRadius(ctx.ctm());
    int width = SkScalarRoundToInt(radius.width());
    int height = SkScalarRoundToInt(radius.height());

    // Width (or height) must fit in a signed 32-bit int to avoid UBSAN issues (crbug.com/1018190)
    // Further, we limit the radius to something much smaller, to avoid extremely slow draw calls:
    // (crbug.com/1123035):
    constexpr int kMaxRadius = 100; // (std::numeric_limits<int>::max() - 1) / 2;

    if (width < 0 || height < 0 || width > kMaxRadius || height > kMaxRadius) {
        return nullptr;
    }

    SkIRect srcBounds = bounds;
    srcBounds.offset(-inputOffset);

    if (0 == width && 0 == height) {
        offset->fX = bounds.left();
        offset->fY = bounds.top();
        return input->makeSubset(srcBounds);
    }

#if defined(SK_GANESH)
    if (ctx.gpuBacked()) {
        auto context = ctx.getContext();

        // Ensure the input is in the destination color space. Typically applyCropRect will have
        // called pad_image to account for our dilation of bounds, so the result will already be
        // moved to the destination color space. If a filter DAG avoids that, then we use this
        // fall-back, which saves us from having to do the xform during the filter itself.
        input = ImageToColorSpace(ctx, input.get());

        sk_sp<SkSpecialImage> result(apply_morphology(context, input.get(), srcBounds, fType,
                                                      SkISize::Make(width, height), ctx));
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

    SkImageInfo info = SkImageInfo::Make(bounds.size(), inputBM.colorType(), inputBM.alphaType());

    SkBitmap dst;
    if (!dst.tryAllocPixels(info)) {
        return nullptr;
    }

    SkMorphologyImageFilter::Proc procX, procY;

    if (MorphType::kDilate == fType) {
        procX = &morph<MorphType::kDilate, MorphDirection::kX>;
        procY = &morph<MorphType::kDilate, MorphDirection::kY>;
    } else {
        procX = &morph<MorphType::kErode,  MorphDirection::kX>;
        procY = &morph<MorphType::kErode,  MorphDirection::kY>;
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
                                          dst, ctx.surfaceProps());
}

#else

#include "src/effects/imagefilters/SkCropImageFilter.h"

#ifdef SK_ENABLE_SKSL

#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkWriteBuffer.h"

#include <algorithm>
#include <cstdint>
#include <utility>

namespace {

enum class MorphType {
    kErode,
    kDilate,
    kLastType = kDilate
};

enum class MorphDirection { kX, kY };

class SkMorphologyImageFilter final : public SkImageFilter_Base {
public:
    SkMorphologyImageFilter(MorphType type, SkSize radii, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1, nullptr)
            , fType(type)
            , fRadii(radii) {}

    SkRect computeFastBounds(const SkRect& src) const override;


protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterMorphologyImageFilterFlattenables();
    SK_FLATTENABLE_HOOKS(SkMorphologyImageFilter)

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkISize> radii(const skif::Mapping& mapping) const {
        skif::LayerSpace<SkISize> radii = mapping.paramToLayer(fRadii).round();
        SkASSERT(radii.width() >= 0 && radii.height() >= 0);

        // We limit the radius to something small, to avoid slow draw calls: crbug.com/1123035
        static constexpr int kMaxRadii = 256;
        return skif::LayerSpace<SkISize>({std::min(radii.width(), kMaxRadii),
                                          std::min(radii.height(), kMaxRadii)});
    }

    skif::LayerSpace<SkIRect> requiredInput(const skif::Mapping& mapping,
                                            skif::LayerSpace<SkIRect> bounds) const {
        // The input for a morphology filter is always the kernel outset, regardless of morph type.
        bounds.outset(this->radii(mapping));
        return bounds;
    }

    skif::LayerSpace<SkIRect> kernelOutputBounds(const skif::Mapping& mapping,
                                                 skif::LayerSpace<SkIRect> bounds) const {
        skif::LayerSpace<SkISize> radii = this->radii(mapping);
        if (fType == MorphType::kDilate) {
            // Transparent pixels up to the kernel radius away will be overridden by kDilate's "max"
            // function and be set to the input's boundary pixel colors, thus expanding the output.
            bounds.outset(radii);
        } else {
            // Pixels closer than the kernel radius to the input image's edges are overridden by
            // kErode's "min" function and will be set to transparent black, contracting the output.
            bounds.inset(radii);
        }
        return bounds;
    }

    MorphType fType;
    skif::ParameterSpace<SkSize> fRadii;
};

sk_sp<SkImageFilter> make_morphology(MorphType type,
                                     SkSize radii,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilters::CropRect& cropRect) {
    if (radii.width() < 0.f || radii.height() < 0.f) {
        return nullptr; // invalid
    }
    sk_sp<SkImageFilter> filter = std::move(input);
    if (radii.width() > 0.f || radii.height() > 0.f) {
        filter = sk_sp<SkImageFilter>(new SkMorphologyImageFilter(type, radii, std::move(filter)));
    }
    // otherwise both radii are 0, so the kernel is always the identity function, in which case
    // we just need to apply the 'cropRect' to the 'input'.

    if (cropRect) {
        filter = SkMakeCropImageFilter(*cropRect, std::move(filter));
    }
    return filter;
}

// The linear morphology kernel does (2R+1) texture samples per pixel, which we want to keep less
// than the maximum fragment samples allowed in DX9SM2 (32), so we choose R=14 to have some head
// room. The other tradeoff is that for R > kMaxLinearRadius, the sparse morphology kernel only
// requires 2 samples to double the accumulated kernel size, but at the cost of another render
// target.
static constexpr int kMaxLinearRadius = 14;
sk_sp<SkShader> make_linear_morphology(sk_sp<SkShader> input,
                                       MorphType type,
                                       MorphDirection direction,
                                       int radius) {
    SkASSERT(radius <= kMaxLinearRadius);
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
            "const int kMaxLinearRadius = 14;" // KEEP IN SYNC WITH ABOVE DEFINITION

            "uniform shader child;"
            "uniform half2 offset;"
            "uniform half flip;" // -1 converts the max() calls to min()
            "uniform int radius;"

            "half4 main(float2 coord) {"
                "half4 aggregate = flip*child.eval(coord);" // case 0 only samples once
                "for (int i = 1; i <= kMaxLinearRadius; ++i) {"
                    "if (i > radius) break;"
                    "half2 delta = half(i) * offset;"
                    "aggregate = max(aggregate, max(flip*child.eval(coord + delta),"
                                                   "flip*child.eval(coord - delta)));"
                "}"
                "return flip*aggregate;"
            "}");

    SkRuntimeShaderBuilder builder(sk_ref_sp(effect));
    builder.child("child") = std::move(input);
    builder.uniform("offset") = direction == MorphDirection::kX ? SkV2{1.f, 0.f} : SkV2{0.f, 1.f};
    builder.uniform("flip") = (type == MorphType::kDilate) ? 1.f : -1.f;
    builder.uniform("radius") = (int32_t) radius;

    return builder.makeShader();
}

// Assuming 'input' was created by a series of morphology passes, each texel holds the aggregate
// (min or max depending on type) of (i-R) to (i+R) for some R. If 'radius' <= R, then the returned
// shader produces a new aggregate at each texel, i, of (i-R-radius) to (i+R+radius) with only two
// texture samples, which can be used to double the kernel size of the morphology effect.
sk_sp<SkShader> make_sparse_morphology(sk_sp<SkShader> input,
                                       MorphType type,
                                       MorphDirection direction,
                                       int radius) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader child;"
        "uniform half2 offset;"
        "uniform half flip;"

        "half4 main(float2 coord) {"
            "half4 aggregate = max(flip*child.eval(coord + offset),"
                                  "flip*child.eval(coord - offset));"
            "return flip*aggregate;"
        "}");

    SkRuntimeShaderBuilder builder(sk_ref_sp(effect));
    builder.child("child") = std::move(input);
    builder.uniform("offset") = direction == MorphDirection::kX ? SkV2{(float)radius, 0.f}
                                                                : SkV2{0.f, (float)radius};
    builder.uniform("flip") = (type == MorphType::kDilate) ? 1.f : -1.f;

    return builder.makeShader();
}

skif::FilterResult morphology_pass(const skif::Context& ctx, const skif::FilterResult& input,
                                   MorphType type, MorphDirection dir, int radius) {
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    auto axisDelta = [dir](int step) {
        return skif::LayerSpace<SkISize>({
                dir == MorphDirection::kX ? step : 0,
                dir == MorphDirection::kY ? step : 0});
    };

    // The first iteration will sample a full kernel outset from the final output.
    skif::LayerSpace<SkIRect> sampleBounds = ctx.desiredOutput();
    sampleBounds.outset(axisDelta(radius));

    skif::FilterResult childOutput = input;
    int appliedRadius = 0;
    while (radius > appliedRadius) {
        if (!childOutput) {
            return {}; // Eroded or dilated transparent black is still transparent black
        }

        // The first iteration uses up to kMaxLinearRadius with a linear accumulation pass.
        // After that we double the radius each step until we can finish with the target radius.
        int stepRadius =
                appliedRadius == 0 ? std::min(kMaxLinearRadius, radius)
                                   : std::min(radius - appliedRadius, appliedRadius);

        skif::Context stepCtx = ctx;
        if (appliedRadius + stepRadius < radius) {
            // Intermediate steps need to output what will be sampled on the next iteration
            auto outputBounds = sampleBounds;
            outputBounds.inset(axisDelta(stepRadius));
            stepCtx = ctx.withNewDesiredOutput(outputBounds);
        } // else the last iteration should output what was originally requested

        skif::FilterResult::Builder builder{stepCtx};
        builder.add(childOutput, sampleBounds);
        childOutput = builder.eval(
                [&](SkSpan<sk_sp<SkShader>> inputs) {
                    if (appliedRadius == 0) {
                        return make_linear_morphology(inputs[0], type, dir, stepRadius);
                    } else {
                        return make_sparse_morphology(inputs[0], type, dir, stepRadius);
                    }
                }, ShaderFlags::kForceResolveInputs);

        sampleBounds = stepCtx.desiredOutput();
        appliedRadius += stepRadius;
        SkASSERT(appliedRadius <= radius); // Our last iteration should hit 'radius' exactly.
    }

    return childOutput;
}

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Dilate(SkScalar radiusX, SkScalar radiusY,
                                            sk_sp<SkImageFilter> input,
                                            const CropRect& cropRect) {
    return make_morphology(MorphType::kDilate, {radiusX, radiusY}, std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Erode(SkScalar radiusX, SkScalar radiusY,
                                           sk_sp<SkImageFilter> input,
                                           const CropRect& cropRect) {
    return make_morphology(MorphType::kErode, {radiusX, radiusY}, std::move(input), cropRect);
}

void SkRegisterMorphologyImageFilterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkMorphologyImageFilter);
    // TODO (michaelludwig): Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkMorphologyImageFilterImpl", SkMorphologyImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkMorphologyImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    SkScalar width = buffer.readScalar();
    SkScalar height = buffer.readScalar();
    MorphType filterType = buffer.read32LE(MorphType::kLastType);

    if (filterType == MorphType::kDilate) {
        return SkImageFilters::Dilate(width, height, common.getInput(0), common.cropRect());
    } else if (filterType == MorphType::kErode) {
        return SkImageFilters::Erode(width, height, common.getInput(0), common.cropRect());
    } else {
        return nullptr;
    }
}

void SkMorphologyImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeScalar(SkSize(fRadii).width());
    buffer.writeScalar(SkSize(fRadii).height());
    buffer.writeInt(static_cast<int>(fType));
}

///////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkMorphologyImageFilter::onFilterImage(const skif::Context& ctx) const {
    skif::LayerSpace<SkIRect> requiredInput =
            this->requiredInput(ctx.mapping(), ctx.desiredOutput());
    skif::FilterResult childOutput =
            this->getChildOutput(0, ctx.withNewDesiredOutput(requiredInput));

    // If childOutput completely fulfilled requiredInput, maxOutput will match the context's
    // desired output, but if the output image is smaller, this will restrict the morphology output
    // to what is actual produceable.
    skif::LayerSpace<SkIRect> maxOutput =
        this->kernelOutputBounds(ctx.mapping(), childOutput.layerBounds());
    if (!maxOutput.intersect(ctx.desiredOutput())) {
        return {};
    }

    // The X pass has to preserve the extra rows to later be consumed by the Y pass.
    skif::LayerSpace<SkISize> radii = this->radii(ctx.mapping());
    skif::LayerSpace<SkIRect> maxOutputX = maxOutput;
    maxOutputX.outset(skif::LayerSpace<SkISize>({0, radii.height()}));
    childOutput = morphology_pass(ctx.withNewDesiredOutput(maxOutputX), childOutput, fType,
                                  MorphDirection::kX, radii.width());
    childOutput = morphology_pass(ctx.withNewDesiredOutput(maxOutput), childOutput, fType,
                                  MorphDirection::kY, radii.height());
    return childOutput;
}

skif::LayerSpace<SkIRect> SkMorphologyImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    skif::LayerSpace<SkIRect> requiredInput = this->requiredInput(mapping, desiredOutput);
    return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
}

skif::LayerSpace<SkIRect> SkMorphologyImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    skif::LayerSpace<SkIRect> childOutput =
            this->getChildOutputLayerBounds(0, mapping, contentBounds);
    return this->kernelOutputBounds(mapping, childOutput);
}

SkRect SkMorphologyImageFilter::computeFastBounds(const SkRect& src) const {
    // See kernelOutputBounds() for rationale
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    if (fType == MorphType::kDilate) {
        bounds.outset(SkSize(fRadii).width(), SkSize(fRadii).height());
    } else {
        bounds.inset(SkSize(fRadii).width(), SkSize(fRadii).height());
    }
    return bounds;
}

#else

// The morphology effects requires SkSL, just return the input, possibly cropped
sk_sp<SkImageFilter> SkImageFilters::Dilate(SkScalar radiusX, SkScalar radiusY,
                                            sk_sp<SkImageFilter> input,
                                            const CropRect& cropRect) {
    return cropRect ? SkMakeCropImageFilter(*cropRect, std::move(input)) : input;
}

sk_sp<SkImageFilter> SkImageFilters::Erode(SkScalar radiusX, SkScalar radiusY,
                                           sk_sp<SkImageFilter> input,
                                           const CropRect& cropRect) {
    return cropRect ? SkMakeCropImageFilter(*cropRect, std::move(input)) : input;
}

void SkRegisterMorphologyImageFilterFlattenables() {}

#endif

#endif // SK_USE_LEGACY_MORPHOLOGY_IMAGEFILTER
