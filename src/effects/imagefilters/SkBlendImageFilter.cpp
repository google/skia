/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#if defined(SK_USE_LEGACY_BLEND_IMAGEFILTER)

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "src/base/SkVx.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

#if defined(SK_GANESH)
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrFragmentProcessors.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#endif

namespace {

class SkBlendImageFilter : public SkImageFilter_Base {
public:
    SkBlendImageFilter(sk_sp<SkBlender> blender, sk_sp<SkImageFilter> inputs[2],
                       const SkRect* cropRect)
          : INHERITED(inputs, 2, cropRect)
          , fBlender(std::move(blender))
    {
        SkASSERT(fBlender);
    }

protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

    SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;

#if defined(SK_GANESH)
    sk_sp<SkSpecialImage> filterImageGPU(const Context& ctx,
                                         sk_sp<SkSpecialImage> background,
                                         const SkIPoint& backgroundOffset,
                                         sk_sp<SkSpecialImage> foreground,
                                         const SkIPoint& foregroundOffset,
                                         const SkIRect& bounds) const;
#endif

    void flatten(SkWriteBuffer&) const override;

    void drawForeground(SkCanvas* canvas, SkSpecialImage*, const SkIRect&) const;

private:
    friend void ::SkRegisterBlendImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkBlendImageFilter)

    sk_sp<SkBlender> fBlender;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Blend(SkBlendMode mode,
                                           sk_sp<SkImageFilter> background,
                                           sk_sp<SkImageFilter> foreground,
                                           const CropRect& cropRect) {
    sk_sp<SkImageFilter> inputs[2] = { std::move(background), std::move(foreground) };
    return sk_sp<SkImageFilter>(new SkBlendImageFilter(SkBlender::Mode(mode), inputs, cropRect));
}

sk_sp<SkImageFilter> SkImageFilters::Blend(sk_sp<SkBlender> blender,
                                           sk_sp<SkImageFilter> background,
                                           sk_sp<SkImageFilter> foreground,
                                           const CropRect& cropRect) {
    if (!blender) {
        blender = SkBlender::Mode(SkBlendMode::kSrcOver);
    }
    sk_sp<SkImageFilter> inputs[2] = { std::move(background), std::move(foreground) };
    return sk_sp<SkImageFilter>(new SkBlendImageFilter(blender, inputs, cropRect));
}

sk_sp<SkFlattenable> SkBlendImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);

    sk_sp<SkBlender> blender;
    const uint32_t mode = buffer.read32();
    if (mode == kCustom_SkBlendMode) {
        blender = buffer.readBlender();
    } else {
        if (mode > (unsigned)SkBlendMode::kLastMode) {
            buffer.validate(false);
            return nullptr;
        }
        blender = SkBlender::Mode((SkBlendMode)mode);
    }
    return SkImageFilters::Blend(std::move(blender), common.getInput(0), common.getInput(1),
                                 common.cropRect());
}

void SkBlendImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    if (auto bm = as_BB(fBlender)->asBlendMode()) {
        buffer.write32((unsigned)bm.value());
    } else {
        buffer.write32(kCustom_SkBlendMode);
        buffer.writeFlattenable(fBlender.get());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSpecialImage> SkBlendImageFilter::onFilterImage(const Context& ctx,
                                                        SkIPoint* offset) const {
    SkIPoint backgroundOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> background(this->filterInput(0, ctx, &backgroundOffset));

    SkIPoint foregroundOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> foreground(this->filterInput(1, ctx, &foregroundOffset));

    SkIRect foregroundBounds = SkIRect::MakeEmpty();
    if (foreground) {
        foregroundBounds = SkIRect::MakeXYWH(foregroundOffset.x(), foregroundOffset.y(),
                                             foreground->width(), foreground->height());
    }

    SkIRect srcBounds = SkIRect::MakeEmpty();
    if (background) {
        srcBounds = SkIRect::MakeXYWH(backgroundOffset.x(), backgroundOffset.y(),
                                      background->width(), background->height());
    }

    srcBounds.join(foregroundBounds);
    if (srcBounds.isEmpty()) {
        return nullptr;
    }

    SkIRect bounds;
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return nullptr;
    }

    offset->fX = bounds.left();
    offset->fY = bounds.top();

#if defined(SK_GANESH)
    if (ctx.gpuBacked()) {
        return this->filterImageGPU(ctx, background, backgroundOffset,
                                    foreground, foregroundOffset, bounds);
    }
#endif

    sk_sp<SkSpecialSurface> surf(ctx.makeSurface(bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0); // can't count on background to fully clear the background
    canvas->translate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));

    if (background) {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        background->draw(canvas,
                         SkIntToScalar(backgroundOffset.fX), SkIntToScalar(backgroundOffset.fY),
                         SkSamplingOptions(), &paint);
    }

    this->drawForeground(canvas, foreground.get(), foregroundBounds);

    return surf->makeImageSnapshot();
}

SkIRect SkBlendImageFilter::onFilterBounds(const SkIRect& src,
                                           const SkMatrix& ctm,
                                           MapDirection dir,
                                           const SkIRect* inputRect) const {
    if (kReverse_MapDirection == dir) {
        return INHERITED::onFilterBounds(src, ctm, dir, inputRect);
    }

    SkASSERT(!inputRect);
    SkASSERT(2 == this->countInputs());
    auto getBackground = [&]() {
        return this->getInput(0) ? this->getInput(0)->filterBounds(src, ctm, dir, inputRect) : src;
    };
    auto getForeground = [&]() {
        return this->getInput(1) ? this->getInput(1)->filterBounds(src, ctm, dir, inputRect) : src;
    };
    if (auto bm = as_BB(fBlender)->asBlendMode()) {
        switch (bm.value()) {
            case SkBlendMode::kClear:
                return SkIRect::MakeEmpty();

            case SkBlendMode::kSrc:
            case SkBlendMode::kDstATop:
                return getForeground();

            case SkBlendMode::kDst:
            case SkBlendMode::kSrcATop:
                return getBackground();

            case SkBlendMode::kSrcIn:
            case SkBlendMode::kDstIn: {
                auto result = getBackground();
                if (!result.intersect(getForeground())) {
                    return SkIRect::MakeEmpty();
                }
                return result;
            }
            default: break;
        }
    }
    auto result = getBackground();
    result.join(getForeground());
    return result;
}

void SkBlendImageFilter::drawForeground(SkCanvas* canvas, SkSpecialImage* img,
                                        const SkIRect& fgBounds) const {
    SkPaint paint;
    paint.setBlender(fBlender);
    if (img) {
        img->draw(canvas, SkIntToScalar(fgBounds.fLeft), SkIntToScalar(fgBounds.fTop),
                  SkSamplingOptions(), &paint);
    }

    SkAutoCanvasRestore acr(canvas, true);
    canvas->clipRect(SkRect::Make(fgBounds), SkClipOp::kDifference);
    paint.setColor(0);
    canvas->drawPaint(paint);
}

#if defined(SK_GANESH)

sk_sp<SkSpecialImage> SkBlendImageFilter::filterImageGPU(const Context& ctx,
                                                         sk_sp<SkSpecialImage> background,
                                                         const SkIPoint& backgroundOffset,
                                                         sk_sp<SkSpecialImage> foreground,
                                                         const SkIPoint& foregroundOffset,
                                                         const SkIRect& bounds) const {
    SkASSERT(ctx.gpuBacked());

    auto rContext = ctx.getContext();

    GrSurfaceProxyView backgroundView, foregroundView;

    if (background) {
        backgroundView = background->view(rContext);
    }

    if (foreground) {
        foregroundView = foreground->view(rContext);
    }

    std::unique_ptr<GrFragmentProcessor> fp;
    const auto& caps = *ctx.getContext()->priv().caps();
    GrSamplerState sampler(GrSamplerState::WrapMode::kClampToBorder,
                           GrSamplerState::Filter::kNearest);

    if (backgroundView.asTextureProxy()) {
        SkRect bgSubset = SkRect::Make(background->subset());
        SkMatrix bgMatrix = SkMatrix::Translate(
                SkIntToScalar(bgSubset.left() - backgroundOffset.fX),
                SkIntToScalar(bgSubset.top()  - backgroundOffset.fY));
        fp = GrTextureEffect::MakeSubset(std::move(backgroundView), background->alphaType(),
                                         bgMatrix, sampler, bgSubset, caps);
        fp = GrColorSpaceXformEffect::Make(std::move(fp), background->getColorSpace(),
                                           background->alphaType(), ctx.colorSpace(),
                                           kPremul_SkAlphaType);
    } else {
        fp = GrFragmentProcessor::MakeColor(SK_PMColor4fTRANSPARENT);
    }

    GrImageInfo info(ctx.grColorType(), kPremul_SkAlphaType, ctx.refColorSpace(), bounds.size());

    if (foregroundView.asTextureProxy()) {
        SkRect fgSubset = SkRect::Make(foreground->subset());
        SkMatrix fgMatrix = SkMatrix::Translate(
                SkIntToScalar(fgSubset.left() - foregroundOffset.fX),
                SkIntToScalar(fgSubset.top()  - foregroundOffset.fY));
        auto fgFP = GrTextureEffect::MakeSubset(std::move(foregroundView), foreground->alphaType(),
                                                fgMatrix, sampler, fgSubset, caps);
        fgFP = GrColorSpaceXformEffect::Make(std::move(fgFP), foreground->getColorSpace(),
                                             foreground->alphaType(), ctx.colorSpace(),
                                             kPremul_SkAlphaType);

        SkSurfaceProps props{}; // default OK; blend-image filters don't render text
        GrFPArgs args(rContext, &info.colorInfo(), props);

        fp = GrFragmentProcessors::Make(as_BB(fBlender), std::move(fgFP), std::move(fp), args);
    }

    auto sfc = rContext->priv().makeSFC(
            info, "BlendImageFilter_FilterImageGPU", SkBackingFit::kApprox);
    if (!sfc) {
        return nullptr;
    }

    sfc->fillRectToRectWithFP(bounds, SkIRect::MakeSize(bounds.size()), std::move(fp));

    return SkSpecialImage::MakeDeferredFromGpu(rContext,
                                               SkIRect::MakeWH(bounds.width(), bounds.height()),
                                               kNeedNewImageUniqueID_SpecialImage,
                                               sfc->readSurfaceView(),
                                               sfc->colorInfo(),
                                               ctx.surfaceProps());
}

#endif

namespace {

class SkArithmeticImageFilter final : public SkImageFilter_Base {
public:
    SkArithmeticImageFilter(float k1, float k2, float k3, float k4, bool enforcePMColor,
                            sk_sp<SkImageFilter> inputs[2], const SkRect* cropRect)
            : INHERITED(inputs, 2, cropRect)
            , fK{k1, k2, k3, k4}
            , fEnforcePMColor(enforcePMColor) {}

protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

    SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;

#if defined(SK_GANESH)
    sk_sp<SkSpecialImage> filterImageGPU(const Context& ctx,
                                         sk_sp<SkSpecialImage> background,
                                         const SkIPoint& backgroundOffset,
                                         sk_sp<SkSpecialImage> foreground,
                                         const SkIPoint& foregroundOffset,
                                         const SkIRect& bounds) const;
#endif

    void flatten(SkWriteBuffer& buffer) const override;

    void drawForeground(SkCanvas* canvas, SkSpecialImage*, const SkIRect&) const;

private:
    friend void ::SkRegisterBlendImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkArithmeticImageFilter)

    bool onAffectsTransparentBlack() const override { return !SkScalarNearlyZero(fK[3]); }

    SkV4 fK;
    bool fEnforcePMColor;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Arithmetic(
        SkScalar k1, SkScalar k2, SkScalar k3, SkScalar k4, bool enforcePMColor,
        sk_sp<SkImageFilter> background, sk_sp<SkImageFilter> foreground,
        const CropRect& cropRect) {
    if (!SkScalarIsFinite(k1) || !SkScalarIsFinite(k2) || !SkScalarIsFinite(k3) ||
        !SkScalarIsFinite(k4)) {
        return nullptr;
    }

    // are we nearly some other "std" mode?
    int mode = -1;  // illegal mode
    if (SkScalarNearlyZero(k1) && SkScalarNearlyEqual(k2, SK_Scalar1) && SkScalarNearlyZero(k3) &&
        SkScalarNearlyZero(k4)) {
        mode = (int)SkBlendMode::kSrc;
    } else if (SkScalarNearlyZero(k1) && SkScalarNearlyZero(k2) &&
               SkScalarNearlyEqual(k3, SK_Scalar1) && SkScalarNearlyZero(k4)) {
        mode = (int)SkBlendMode::kDst;
    } else if (SkScalarNearlyZero(k1) && SkScalarNearlyZero(k2) && SkScalarNearlyZero(k3) &&
               SkScalarNearlyZero(k4)) {
        mode = (int)SkBlendMode::kClear;
    }
    if (mode >= 0) {
        return SkImageFilters::Blend((SkBlendMode)mode, std::move(background),
                                     std::move(foreground), cropRect);
    }

    sk_sp<SkImageFilter> inputs[2] = {std::move(background), std::move(foreground)};
    return sk_sp<SkImageFilter>(
            new SkArithmeticImageFilter(k1, k2, k3, k4, enforcePMColor, inputs, cropRect));
}

sk_sp<SkFlattenable> SkArithmeticImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    float k[4];
    for (int i = 0; i < 4; ++i) {
        k[i] = buffer.readScalar();
    }
    const bool enforcePMColor = buffer.readBool();
    if (!buffer.isValid()) {
        return nullptr;
    }
    return SkImageFilters::Arithmetic(k[0], k[1], k[2], k[3], enforcePMColor, common.getInput(0),
                                      common.getInput(1), common.cropRect());
}

void SkArithmeticImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    for (int i = 0; i < 4; ++i) {
        buffer.writeScalar(fK[i]);
    }
    buffer.writeBool(fEnforcePMColor);
}

void SkRegisterBlendImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkBlendImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkXfermodeImageFilter_Base", SkBlendImageFilter::CreateProc);
    SkFlattenable::Register("SkXfermodeImageFilterImpl", SkBlendImageFilter::CreateProc);

    // SkRegisterArithmeticImageFilterFlattenable
    SK_REGISTER_FLATTENABLE(SkArithmeticImageFilter);
    SkFlattenable::Register("ArithmeticImageFilterImpl", SkArithmeticImageFilter::CreateProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <bool EnforcePMColor>
void arith_span(const SkV4& k, SkPMColor dst[], const SkPMColor src[], int count) {
    const skvx::float4 k1 = k[0] * (1/255.0f),
                       k2 = k[1],
                       k3 = k[2],
                       k4 = k[3] * 255.0f + 0.5f;

    for (int i = 0; i < count; i++) {
        skvx::float4 s = skvx::cast<float>(skvx::byte4::Load(src+i)),
                     d = skvx::cast<float>(skvx::byte4::Load(dst+i)),
                     r = pin(k1*s*d + k2*s + k3*d + k4, skvx::float4(0.f), skvx::float4(255.f));
        if (EnforcePMColor) {
            auto a = skvx::shuffle<3,3,3,3>(r);
            r = min(a, r);
        }
        skvx::cast<uint8_t>(r).store(dst+i);
    }
}

// apply mode to src==transparent (0)
template<bool EnforcePMColor> void arith_transparent(const SkV4& k, SkPMColor dst[], int count) {
    const skvx::float4 k3 = k[2],
                       k4 = k[3] * 255.0f + 0.5f;

    for (int i = 0; i < count; i++) {
        skvx::float4 d = skvx::cast<float>(skvx::byte4::Load(dst+i)),
                     r = pin(k3*d + k4, skvx::float4(0.f), skvx::float4(255.f));
        if (EnforcePMColor) {
            auto a = skvx::shuffle<3,3,3,3>(r);
            r = min(a, r);
        }
        skvx::cast<uint8_t>(r).store(dst+i);
    }
}

static bool intersect(SkPixmap* dst, SkPixmap* src, int srcDx, int srcDy) {
    SkIRect dstR = SkIRect::MakeWH(dst->width(), dst->height());
    SkIRect srcR = SkIRect::MakeXYWH(srcDx, srcDy, src->width(), src->height());
    SkIRect sect;
    if (!sect.intersect(dstR, srcR)) {
        return false;
    }
    *dst = SkPixmap(dst->info().makeDimensions(sect.size()),
                    dst->addr(sect.fLeft, sect.fTop),
                    dst->rowBytes());
    *src = SkPixmap(src->info().makeDimensions(sect.size()),
                    src->addr(std::max(0, -srcDx), std::max(0, -srcDy)),
                    src->rowBytes());
    return true;
}

sk_sp<SkSpecialImage> SkArithmeticImageFilter::onFilterImage(const Context& ctx,
                                                             SkIPoint* offset) const {
    SkIPoint backgroundOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> background(this->filterInput(0, ctx, &backgroundOffset));

    SkIPoint foregroundOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> foreground(this->filterInput(1, ctx, &foregroundOffset));

    SkIRect foregroundBounds = SkIRect::MakeEmpty();
    if (foreground) {
        foregroundBounds = SkIRect::MakeXYWH(foregroundOffset.x(), foregroundOffset.y(),
                                             foreground->width(), foreground->height());
    }

    SkIRect srcBounds = SkIRect::MakeEmpty();
    if (background) {
        srcBounds = SkIRect::MakeXYWH(backgroundOffset.x(), backgroundOffset.y(),
                                      background->width(), background->height());
    }

    srcBounds.join(foregroundBounds);
    if (srcBounds.isEmpty()) {
        return nullptr;
    }

    SkIRect bounds;
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return nullptr;
    }

    offset->fX = bounds.left();
    offset->fY = bounds.top();

#if defined(SK_GANESH)
    if (ctx.gpuBacked()) {
        return this->filterImageGPU(ctx, background, backgroundOffset, foreground,
                                    foregroundOffset, bounds);
    }
#endif

    sk_sp<SkSpecialSurface> surf(ctx.makeSurface(bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);  // can't count on background to fully clear the background
    canvas->translate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));

    if (background) {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        background->draw(canvas, SkIntToScalar(backgroundOffset.fX),
                         SkIntToScalar(backgroundOffset.fY), SkSamplingOptions(), &paint);
    }

    this->drawForeground(canvas, foreground.get(), foregroundBounds);

    return surf->makeImageSnapshot();
}

SkIRect SkArithmeticImageFilter::onFilterBounds(const SkIRect& src,
                                                const SkMatrix& ctm,
                                                MapDirection dir,
                                                const SkIRect* inputRect) const {
    if (kReverse_MapDirection == dir) {
        return INHERITED::onFilterBounds(src, ctm, dir, inputRect);
    }

    SkASSERT(2 == this->countInputs());

    // result(i1,i2) = k1*i1*i2 + k2*i1 + k3*i2 + k4
    // Note that background (getInput(0)) is i2, and foreground (getInput(1)) is i1.
    auto i2 = this->getInput(0) ? this->getInput(0)->filterBounds(src, ctm, dir, nullptr) : src;
    auto i1 = this->getInput(1) ? this->getInput(1)->filterBounds(src, ctm, dir, nullptr) : src;

    // Arithmetic with non-zero k4 may influence the complete filter primitive
    // region. [k4 > 0 => result(0,0) = k4 => result(i1,i2) >= k4]
    if (!SkScalarNearlyZero(fK[3])) {
        i1.join(i2);
        return i1;
    }

    // If both K2 or K3 are non-zero, both i1 and i2 appear.
    if (!SkScalarNearlyZero(fK[1]) && !SkScalarNearlyZero(fK[2])) {
        i1.join(i2);
        return i1;
    }

    // If k2 is non-zero, output can be produced whenever i1 is non-transparent.
    // [k3 = k4 = 0 => result(i1,i2) = k1*i1*i2 + k2*i1 = (k1*i2 + k2)*i1]
    if (!SkScalarNearlyZero(fK[1])) {
        return i1;
    }

    // If k3 is non-zero, output can be produced whenever i2 is non-transparent.
    // [k2 = k4 = 0 => result(i1,i2) = k1*i1*i2 + k3*i2 = (k1*i1 + k3)*i2]
    if (!SkScalarNearlyZero(fK[2])) {
        return i2;
    }

    // If just k1 is non-zero, output will only be produce where both inputs
    // are non-transparent. Use intersection.
    // [k1 > 0 and k2 = k3 = k4 = 0 => result(i1,i2) = k1*i1*i2]
    if (!SkScalarNearlyZero(fK[0])) {
        if (!i1.intersect(i2)) {
            return SkIRect::MakeEmpty();
        }
        return i1;
    }

    // [k1 = k2 = k3 = k4 = 0 => result(i1,i2) = 0]
    return SkIRect::MakeEmpty();
}

#if defined(SK_GANESH)

std::unique_ptr<GrFragmentProcessor> make_arithmetic_fp(
        std::unique_ptr<GrFragmentProcessor> srcFP,
        std::unique_ptr<GrFragmentProcessor> dstFP,
        const SkV4& k,
        bool enforcePMColor) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader srcFP;"
        "uniform shader dstFP;"
        "uniform half4 k;"
        "uniform half pmClamp;"
        "half4 main(float2 xy) {"
            "half4 src = srcFP.eval(xy);"
            "half4 dst = dstFP.eval(xy);"
            "half4 color = saturate(k.x * src * dst +"
                                   "k.y * src +"
                                   "k.z * dst +"
                                   "k.w);"
            "color.rgb = min(color.rgb, max(color.a, pmClamp));"
            "return color;"
        "}"
    );
    return GrSkSLFP::Make(effect, "arithmetic_fp", /*inputFP=*/nullptr, GrSkSLFP::OptFlags::kNone,
                          "srcFP", std::move(srcFP),
                          "dstFP", std::move(dstFP),
                          "k", k,
                          "pmClamp", enforcePMColor ? 0.0f : 1.0f);
}

sk_sp<SkSpecialImage> SkArithmeticImageFilter::filterImageGPU(
        const Context& ctx,
        sk_sp<SkSpecialImage> background,
        const SkIPoint& backgroundOffset,
        sk_sp<SkSpecialImage> foreground,
        const SkIPoint& foregroundOffset,
        const SkIRect& bounds) const {
    SkASSERT(ctx.gpuBacked());

    auto rContext = ctx.getContext();

    GrSurfaceProxyView backgroundView, foregroundView;

    GrProtected isProtected = GrProtected::kNo;
    if (background) {
        backgroundView = background->view(rContext);
        SkASSERT(backgroundView.proxy());
        isProtected = backgroundView.proxy()->isProtected();
    }

    if (foreground) {
        foregroundView = foreground->view(rContext);
        SkASSERT(foregroundView.proxy());
        isProtected = foregroundView.proxy()->isProtected();
    }

    std::unique_ptr<GrFragmentProcessor> fp;
    const auto& caps = *ctx.getContext()->priv().caps();
    GrSamplerState sampler(GrSamplerState::WrapMode::kClampToBorder,
                           GrSamplerState::Filter::kNearest);

    if (background) {
        SkRect bgSubset = SkRect::Make(background->subset());
        SkMatrix backgroundMatrix = SkMatrix::Translate(
                SkIntToScalar(bgSubset.left() - backgroundOffset.fX),
                SkIntToScalar(bgSubset.top()  - backgroundOffset.fY));
        fp = GrTextureEffect::MakeSubset(std::move(backgroundView),
                                         background->alphaType(),
                                         backgroundMatrix,
                                         sampler,
                                         bgSubset,
                                         caps);
        fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                           background->getColorSpace(),
                                           background->alphaType(),
                                           ctx.colorSpace(),
                                           kPremul_SkAlphaType);
    } else {
        fp = GrFragmentProcessor::MakeColor(SK_PMColor4fTRANSPARENT);
    }

    if (foreground) {
        SkRect fgSubset = SkRect::Make(foreground->subset());
        SkMatrix foregroundMatrix = SkMatrix::Translate(
                SkIntToScalar(fgSubset.left() - foregroundOffset.fX),
                SkIntToScalar(fgSubset.top()  - foregroundOffset.fY));
        auto fgFP = GrTextureEffect::MakeSubset(std::move(foregroundView),
                                                foreground->alphaType(),
                                                foregroundMatrix,
                                                sampler,
                                                fgSubset,
                                                caps);
        fgFP = GrColorSpaceXformEffect::Make(std::move(fgFP),
                                             foreground->getColorSpace(),
                                             foreground->alphaType(),
                                             ctx.colorSpace(),
                                             kPremul_SkAlphaType);
        fp = make_arithmetic_fp(std::move(fgFP), std::move(fp), fK, fEnforcePMColor);
    }

    GrImageInfo info(ctx.grColorType(), kPremul_SkAlphaType, ctx.refColorSpace(), bounds.size());
    auto sfc = rContext->priv().makeSFC(info,
                                        "ArithmeticImageFilter_FilterImageGPU",
                                        SkBackingFit::kApprox,
                                        1,
                                        GrMipmapped::kNo,
                                        isProtected,
                                        kBottomLeft_GrSurfaceOrigin);
    if (!sfc) {
        return nullptr;
    }

    sfc->fillRectToRectWithFP(bounds, SkIRect::MakeSize(bounds.size()), std::move(fp));

    return SkSpecialImage::MakeDeferredFromGpu(rContext,
                                               SkIRect::MakeWH(bounds.width(), bounds.height()),
                                               kNeedNewImageUniqueID_SpecialImage,
                                               sfc->readSurfaceView(),
                                               sfc->colorInfo(),
                                               ctx.surfaceProps());
}
#endif

void SkArithmeticImageFilter::drawForeground(SkCanvas* canvas, SkSpecialImage* img,
                                             const SkIRect& fgBounds) const {
    SkPixmap dst;
    if (!canvas->peekPixels(&dst)) {
        return;
    }

    const SkMatrix& ctm = canvas->getTotalMatrix();
    SkASSERT(ctm.getType() <= SkMatrix::kTranslate_Mask);
    const int dx = SkScalarRoundToInt(ctm.getTranslateX());
    const int dy = SkScalarRoundToInt(ctm.getTranslateY());
    // be sure to perform this offset using SkIRect, since it saturates to avoid overflows
    const SkIRect fgoffset = fgBounds.makeOffset(dx, dy);

    if (img) {
        SkBitmap srcBM;
        SkPixmap src;
        if (!img->getROPixels(&srcBM)) {
            return;
        }
        if (!srcBM.peekPixels(&src)) {
            return;
        }

        auto proc = fEnforcePMColor ? arith_span<true> : arith_span<false>;
        SkPixmap tmpDst = dst;
        if (intersect(&tmpDst, &src, fgoffset.fLeft, fgoffset.fTop)) {
            for (int y = 0; y < tmpDst.height(); ++y) {
                proc(fK, tmpDst.writable_addr32(0, y), src.addr32(0, y), tmpDst.width());
            }
        }
    }

    // Now apply the mode with transparent-color to the outside of the fg image
    SkRegion outside(SkIRect::MakeWH(dst.width(), dst.height()));
    outside.op(fgoffset, SkRegion::kDifference_Op);
    auto proc = fEnforcePMColor ? arith_transparent<true> : arith_transparent<false>;
    for (SkRegion::Iterator iter(outside); !iter.done(); iter.next()) {
        const SkIRect r = iter.rect();
        for (int y = r.fTop; y < r.fBottom; ++y) {
            proc(fK, dst.writable_addr32(r.fLeft, y), r.width());
        }
    }
}

#else

#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkBlenders.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/imagefilters/SkCropImageFilter.h"

#include <cstdint>
#include <optional>
#include <utility>

namespace {

class SkBlendImageFilter : public SkImageFilter_Base {
    // Input image filter indices
    static constexpr int kBackground = 0;
    static constexpr int kForeground = 1;

public:
    SkBlendImageFilter(sk_sp<SkBlender> blender,
                       const std::optional<SkV4>& coefficients,
                       bool enforcePremul,
                       sk_sp<SkImageFilter> inputs[2])
            : SkImageFilter_Base(inputs, 2, nullptr)
            , fBlender(std::move(blender))
            , fArithmeticCoefficients(coefficients)
            , fEnforcePremul(enforcePremul) {
        // A null blender represents src-over, which should have been filled in by the factory
        SkASSERT(fBlender);
    }

    SkRect computeFastBounds(const SkRect& bounds) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    static constexpr uint32_t kArithmetic_SkBlendMode = kCustom_SkBlendMode + 1;

    friend void ::SkRegisterBlendImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkBlendImageFilter)
    static sk_sp<SkFlattenable> LegacyArithmeticCreateProc(SkReadBuffer& buffer);

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

    bool onAffectsTransparentBlack() const override {
        // An arbitrary runtime blender or an arithmetic runtime blender with k3 != 0 affects
        // transparent black.
        return !as_BB(fBlender)->asBlendMode().has_value() &&
               (!fArithmeticCoefficients.has_value() || (*fArithmeticCoefficients)[3] != 0.f);
    }

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    sk_sp<SkShader> makeBlendShader(sk_sp<SkShader> bg, sk_sp<SkShader> fg) const;

    sk_sp<SkBlender> fBlender;

    // Normally runtime SkBlenders are pessimistic about the bounds they affect. For Arithmetic,
    // we remember the coefficients so that bounds can be reasoned about.
    std::optional<SkV4> fArithmeticCoefficients;
    bool fEnforcePremul; // Remembered to serialize the Arithmetic variant correctly
};

sk_sp<SkImageFilter> make_blend(sk_sp<SkBlender> blender,
                                sk_sp<SkImageFilter> background,
                                sk_sp<SkImageFilter> foreground,
                                const SkImageFilters::CropRect& cropRect,
                                std::optional<SkV4> coefficients = {},
                                bool enforcePremul = false) {
    if (!blender) {
        blender = SkBlender::Mode(SkBlendMode::kSrcOver);
    }

    auto cropped = [cropRect](sk_sp<SkImageFilter> filter) {
        if (cropRect) {
            filter = SkMakeCropImageFilter(*cropRect, std::move(filter));
        }
        return filter;
    };

    if (auto bm = as_BB(blender)->asBlendMode()) {
        if (bm == SkBlendMode::kSrc) {
            return cropped(std::move(foreground));
        } else if (bm == SkBlendMode::kDst) {
            return cropped(std::move(background));
        }
        // TODO(b/283548627): Route kClear to a dedicated Empty image filter.
    }

    sk_sp<SkImageFilter> inputs[2] = { std::move(background), std::move(foreground) };
    sk_sp<SkImageFilter> filter{new SkBlendImageFilter(blender, coefficients,
                                                       enforcePremul, inputs)};
    return cropped(std::move(filter));
}

} // anonymous namespace

sk_sp<SkImageFilter> SkImageFilters::Blend(SkBlendMode mode,
                                           sk_sp<SkImageFilter> background,
                                           sk_sp<SkImageFilter> foreground,
                                           const CropRect& cropRect) {
    return make_blend(SkBlender::Mode(mode),
                      std::move(background),
                      std::move(foreground),
                      cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Blend(sk_sp<SkBlender> blender,
                                           sk_sp<SkImageFilter> background,
                                           sk_sp<SkImageFilter> foreground,
                                           const CropRect& cropRect) {
    return make_blend(std::move(blender), std::move(background), std::move(foreground), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Arithmetic(SkScalar k1,
                                                SkScalar k2,
                                                SkScalar k3,
                                                SkScalar k4,
                                                bool enforcePMColor,
                                                sk_sp<SkImageFilter> background,
                                                sk_sp<SkImageFilter> foreground,
                                                const CropRect& cropRect) {
    auto blender = SkBlenders::Arithmetic(k1, k2, k3, k4, enforcePMColor);
    if (!blender) {
        // Arithmetic() returns null on an error, not to optimize src-over
        return nullptr;
    }
    return make_blend(std::move(blender),
                      std::move(background),
                      std::move(foreground),
                      cropRect,
                      // Carry arithmetic coefficients and premul behavior into image filter for
                      // serialization and bounds analysis
                      SkV4{k1, k2, k3, k4},
                      enforcePMColor);
}

void SkRegisterBlendImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkBlendImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkXfermodeImageFilter_Base", SkBlendImageFilter::CreateProc);
    SkFlattenable::Register("SkXfermodeImageFilterImpl", SkBlendImageFilter::CreateProc);
    SkFlattenable::Register("ArithmeticImageFilterImpl",
                            SkBlendImageFilter::LegacyArithmeticCreateProc);
    SkFlattenable::Register("SkArithmeticImageFilter",
                            SkBlendImageFilter::LegacyArithmeticCreateProc);
}

sk_sp<SkFlattenable> SkBlendImageFilter::LegacyArithmeticCreateProc(SkReadBuffer& buffer) {
    // Newer SKPs should be using the updated Blend CreateProc.
    if (!buffer.validate(buffer.isVersionLT(SkPicturePriv::kCombineBlendArithmeticFilters))) {
        SkASSERT(false); // debug-only, so release will just see a failed deserialization
        return nullptr;
    }

    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    float k[4];
    for (int i = 0; i < 4; ++i) {
        k[i] = buffer.readScalar();
    }
    const bool enforcePremul = buffer.readBool();
    return SkImageFilters::Arithmetic(k[0], k[1], k[2], k[3], enforcePremul,
                                      common.getInput(0), common.getInput(1), common.cropRect());
}

sk_sp<SkFlattenable> SkBlendImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);

    sk_sp<SkBlender> blender;
    std::optional<SkV4> coefficients;
    bool enforcePremul = false;

    const uint32_t mode = buffer.read32();
    if (mode == kArithmetic_SkBlendMode) {
        // Should only see this sentinel value in newer SKPs
        if (buffer.validate(!buffer.isVersionLT(SkPicturePriv::kCombineBlendArithmeticFilters))) {
            SkV4 k;
            for (int i = 0; i < 4; ++i) {
                k[i] = buffer.readScalar();
            }
            coefficients = k;
            enforcePremul = buffer.readBool();
            blender = SkBlenders::Arithmetic(k.x, k.y, k.z, k.w, enforcePremul);
            if (!buffer.validate(SkToBool(blender))) {
                return nullptr; // A null arithmetic blender is an error condition
            }
        }
    } else if (mode == kCustom_SkBlendMode) {
        blender = buffer.readBlender();
    } else {
        if (!buffer.validate(mode <= (unsigned) SkBlendMode::kLastMode)) {
            return nullptr;
        }
        blender = SkBlender::Mode((SkBlendMode)mode);
    }

    return make_blend(std::move(blender),
                      common.getInput(kBackground),
                      common.getInput(kForeground),
                      common.cropRect(),
                      coefficients,
                      enforcePremul);
}

void SkBlendImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    if (fArithmeticCoefficients.has_value()) {
        buffer.write32(kArithmetic_SkBlendMode);

        const SkV4& k = *fArithmeticCoefficients;
        buffer.writeScalar(k[0]);
        buffer.writeScalar(k[1]);
        buffer.writeScalar(k[2]);
        buffer.writeScalar(k[3]);
        buffer.writeBool(fEnforcePremul);
    } else if (auto bm = as_BB(fBlender)->asBlendMode()) {
        buffer.write32((unsigned)bm.value());
    } else {
        buffer.write32(kCustom_SkBlendMode);
        buffer.writeFlattenable(fBlender.get());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkBlendImageFilter::makeBlendShader(sk_sp<SkShader> bg, sk_sp<SkShader> fg) const {
    // A null input shader signifies transparent black when image filtering, but SkShaders::Blend
    // expects non-null shaders. So we have to do some clean up.
    if (!bg || !fg) {
        // If we don't affect transparent black and both inputs are null, then return a null
        // shader to skip any evaluation.
        if (!this->onAffectsTransparentBlack() && !bg && !fg) {
            return nullptr;
        }
        // Otherwise if only one input is null, we might be able to just return that one.
        if (auto bm = as_BB(fBlender)->asBlendMode()) {
            SkBlendModeCoeff src, dst;
            if (SkBlendMode_AsCoeff(*bm, &src, &dst)) {
                if (bg && (dst == SkBlendModeCoeff::kOne ||
                           dst == SkBlendModeCoeff::kISA ||
                           dst == SkBlendModeCoeff::kISC)) {
                    return bg;
                }
                if (fg && (src == SkBlendModeCoeff::kOne ||
                           src == SkBlendModeCoeff::kIDA)) {
                    return fg;
                }
            }
        }
        // If we made it this far, the blend has non-trivial behavior even when one of the
        // inputs is transparent black, so replace the null shaders with that color.
        if (!bg) { bg = SkShaders::Color(SK_ColorTRANSPARENT); }
        if (!fg) { fg = SkShaders::Color(SK_ColorTRANSPARENT); }
    }

    return SkShaders::Blend(fBlender, std::move(bg), std::move(fg));
}

skif::FilterResult SkBlendImageFilter::onFilterImage(const skif::Context& ctx) const {
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    // We could just request 'desiredOutput' for the blend's required input size, since that's what
    // it is expected to fill. However, some blend modes restrict the output to something other
    // than the union of the foreground and background. To make this restriction available to both
    // children before evaluating them, we determine the maximum possible output the blend can
    // produce from the contentBounds and require that for both children to produce.
    skif::LayerSpace<SkIRect> requiredInput = this->onGetOutputLayerBounds(
            ctx.mapping(), ctx.source().layerBounds());
    if (!requiredInput.intersect(ctx.desiredOutput())) {
        return {};
    }
    skif::Context inputCtx = ctx.withNewDesiredOutput(requiredInput);

    skif::FilterResult::Builder builder{ctx};
    builder.add(this->getChildOutput(kBackground, inputCtx));
    builder.add(this->getChildOutput(kForeground, inputCtx));
    return builder.eval(
            [&](SkSpan<sk_sp<SkShader>> inputs) -> sk_sp<SkShader> {
                return this->makeBlendShader(inputs[kBackground], inputs[kForeground]);
            }, ShaderFlags::kExplicitOutputBounds, requiredInput);
}

skif::LayerSpace<SkIRect> SkBlendImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // See comment in onFilterImage().
    skif::LayerSpace<SkIRect> requiredInput = this->onGetOutputLayerBounds(mapping, contentBounds);
    if (!requiredInput.intersect(desiredOutput)) {
        // Don't bother recursing if we know the blend will discard everything
        return skif::LayerSpace<SkIRect>::Empty();
    }

    // Return the union of both FG and BG required inputs to ensure both have all necessary pixels
    skif::LayerSpace<SkIRect> bgInput =
            this->getChildInputLayerBounds(kBackground, mapping, requiredInput, contentBounds);
    skif::LayerSpace<SkIRect> fgInput =
            this->getChildInputLayerBounds(kForeground, mapping, requiredInput, contentBounds);

    bgInput.join(fgInput);
    return bgInput;
}

skif::LayerSpace<SkIRect> SkBlendImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // Blending is (k0*FG*BG +       k1*FG +       k2*BG + k3) for arithmetic blenders OR
    //             ( 0*FG*BG + srcCoeff*FG + dstCoeff*BG + 0 ) for Porter-Duff blend modes OR
    //              un-inspectable(FG, BG) for advanced blend modes and other runtime blenders.
    //
    // There are six possible output bounds that can be produced:
    //   1. No output: K = (0,0,0,0) or (srcCoeff,dstCoeff) = (kZero,kZero)
    //   2. intersect(FG,BG): K = (non-zero, 0,0,0) or (srcCoeff,dstCoeff) = (kZero|kDA, kZero|kSA)
    //   3. FG-only: K = (0, non-zero, 0,0) or (srcCoeff,dstCoeff) = (!kZero&!kDA, kZero|kSA)
    //   4. BG-only: K = (0,0, non-zero, 0) or (srcCoeff,dstCoeff) = (kZero|kDA, !kZero&!kSA)
    //   5. union(FG,BG): K = (*,*,*,0) or (srcCoeff,dstCoeff) = (!kZero&!kDA, !kZero&!kSA)
    //        or an advanced blend mode.
    //   6. infinite: K = (*,*,*, non-zero) or a runtime blender other than SkBlenders::Arithmetic.
    bool transparentOutsideFG = false;
    bool transparentOutsideBG = false;
    if (auto bm = as_BB(fBlender)->asBlendMode()) {
        if (*bm == SkBlendMode::kClear) {
            return skif::LayerSpace<SkIRect>::Empty();
        }
        SkBlendModeCoeff src, dst;
        if (SkBlendMode_AsCoeff(*bm, &src, &dst)) {
            // If dst's coefficient is 0 then nothing can produce non-transparent content outside
            // of the foreground. When dst coefficient is SA, it will always be 0 outside the FG.
            // For purposes of transparency analysis, SC == SA.
            transparentOutsideFG = dst == SkBlendModeCoeff::kZero || dst == SkBlendModeCoeff::kSA
                                                                  || dst == SkBlendModeCoeff::kSC;
            // And the reverse is true for src and the background content.
            transparentOutsideBG = src == SkBlendModeCoeff::kZero || src == SkBlendModeCoeff::kDA;
        }
        // NOTE: advanced blends use src-over for their alpha channel, which should produce the
        // union of FG and BG. That is the outcome if we leave transparentOutsideFG/BG false.
    } else if (fArithmeticCoefficients.has_value()) {
        [[maybe_unused]] static constexpr SkV4 kClearCoeff = {0.f, 0.f, 0.f, 0.f};
        const SkV4& k = *fArithmeticCoefficients;
        SkASSERT(k != kClearCoeff); // Should have been converted to a clear blender

        if (k[3] != 0.f) {
            // The arithmetic equation produces non-transparent black everywhere
            return skif::LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
        } else {
            // Given the earlier assert and if, then (k[1] == k[2] == 0) implies k[0] != 0. If only
            // one of k[1] or k[2] are non-zero then, regardless of k[0], then only that bounds
            // has non-transparent content.
            transparentOutsideFG = k[2] == 0.f;
            transparentOutsideBG = k[1] == 0.f;
        }
    } else {
        // A non-arithmetic runtime blender, so pessimistically assume it can return non-transparent
        // black anywhere.
        return skif::LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
    }

    skif::LayerSpace<SkIRect> foregroundBounds =
            this->getChildOutputLayerBounds(kForeground, mapping, contentBounds);
    skif::LayerSpace<SkIRect> backgroundBounds =
            this->getChildOutputLayerBounds(kBackground, mapping, contentBounds);
    if (transparentOutsideFG) {
        if (transparentOutsideBG) {
            // Output is the intersection of both
            if (!foregroundBounds.intersect(backgroundBounds)) {
                return skif::LayerSpace<SkIRect>::Empty();
            }
        }
        return foregroundBounds;
    } else {
        if (!transparentOutsideBG) {
            // Output is the union of both (infinite bounds were detected earlier).
            backgroundBounds.join(foregroundBounds);
        }
        return backgroundBounds;
    }
}

SkRect SkBlendImageFilter::computeFastBounds(const SkRect& bounds) const {
    // TODO: This is a prime example of why computeFastBounds() and onGetOutputLayerBounds() should
    // be combined into the same function.
    bool transparentOutsideFG = false;
    bool transparentOutsideBG = false;
    if (auto bm = as_BB(fBlender)->asBlendMode()) {
        if (*bm == SkBlendMode::kClear) {
            return SkRect::MakeEmpty();
        }
        SkBlendModeCoeff src, dst;
        if (SkBlendMode_AsCoeff(*bm, &src, &dst)) {
            // If dst's coefficient is 0 then nothing can produce non-transparent content outside
            // of the foreground. When dst coefficient is SA, it will always be 0 outside the FG.
            transparentOutsideFG = dst == SkBlendModeCoeff::kZero || dst == SkBlendModeCoeff::kSA;
            // And the reverse is true for src and the background content.
            transparentOutsideBG = src == SkBlendModeCoeff::kZero || src == SkBlendModeCoeff::kDA;
        }
    } else if (fArithmeticCoefficients.has_value()) {
        [[maybe_unused]] static constexpr SkV4 kClearCoeff = {0.f, 0.f, 0.f, 0.f};
        const SkV4& k = *fArithmeticCoefficients;
        SkASSERT(k != kClearCoeff); // Should have been converted to a clear blender

        if (k[3] != 0.f) {
            // The arithmetic equation produces non-transparent black everywhere
            return SkRectPriv::MakeLargeS32();
        } else {
            // Given the earlier assert and if, then (k[1] == k[2] == 0) implies k[0] != 0. If only
            // one of k[1] or k[2] are non-zero then, regardless of k[0], then only that bounds
            // has non-transparent content.
            transparentOutsideFG = k[2] == 0.f;
            transparentOutsideBG = k[1] == 0.f;
        }
    } else {
        // A non-arithmetic runtime blender, so pessimistically assume it can return non-transparent
        // black anywhere.
        return SkRectPriv::MakeLargeS32();
    }

    SkRect foregroundBounds = this->getInput(kForeground) ?
            this->getInput(kForeground)->computeFastBounds(bounds) : bounds;
    SkRect backgroundBounds = this->getInput(kBackground) ?
            this->getInput(kBackground)->computeFastBounds(bounds) : bounds;
    if (transparentOutsideFG) {
        if (transparentOutsideBG) {
            // Output is the intersection of both
            if (!foregroundBounds.intersect(backgroundBounds)) {
                return SkRect::MakeEmpty();
            }
        }
        return foregroundBounds;
    } else {
        if (!transparentOutsideBG) {
            // Output is the union of both (infinite bounds were detected earlier).
            backgroundBounds.join(foregroundBounds);
        }
        return backgroundBounds;
    }
}

#endif // SK_USE_LEGACY_BLEND_IMAGEFILTER
