/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/base/SkTPin.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"

#include <cstdint>
#include <memory>
#include <utility>

#if defined(SK_GANESH)
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"

class GrRecordingContext;
class SkSurfaceProps;
enum GrSurfaceOrigin : int;
namespace skgpu { enum class Protected : bool; }

#endif // defined(SK_GANESH)

namespace {

class SkAlphaThresholdImageFilter final : public SkImageFilter_Base {
public:
    SkAlphaThresholdImageFilter(const SkRegion& region, SkScalar innerThreshold,
                                SkScalar outerThreshold, sk_sp<SkImageFilter> input,
                                const SkRect* cropRect = nullptr)
            : INHERITED(&input, 1, cropRect)
            , fRegion(region)
            , fInnerThreshold(innerThreshold)
            , fOuterThreshold(outerThreshold) {}

protected:
    void flatten(SkWriteBuffer&) const override;

    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

#if defined(SK_GANESH)
    GrSurfaceProxyView createMaskTexture(GrRecordingContext*,
                                         const SkMatrix&,
                                         const SkIRect& bounds,
                                         const SkSurfaceProps&) const;
#endif

private:
    friend void ::SkRegisterAlphaThresholdImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkAlphaThresholdImageFilter)

    SkRegion fRegion;
    SkScalar fInnerThreshold;
    SkScalar fOuterThreshold;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::AlphaThreshold(
        const SkRegion& region, SkScalar innerMin, SkScalar outerMax, sk_sp<SkImageFilter> input,
        const CropRect& cropRect) {
    innerMin = SkTPin(innerMin, 0.f, 1.f);
    outerMax = SkTPin(outerMax, 0.f, 1.f);
    if (!SkScalarIsFinite(innerMin) || !SkScalarIsFinite(outerMax)) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkAlphaThresholdImageFilter(
            region, innerMin, outerMax, std::move(input), cropRect));
}

void SkRegisterAlphaThresholdImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkAlphaThresholdImageFilter);
    SkFlattenable::Register("SkAlphaThresholdFilterImpl", SkAlphaThresholdImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkAlphaThresholdImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar inner = buffer.readScalar();
    SkScalar outer = buffer.readScalar();
    SkRegion rgn;
    buffer.readRegion(&rgn);
    return SkImageFilters::AlphaThreshold(rgn, inner, outer, common.getInput(0), common.cropRect());
}

void SkAlphaThresholdImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fInnerThreshold);
    buffer.writeScalar(fOuterThreshold);
    buffer.writeRegion(fRegion);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(SK_GANESH)
GrSurfaceProxyView SkAlphaThresholdImageFilter::createMaskTexture(
        GrRecordingContext* rContext,
        const SkMatrix& inMatrix,
        const SkIRect& bounds,
        const SkSurfaceProps& surfaceProps) const {
    auto sdc = skgpu::v1::SurfaceDrawContext::MakeWithFallback(
            rContext, GrColorType::kAlpha_8, nullptr, SkBackingFit::kApprox, bounds.size(),
            surfaceProps);
    if (!sdc) {
        return {};
    }

    SkRegion::Iterator iter(fRegion);
    sdc->clear(SK_PMColor4fTRANSPARENT);

    while (!iter.done()) {
        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

        SkRect rect = SkRect::Make(iter.rect());

        sdc->drawRect(nullptr, std::move(paint), GrAA::kNo, inMatrix, rect);

        iter.next();
    }

    return sdc->readSurfaceView();
}

static std::unique_ptr<GrFragmentProcessor> make_alpha_threshold_fp(
        std::unique_ptr<GrFragmentProcessor> inputFP,
        std::unique_ptr<GrFragmentProcessor> maskFP,
        float innerThreshold,
        float outerThreshold) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader maskFP;"
        "uniform shader inputFP;"
        "uniform half innerThreshold;"
        "uniform half outerThreshold;"

        "half4 main(float2 xy) {"
            "half4 color = inputFP.eval(xy);"
            "half4 mask_color = maskFP.eval(xy);"
            "if (mask_color.a < 0.5) {"
                "if (color.a > outerThreshold) {"
                    "half scale = outerThreshold / color.a;"
                    "color.rgb *= scale;"
                    "color.a = outerThreshold;"
                "}"
            "} else if (color.a < innerThreshold) {"
                "half scale = innerThreshold / max(0.001, color.a);"
                "color.rgb *= scale;"
                "color.a = innerThreshold;"
            "}"
            "return color;"
        "}"
    );

    return GrSkSLFP::Make(effect, "AlphaThreshold", /*inputFP=*/nullptr,
                          (outerThreshold >= 1.0f) ? GrSkSLFP::OptFlags::kPreservesOpaqueInput
                                                   : GrSkSLFP::OptFlags::kNone,
                          "maskFP", GrSkSLFP::IgnoreOptFlags(std::move(maskFP)),
                          "inputFP", std::move(inputFP),
                          "innerThreshold", innerThreshold,
                          "outerThreshold", outerThreshold);
}
#endif  // defined(SK_GANESH)

sk_sp<SkSpecialImage> SkAlphaThresholdImageFilter::onFilterImage(const Context& ctx,
                                                                 SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    const SkIRect inputBounds = SkIRect::MakeXYWH(inputOffset.x(), inputOffset.y(),
                                                  input->width(), input->height());

    SkIRect bounds;
    if (!this->applyCropRect(ctx, inputBounds, &bounds)) {
        return nullptr;
    }

#if defined(SK_GANESH)
    if (ctx.gpuBacked()) {
        auto context = ctx.getContext();

        GrSurfaceProxyView inputView = (input->view(context));
        SkASSERT(inputView.asTextureProxy());
        const skgpu::Protected isProtected = inputView.proxy()->isProtected();
        const GrSurfaceOrigin origin = inputView.origin();

        offset->fX = bounds.left();
        offset->fY = bounds.top();

        bounds.offset(-inputOffset);

        SkMatrix matrix(ctx.ctm());
        matrix.postTranslate(SkIntToScalar(-offset->fX), SkIntToScalar(-offset->fY));

        GrSurfaceProxyView maskView = this->createMaskTexture(context, matrix, bounds,
                                                              ctx.surfaceProps());
        if (!maskView) {
            return nullptr;
        }
        auto maskFP = GrTextureEffect::Make(std::move(maskView), kPremul_SkAlphaType,
                                            SkMatrix::Translate(-bounds.x(), -bounds.y()));

        auto textureFP = GrTextureEffect::Make(
                std::move(inputView), input->alphaType(),
                SkMatrix::Translate(input->subset().x(), input->subset().y()));
        textureFP = GrColorSpaceXformEffect::Make(std::move(textureFP),
                                                  input->getColorSpace(), input->alphaType(),
                                                  ctx.colorSpace(), kPremul_SkAlphaType);
        if (!textureFP) {
            return nullptr;
        }

        auto thresholdFP = make_alpha_threshold_fp(
                std::move(textureFP), std::move(maskFP), fInnerThreshold, fOuterThreshold);
        if (!thresholdFP) {
            return nullptr;
        }

        return DrawWithFP(context, std::move(thresholdFP), bounds, ctx.colorType(),
                          ctx.colorSpace(), ctx.surfaceProps(), origin, isProtected);
    }
#endif

    SkBitmap inputBM;

    if (!input->getROPixels(&inputBM)) {
        return nullptr;
    }

    if (inputBM.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    if (!inputBM.getPixels() || inputBM.width() <= 0 || inputBM.height() <= 0) {
        return nullptr;
    }


    SkMatrix localInverse;
    if (!ctx.ctm().invert(&localInverse)) {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::MakeN32(bounds.width(), bounds.height(),
                                            kPremul_SkAlphaType);

    SkBitmap dst;
    if (!dst.tryAllocPixels(info)) {
        return nullptr;
    }

    U8CPU innerThreshold = (U8CPU)(fInnerThreshold * 0xFF);
    U8CPU outerThreshold = (U8CPU)(fOuterThreshold * 0xFF);
    SkColor* dptr = dst.getAddr32(0, 0);
    int dstWidth = dst.width(), dstHeight = dst.height();
    SkIPoint srcOffset = { bounds.fLeft - inputOffset.fX, bounds.fTop - inputOffset.fY };
    for (int y = 0; y < dstHeight; ++y) {
        const SkColor* sptr = inputBM.getAddr32(srcOffset.fX, srcOffset.fY+y);

        for (int x = 0; x < dstWidth; ++x) {
            const SkColor& source = sptr[x];
            SkColor outputColor(source);
            SkPoint position;
            localInverse.mapXY((SkScalar)x + bounds.fLeft, (SkScalar)y + bounds.fTop, &position);
            if (fRegion.contains((int32_t)position.x(), (int32_t)position.y())) {
                if (SkColorGetA(source) < innerThreshold) {
                    U8CPU alpha = SkColorGetA(source);
                    if (alpha == 0) {
                        alpha = 1;
                    }
                    float scale = (float)innerThreshold / alpha;
                    outputColor = SkColorSetARGB(innerThreshold,
                                                  (U8CPU)(SkColorGetR(source) * scale),
                                                  (U8CPU)(SkColorGetG(source) * scale),
                                                  (U8CPU)(SkColorGetB(source) * scale));
                }
            } else {
                if (SkColorGetA(source) > outerThreshold) {
                    float scale = (float)outerThreshold / SkColorGetA(source);
                    outputColor = SkColorSetARGB(outerThreshold,
                                                  (U8CPU)(SkColorGetR(source) * scale),
                                                  (U8CPU)(SkColorGetG(source) * scale),
                                                  (U8CPU)(SkColorGetB(source) * scale));
                }
            }
            dptr[y * dstWidth + x] = outputColor;
        }
    }

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(bounds.width(), bounds.height()),
                                          dst, ctx.surfaceProps());
}
