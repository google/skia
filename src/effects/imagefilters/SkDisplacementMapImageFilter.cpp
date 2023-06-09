/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#if defined(SK_USE_LEGACY_DISPLACEMENT_MAP_IMAGEFILTER)

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
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkSafe32.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"

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
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
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

namespace {

class SkDisplacementMapImageFilter final : public SkImageFilter_Base {
public:
    SkDisplacementMapImageFilter(SkColorChannel xChannelSelector, SkColorChannel yChannelSelector,
                                 SkScalar scale, sk_sp<SkImageFilter> inputs[2],
                                 const SkRect* cropRect)
            : INHERITED(inputs, 2, cropRect)
            , fXChannelSelector(xChannelSelector)
            , fYChannelSelector(yChannelSelector)
            , fScale(scale) {}

    SkRect computeFastBounds(const SkRect& src) const override;

    SkIRect onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;
    SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterDisplacementMapImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkDisplacementMapImageFilter)

    SkColorChannel fXChannelSelector;
    SkColorChannel fYChannelSelector;
    SkScalar fScale;

    const SkImageFilter* getDisplacementInput() const { return getInput(0); }
    const SkImageFilter* getColorInput() const { return getInput(1); }

    using INHERITED = SkImageFilter_Base;
};

// Shift values to extract channels from an SkColor (SkColorGetR, SkColorGetG, etc)
const uint8_t gChannelTypeToShift[] = {
    16,  // R
     8,  // G
     0,  // B
    24,  // A
};
struct Extractor {
    Extractor(SkColorChannel typeX,
              SkColorChannel typeY)
        : fShiftX(gChannelTypeToShift[static_cast<int>(typeX)])
        , fShiftY(gChannelTypeToShift[static_cast<int>(typeY)])
    {}

    unsigned fShiftX, fShiftY;

    unsigned getX(SkColor c) const { return (c >> fShiftX) & 0xFF; }
    unsigned getY(SkColor c) const { return (c >> fShiftY) & 0xFF; }
};

static bool channel_selector_type_is_valid(SkColorChannel cst) {
    switch (cst) {
        case SkColorChannel::kR:
        case SkColorChannel::kG:
        case SkColorChannel::kB:
        case SkColorChannel::kA:
            return true;
        default:
            break;
    }
    return false;
}

}  // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkImageFilters::DisplacementMap(
        SkColorChannel xChannelSelector, SkColorChannel yChannelSelector, SkScalar scale,
        sk_sp<SkImageFilter> displacement, sk_sp<SkImageFilter> color, const CropRect& cropRect) {
    if (!channel_selector_type_is_valid(xChannelSelector) ||
        !channel_selector_type_is_valid(yChannelSelector)) {
        return nullptr;
    }

    sk_sp<SkImageFilter> inputs[2] = { std::move(displacement), std::move(color) };
    return sk_sp<SkImageFilter>(new SkDisplacementMapImageFilter(xChannelSelector, yChannelSelector,
                                                                 scale, inputs, cropRect));
}

void SkRegisterDisplacementMapImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkDisplacementMapImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkDisplacementMapEffect", SkDisplacementMapImageFilter::CreateProc);
    SkFlattenable::Register("SkDisplacementMapEffectImpl",
                            SkDisplacementMapImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkDisplacementMapImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);

    SkColorChannel xsel = buffer.read32LE(SkColorChannel::kLastEnum);
    SkColorChannel ysel = buffer.read32LE(SkColorChannel::kLastEnum);
    SkScalar      scale = buffer.readScalar();

    return SkImageFilters::DisplacementMap(xsel, ysel, scale, common.getInput(0),
                                           common.getInput(1), common.cropRect());
}

void SkDisplacementMapImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt((int) fXChannelSelector);
    buffer.writeInt((int) fYChannelSelector);
    buffer.writeScalar(fScale);
}

///////////////////////////////////////////////////////////////////////////////

#if defined(SK_GANESH)

namespace {

class GrDisplacementMapEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(SkColorChannel xChannelSelector,
                                                     SkColorChannel yChannelSelector,
                                                     SkVector scale,
                                                     GrSurfaceProxyView displacement,
                                                     const SkIRect& displSubset,
                                                     const SkMatrix& offsetMatrix,
                                                     GrSurfaceProxyView color,
                                                     const SkIRect& colorSubset,
                                                     const GrCaps&);

    ~GrDisplacementMapEffect() override;

    const char* name() const override { return "DisplacementMap"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    class Impl;

    explicit GrDisplacementMapEffect(const GrDisplacementMapEffect&);

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GrDisplacementMapEffect(SkColorChannel xChannelSelector,
                            SkColorChannel yChannelSelector,
                            const SkVector& scale,
                            std::unique_ptr<GrFragmentProcessor> displacement,
                            std::unique_ptr<GrFragmentProcessor> color);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    SkColorChannel fXChannelSelector;
    SkColorChannel fYChannelSelector;
    SkVector fScale;

    using INHERITED = GrFragmentProcessor;
};

}  // anonymous namespace
#endif

static void compute_displacement(Extractor ex, const SkVector& scale, SkBitmap* dst,
                                 const SkBitmap& displ, const SkIPoint& offset,
                                 const SkBitmap& src,
                                 const SkIRect& bounds) {
    static const SkScalar Inv8bit = SkScalarInvert(255);
    const int srcW = src.width();
    const int srcH = src.height();
    const SkVector scaleForColor = SkVector::Make(scale.fX * Inv8bit, scale.fY * Inv8bit);
    const SkVector scaleAdj = SkVector::Make(SK_ScalarHalf - scale.fX * SK_ScalarHalf,
                                             SK_ScalarHalf - scale.fY * SK_ScalarHalf);
    SkPMColor* dstPtr = dst->getAddr32(0, 0);
    for (int y = bounds.top(); y < bounds.bottom(); ++y) {
        const SkPMColor* displPtr = displ.getAddr32(bounds.left() + offset.fX, y + offset.fY);
        for (int x = bounds.left(); x < bounds.right(); ++x, ++displPtr) {
            SkColor c = SkUnPreMultiply::PMColorToColor(*displPtr);

            SkScalar displX = scaleForColor.fX * ex.getX(c) + scaleAdj.fX;
            SkScalar displY = scaleForColor.fY * ex.getY(c) + scaleAdj.fY;
            // Truncate the displacement values
            const int32_t srcX = Sk32_sat_add(x, SkScalarTruncToInt(displX));
            const int32_t srcY = Sk32_sat_add(y, SkScalarTruncToInt(displY));
            *dstPtr++ = ((srcX < 0) || (srcX >= srcW) || (srcY < 0) || (srcY >= srcH)) ?
                      0 : *(src.getAddr32(srcX, srcY));
        }
    }
}

sk_sp<SkSpecialImage> SkDisplacementMapImageFilter::onFilterImage(const Context& ctx,
                                                                  SkIPoint* offset) const {
    SkIPoint colorOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> color(this->filterInput(1, ctx, &colorOffset));
    if (!color) {
        return nullptr;
    }

    SkIPoint displOffset = SkIPoint::Make(0, 0);
    // Creation of the displacement map should happen in a non-colorspace aware context. This
    // texture is a purely mathematical construct, so we want to just operate on the stored
    // values. Consider:
    // User supplies an sRGB displacement map. If we're rendering to a wider gamut, then we could
    // end up filtering the displacement map into that gamut, which has the effect of reducing
    // the amount of displacement that it represents (as encoded values move away from the
    // primaries).
    // With a more complex DAG attached to this input, it's not clear that working in ANY specific
    // color space makes sense, so we ignore color spaces (and gamma) entirely. This may not be
    // ideal, but it's at least consistent and predictable.
    Context displContext = ctx.withNewColorSpace(/*cs=*/nullptr);
    sk_sp<SkSpecialImage> displ(this->filterInput(0, displContext, &displOffset));
    if (!displ) {
        return nullptr;
    }

    const SkIRect srcBounds = SkIRect::MakeXYWH(colorOffset.x(), colorOffset.y(),
                                                color->width(), color->height());

    // Both paths do bounds checking on color pixel access, we don't need to
    // pad the color bitmap to bounds here.
    SkIRect bounds;
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return nullptr;
    }

    SkIRect displBounds;
    displ = this->applyCropRectAndPad(ctx, displ.get(), &displOffset, &displBounds);
    if (!displ) {
        return nullptr;
    }

    if (!bounds.intersect(displBounds)) {
        return nullptr;
    }

    const SkIRect colorBounds = bounds.makeOffset(-colorOffset);
    // If the offset overflowed (saturated) then we have to abort, as we need their
    // dimensions to be equal. See https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=7209
    if (colorBounds.size() != bounds.size()) {
        return nullptr;
    }

    SkVector scale = SkVector::Make(fScale, fScale);
    ctx.ctm().mapVectors(&scale, 1);

#if defined(SK_GANESH)
    if (ctx.gpuBacked()) {
        auto rContext = ctx.getContext();

        GrSurfaceProxyView colorView = color->view(rContext);
        GrSurfaceProxyView displView = displ->view(rContext);
        if (!colorView.proxy() || !displView.proxy()) {
            return nullptr;
        }
        const auto isProtected = colorView.proxy()->isProtected();

        SkMatrix offsetMatrix = SkMatrix::Translate(SkIntToScalar(colorOffset.fX - displOffset.fX),
                                                    SkIntToScalar(colorOffset.fY - displOffset.fY));

        std::unique_ptr<GrFragmentProcessor> fp =
                GrDisplacementMapEffect::Make(fXChannelSelector,
                                              fYChannelSelector,
                                              scale,
                                              std::move(displView),
                                              displ->subset(),
                                              offsetMatrix,
                                              std::move(colorView),
                                              color->subset(),
                                              *rContext->priv().caps());
        fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                           color->getColorSpace(), color->alphaType(),
                                           ctx.colorSpace(), kPremul_SkAlphaType);
        GrImageInfo info(ctx.grColorType(),
                         kPremul_SkAlphaType,
                         ctx.refColorSpace(),
                         bounds.size());
        auto sfc = rContext->priv().makeSFC(info,
                                            "DisplacementMapImageFilter_FilterImage",
                                            SkBackingFit::kApprox,
                                            1,
                                            GrMipmapped::kNo,
                                            isProtected,
                                            kBottomLeft_GrSurfaceOrigin);
        if (!sfc) {
            return nullptr;
        }

        sfc->fillRectToRectWithFP(colorBounds,
                                  SkIRect::MakeSize(colorBounds.size()),
                                  std::move(fp));

        offset->fX = bounds.left();
        offset->fY = bounds.top();
        return SkSpecialImage::MakeDeferredFromGpu(rContext,
                                                   SkIRect::MakeWH(bounds.width(), bounds.height()),
                                                   kNeedNewImageUniqueID_SpecialImage,
                                                   sfc->readSurfaceView(),
                                                   sfc->colorInfo(),
                                                   ctx.surfaceProps());
    }
#endif

    SkBitmap colorBM, displBM;

    if (!color->getROPixels(&colorBM) || !displ->getROPixels(&displBM)) {
        return nullptr;
    }

    if ((colorBM.colorType() != kN32_SkColorType) ||
        (displBM.colorType() != kN32_SkColorType)) {
        return nullptr;
    }

    if (!colorBM.getPixels() || !displBM.getPixels()) {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::MakeN32(bounds.width(), bounds.height(),
                                            colorBM.alphaType());

    SkBitmap dst;
    if (!dst.tryAllocPixels(info)) {
        return nullptr;
    }

    compute_displacement(Extractor(fXChannelSelector, fYChannelSelector), scale, &dst,
                         displBM, colorOffset - displOffset, colorBM, colorBounds);

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(bounds.width(), bounds.height()),
                                          dst, ctx.surfaceProps());
}

SkRect SkDisplacementMapImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getColorInput() ? this->getColorInput()->computeFastBounds(src) : src;
    bounds.outset(SkScalarAbs(fScale) * SK_ScalarHalf, SkScalarAbs(fScale) * SK_ScalarHalf);
    return bounds;
}

SkIRect SkDisplacementMapImageFilter::onFilterNodeBounds(
        const SkIRect& src, const SkMatrix& ctm, MapDirection, const SkIRect* inputRect) const {
    SkVector scale = SkVector::Make(fScale, fScale);
    ctm.mapVectors(&scale, 1);
    return src.makeOutset(SkScalarCeilToInt(SkScalarAbs(scale.fX) * SK_ScalarHalf),
                          SkScalarCeilToInt(SkScalarAbs(scale.fY) * SK_ScalarHalf));
}

SkIRect SkDisplacementMapImageFilter::onFilterBounds(
        const SkIRect& src, const SkMatrix& ctm, MapDirection dir, const SkIRect* inputRect) const {
    if (kReverse_MapDirection == dir) {
        return INHERITED::onFilterBounds(src, ctm, dir, inputRect);
    }
    // Recurse only into color input.
    if (this->getColorInput()) {
        return this->getColorInput()->filterBounds(src, ctm, dir, inputRect);
    }
    return src;
}

///////////////////////////////////////////////////////////////////////////////

#if defined(SK_GANESH)
class GrDisplacementMapEffect::Impl : public ProgramImpl {
public:
    void emitCode(EmitArgs&) override;

private:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

    UniformHandle fScaleUni;
};

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrDisplacementMapEffect::Make(SkColorChannel xChannelSelector,
                                                                   SkColorChannel yChannelSelector,
                                                                   SkVector scale,
                                                                   GrSurfaceProxyView displacement,
                                                                   const SkIRect& displSubset,
                                                                   const SkMatrix& offsetMatrix,
                                                                   GrSurfaceProxyView color,
                                                                   const SkIRect& colorSubset,
                                                                   const GrCaps& caps) {
    static constexpr GrSamplerState kColorSampler(GrSamplerState::WrapMode::kClampToBorder,
                                                  GrSamplerState::Filter::kNearest);
    auto colorEffect = GrTextureEffect::MakeSubset(std::move(color),
                                                   kPremul_SkAlphaType,
                                                   SkMatrix::Translate(colorSubset.topLeft()),
                                                   kColorSampler,
                                                   SkRect::Make(colorSubset),
                                                   caps);

    auto dispM = SkMatrix::Concat(SkMatrix::Translate(displSubset.topLeft()), offsetMatrix);
    auto dispEffect = GrTextureEffect::Make(std::move(displacement),
                                            kPremul_SkAlphaType,
                                            dispM,
                                            GrSamplerState::Filter::kNearest);

    return std::unique_ptr<GrFragmentProcessor>(
            new GrDisplacementMapEffect(xChannelSelector,
                                        yChannelSelector,
                                        scale,
                                        std::move(dispEffect),
                                        std::move(colorEffect)));
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl>
GrDisplacementMapEffect::onMakeProgramImpl() const {
    return std::make_unique<Impl>();
}

void GrDisplacementMapEffect::onAddToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const {
    static constexpr int kChannelSelectorKeyBits = 2;  // Max value is 3, so 2 bits are required

    uint32_t xKey = static_cast<uint32_t>(fXChannelSelector);
    uint32_t yKey = static_cast<uint32_t>(fYChannelSelector) << kChannelSelectorKeyBits;

    b->add32(xKey | yKey);
}

GrDisplacementMapEffect::GrDisplacementMapEffect(SkColorChannel xChannelSelector,
                                                 SkColorChannel yChannelSelector,
                                                 const SkVector& scale,
                                                 std::unique_ptr<GrFragmentProcessor> displacement,
                                                 std::unique_ptr<GrFragmentProcessor> color)
        : INHERITED(kGrDisplacementMapEffect_ClassID, GrFragmentProcessor::kNone_OptimizationFlags)
        , fXChannelSelector(xChannelSelector)
        , fYChannelSelector(yChannelSelector)
        , fScale(scale) {
    this->registerChild(std::move(displacement));
    this->registerChild(std::move(color), SkSL::SampleUsage::Explicit());
    this->setUsesSampleCoordsDirectly();
}

GrDisplacementMapEffect::GrDisplacementMapEffect(const GrDisplacementMapEffect& that)
        : INHERITED(that)
        , fXChannelSelector(that.fXChannelSelector)
        , fYChannelSelector(that.fYChannelSelector)
        , fScale(that.fScale) {}

GrDisplacementMapEffect::~GrDisplacementMapEffect() {}

std::unique_ptr<GrFragmentProcessor> GrDisplacementMapEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrDisplacementMapEffect(*this));
}

bool GrDisplacementMapEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrDisplacementMapEffect& s = sBase.cast<GrDisplacementMapEffect>();
    return fXChannelSelector == s.fXChannelSelector &&
           fYChannelSelector == s.fYChannelSelector &&
           fScale == s.fScale;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrDisplacementMapEffect)

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrDisplacementMapEffect::TestCreate(GrProcessorTestData* d) {
    auto [dispView,  ct1, at1] = d->randomView();
    auto [colorView, ct2, at2] = d->randomView();
    static const int kMaxComponent = static_cast<int>(SkColorChannel::kLastEnum);
    SkColorChannel xChannelSelector =
        static_cast<SkColorChannel>(d->fRandom->nextRangeU(1, kMaxComponent));
    SkColorChannel yChannelSelector =
        static_cast<SkColorChannel>(d->fRandom->nextRangeU(1, kMaxComponent));
    SkVector scale;
    scale.fX = d->fRandom->nextRangeScalar(0, 100.0f);
    scale.fY = d->fRandom->nextRangeScalar(0, 100.0f);
    SkISize colorDimensions;
    colorDimensions.fWidth = d->fRandom->nextRangeU(0, colorView.width());
    colorDimensions.fHeight = d->fRandom->nextRangeU(0, colorView.height());
    SkIRect dispRect = SkIRect::MakeSize(dispView.dimensions());

    return GrDisplacementMapEffect::Make(xChannelSelector,
                                         yChannelSelector,
                                         scale,
                                         std::move(dispView),
                                         dispRect,
                                         SkMatrix::I(),
                                         std::move(colorView),
                                         SkIRect::MakeSize(colorDimensions),
                                         *d->caps());
}

#endif

///////////////////////////////////////////////////////////////////////////////

void GrDisplacementMapEffect::Impl::emitCode(EmitArgs& args) {
    const GrDisplacementMapEffect& displacementMap = args.fFp.cast<GrDisplacementMapEffect>();

    fScaleUni = args.fUniformHandler->addUniform(&displacementMap, kFragment_GrShaderFlag,
                                                 SkSLType::kHalf2, "Scale");
    const char* scaleUni = args.fUniformHandler->getUniformCStr(fScaleUni);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString displacementSample = this->invokeChild(/*childIndex=*/0, args);
    fragBuilder->codeAppendf("half4 dColor = unpremul(%s);", displacementSample.c_str());

    auto chanChar = [](SkColorChannel c) {
        switch(c) {
            case SkColorChannel::kR: return 'r';
            case SkColorChannel::kG: return 'g';
            case SkColorChannel::kB: return 'b';
            case SkColorChannel::kA: return 'a';
            default: SkUNREACHABLE;
        }
    };
    fragBuilder->codeAppendf("float2 cCoords = %s + %s * (dColor.%c%c - half2(0.5));",
                             args.fSampleCoord,
                             scaleUni,
                             chanChar(displacementMap.fXChannelSelector),
                             chanChar(displacementMap.fYChannelSelector));

    SkString colorSample = this->invokeChild(/*childIndex=*/1, args, "cCoords");

    fragBuilder->codeAppendf("return %s;", colorSample.c_str());
}

void GrDisplacementMapEffect::Impl::onSetData(const GrGLSLProgramDataManager& pdman,
                                              const GrFragmentProcessor& proc) {
    const auto& displacementMap = proc.cast<GrDisplacementMapEffect>();
    pdman.set2f(fScaleUni, displacementMap.fScale.x(), displacementMap.fScale.y());
}
#endif

#else

#include "src/effects/imagefilters/SkCropImageFilter.h"

#ifdef SK_ENABLE_SKSL

#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
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

#include <utility>

namespace {

class SkDisplacementMapImageFilter final : public SkImageFilter_Base {
    // Input image filter indices
    static constexpr int kDisplacement = 0;
    static constexpr int kColor = 1;

    // TODO(skbug.com/14376): Use nearest to match historical behavior, but eventually this should
    // become a factory option.
    static constexpr SkSamplingOptions kDisplacementSampling{SkFilterMode::kNearest};

public:
    SkDisplacementMapImageFilter(SkColorChannel xChannel, SkColorChannel yChannel,
                                 SkScalar scale, sk_sp<SkImageFilter> inputs[2])
            : SkImageFilter_Base(inputs, 2, nullptr)
            , fXChannel(xChannel)
            , fYChannel(yChannel)
            , fScale(scale) {}

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterDisplacementMapImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkDisplacementMapImageFilter)

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> outsetByMaxDisplacement(const skif::Mapping& mapping,
                                                      skif::LayerSpace<SkIRect> bounds) const {
        // For max displacement, we treat 'scale' as a size instead of a vector. The vector offset
        // maps a [0,1] channel value to [-scale/2, scale/2], and treating it as a size
        // automatically accounts for the absolute magnitude when transforming from param to layer.
        skif::LayerSpace<SkSize> maxDisplacement = mapping.paramToLayer(
            skif::ParameterSpace<SkSize>({0.5f * fScale, 0.5f * fScale}));
        bounds.outset(maxDisplacement.ceil());
        return bounds;
    }

    SkColorChannel fXChannel;
    SkColorChannel fYChannel;
    // Scale is really a ParameterSpace<Vector> where width = height = fScale, but we store just the
    // float here for easier serialization and convert to a size in onFilterImage().
    SkScalar fScale;
};

bool channel_selector_type_is_valid(SkColorChannel cst) {
    switch (cst) {
        case SkColorChannel::kR:
        case SkColorChannel::kG:
        case SkColorChannel::kB:
        case SkColorChannel::kA:
            return true;
        default:
            break;
    }
    return false;
}

sk_sp<SkShader> make_displacement_shader(
        sk_sp<SkShader> displacement,
        sk_sp<SkShader> color,
        skif::LayerSpace<skif::Vector> scale,
        SkColorChannel xChannel,
        SkColorChannel yChannel) {
    if (!color) {
        // Color is fully transparent, so no point in displacing it
        return nullptr;
    }
    if (!displacement) {
        // Somehow we had a valid displacement image but failed to produce a shader
        // (e.g. an internal resolve to a new image failed). Treat the displacement as
        // transparent, but it's too late to switch to the applyTransform() optimization.
        displacement = SkShaders::Color(SK_ColorTRANSPARENT);
    }

    // NOTE: This uses dot product selection to work on all GLES2 hardware (enforced by public
    // runtime effect restrictions). Otherwise, this would use a "uniform ivec2" and component
    // indexing to convert the displacement color into a vector.
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader displMap;"
        "uniform shader colorMap;"
        "uniform half2 scale;"
        "uniform half4 xSelect;" // Only one of RGBA will be 1, the rest are 0
        "uniform half4 ySelect;"

        "half4 main(float2 coord) {"
            "half4 displColor = unpremul(displMap.eval(coord));"
            "half2 displ = half2(dot(displColor, xSelect), dot(displColor, ySelect));"
            "displ = scale * (displ - 0.5);"
            "return colorMap.eval(coord + displ);"
        "}");

    auto channelSelector = [](SkColorChannel c) {
        return SkV4{c == SkColorChannel::kR ? 1.f : 0.f,
                    c == SkColorChannel::kG ? 1.f : 0.f,
                    c == SkColorChannel::kB ? 1.f : 0.f,
                    c == SkColorChannel::kA ? 1.f : 0.f};
    };

    SkRuntimeShaderBuilder builder(sk_ref_sp(effect));
    builder.child("displMap") = std::move(displacement);
    builder.child("colorMap") = std::move(color);
    builder.uniform("scale") = SkV2{scale.x(), scale.y()};
    builder.uniform("xSelect") = channelSelector(xChannel);
    builder.uniform("ySelect") = channelSelector(yChannel);

    return builder.makeShader();
}

}  // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkImageFilters::DisplacementMap(
        SkColorChannel xChannelSelector, SkColorChannel yChannelSelector, SkScalar scale,
        sk_sp<SkImageFilter> displacement, sk_sp<SkImageFilter> color, const CropRect& cropRect) {
    if (!channel_selector_type_is_valid(xChannelSelector) ||
        !channel_selector_type_is_valid(yChannelSelector)) {
        return nullptr;
    }

    sk_sp<SkImageFilter> inputs[2] = { std::move(displacement), std::move(color) };
    sk_sp<SkImageFilter> filter(new SkDisplacementMapImageFilter(xChannelSelector, yChannelSelector,
                                                                 scale, inputs));
    if (cropRect) {
        filter = SkMakeCropImageFilter(*cropRect, std::move(filter));
    }
    return filter;
}

void SkRegisterDisplacementMapImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkDisplacementMapImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkDisplacementMapEffect", SkDisplacementMapImageFilter::CreateProc);
    SkFlattenable::Register("SkDisplacementMapEffectImpl",
                            SkDisplacementMapImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkDisplacementMapImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);

    SkColorChannel xsel = buffer.read32LE(SkColorChannel::kLastEnum);
    SkColorChannel ysel = buffer.read32LE(SkColorChannel::kLastEnum);
    SkScalar      scale = buffer.readScalar();

    return SkImageFilters::DisplacementMap(xsel, ysel, scale, common.getInput(0),
                                           common.getInput(1), common.cropRect());
}

void SkDisplacementMapImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeInt((int) fXChannel);
    buffer.writeInt((int) fYChannel);
    buffer.writeScalar(fScale);
}

///////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkDisplacementMapImageFilter::onFilterImage(const skif::Context& ctx) const {
    skif::LayerSpace<SkIRect> requiredColorInput =
            this->outsetByMaxDisplacement(ctx.mapping(), ctx.desiredOutput());
    skif::FilterResult colorOutput =
            this->getChildOutput(kColor, ctx.withNewDesiredOutput(requiredColorInput));
    if (!colorOutput) {
        return {}; // No non-transparent black colors to displace
    }

    // When the color image filter is unrestricted, its output will be 'maxDisplacement' larger than
    // this filter's desired output. However, if it is cropped, we can restrict this filter's final
    // output. However it's not simply colorOutput intersected with desiredOutput since we have to
    // account for how the clipped colorOutput might still be displaced.
    skif::LayerSpace<SkIRect> outputBounds =
            this->outsetByMaxDisplacement(ctx.mapping(), colorOutput.layerBounds());
    // 'outputBounds' has double the max displacement for edges where colorOutput had not been
    // clipped, but that's fine since we intersect with 'desiredOutput'. For edges that were cropped
    // the second max displacement represents how far they can be displaced, which might be inside
    // the original 'desiredOutput'.
    if (!outputBounds.intersect(ctx.desiredOutput())) {
        // None of the non-transparent black colors can be displaced into the desired bounds.
        return {};
    }

    // Creation of the displacement map should happen in a non-colorspace aware context. This
    // texture is a purely mathematical construct, so we want to just operate on the stored
    // values. Consider:
    //
    //   User supplies an sRGB displacement map. If we're rendering to a wider gamut, then we could
    //   end up filtering the displacement map into that gamut, which has the effect of reducing
    //   the amount of displacement that it represents (as encoded values move away from the
    //   primaries).
    //
    //   With a more complex DAG attached to this input, it's not clear that working in ANY specific
    //   color space makes sense, so we ignore color spaces (and gamma) entirely. This may not be
    //   ideal, but it's at least consistent and predictable.
    skif::FilterResult displacementOutput =
            this->getChildOutput(kDisplacement, ctx.withNewDesiredOutput(outputBounds)
                                                   .withNewColorSpace(/*cs=*/nullptr));

    // NOTE: The scale is a "vector" not a "size" since we want to preserve negations on the final
    // displacement vector.
    const skif::LayerSpace<skif::Vector> scale =
            ctx.mapping().paramToLayer(skif::ParameterSpace<skif::Vector>({fScale, fScale}));
    if (!displacementOutput) {
        // A null displacement map means its transparent black, but (0,0,0,0) becomes the vector
        // (-scale/2, -scale/2) applied to the color image, so represent the displacement as a
        // simple transform.
        skif::LayerSpace<SkMatrix> constantDisplacement{SkMatrix::Translate(-0.5f * scale.x(),
                                                                            -0.5f * scale.y())};
        return colorOutput.applyTransform(ctx, constantDisplacement, kDisplacementSampling);
    }

    // If we made it this far, then we actually have per-pixel displacement affecting the color
    // image. We need to evaluate each pixel within 'outputBounds'.
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    skif::FilterResult::Builder builder{ctx};
    builder.add(displacementOutput, /*sampleBounds=*/outputBounds);
    builder.add(colorOutput,
                /*sampleBounds=*/requiredColorInput,
                ShaderFlags::kNonLinearSampling,
                kDisplacementSampling);
    return builder.eval(
            [&](SkSpan<sk_sp<SkShader>> inputs) {
                return make_displacement_shader(inputs[kDisplacement], inputs[kColor],
                                                scale, fXChannel, fYChannel);
            }, ShaderFlags::kExplicitOutputBounds, outputBounds);
}

skif::LayerSpace<SkIRect> SkDisplacementMapImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // Pixels up to the maximum displacement away from 'desiredOutput' can be moved into those
    // bounds, depending on how the displacement map renders. To ensure those colors are defined,
    // we require that outset buffer around 'desiredOutput' from the color map.
    skif::LayerSpace<SkIRect> requiredInput = this->outsetByMaxDisplacement(mapping, desiredOutput);
    requiredInput = this->getChildInputLayerBounds(kColor, mapping, requiredInput, contentBounds);

    // Accumulate the required input for the displacement filter to cover the original desired out
    requiredInput.join(this->getChildInputLayerBounds(
            kDisplacement, mapping, desiredOutput, contentBounds));
    return requiredInput;
}

skif::LayerSpace<SkIRect> SkDisplacementMapImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    skif::LayerSpace<SkIRect> colorOutput =
            this->getChildOutputLayerBounds(kColor, mapping, contentBounds);
    return this->outsetByMaxDisplacement(mapping, colorOutput);
}

SkRect SkDisplacementMapImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect colorBounds = this->getInput(kColor) ? this->getInput(kColor)->computeFastBounds(src)
                                                : src;
    float maxDisplacement = 0.5f * SkScalarAbs(fScale);
    return colorBounds.makeOutset(maxDisplacement, maxDisplacement);
}

#else

// The displacement map effect requires SkSL, just return the color input, possibly cropped
sk_sp<SkImageFilter> SkImageFilters::DisplacementMap(
        SkColorChannel xChannelSelector, SkColorChannel yChannelSelector, SkScalar scale,
        sk_sp<SkImageFilter> displacement, sk_sp<SkImageFilter> color, const CropRect& cropRect) {
    return cropRect ? SkMakeCropImageFilter(*cropRect, std::move(color)) : color;
}

void SkRegisterDisplacementMapImageFilterFlattenable() {}

#endif

#endif // SK_USE_LEGACY_DISPLACEMENT_MAP_IMAGEFILTER
