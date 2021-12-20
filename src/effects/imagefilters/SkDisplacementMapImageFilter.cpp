/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/SkColorData.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/SurfaceFillContext.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
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

#if SK_SUPPORT_GPU

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
    Context displContext(ctx.mapping(), ctx.desiredOutput(), ctx.cache(),
                         kN32_SkColorType, nullptr, ctx.source());
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

#if SK_SUPPORT_GPU
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
                                                   sfc->colorInfo().colorType(),
                                                   sfc->colorInfo().refColorSpace(),
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

#if SK_SUPPORT_GPU
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

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrDisplacementMapEffect);

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
                                                 kHalf2_GrSLType, "Scale");
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
