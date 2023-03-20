/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
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
#include "include/core/SkRegion.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "src/base/SkVx.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

#if defined(SK_GANESH)
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
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
    friend void ::SkRegisterArithmeticImageFilterFlattenable();
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

void SkRegisterArithmeticImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkArithmeticImageFilter);
    SkFlattenable::Register("ArithmeticImageFilterImpl", SkArithmeticImageFilter::CreateProc);
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
