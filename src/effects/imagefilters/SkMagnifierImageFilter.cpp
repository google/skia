/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/SkColorData.h"
#include "include/private/SkTPin.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"

////////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/GrTextureEffect.h"
#endif

namespace {

class SkMagnifierImageFilter final : public SkImageFilter_Base {
public:
    SkMagnifierImageFilter(const SkRect& srcRect, SkScalar inset, sk_sp<SkImageFilter> input,
                           const SkRect* cropRect)
            : INHERITED(&input, 1, cropRect)
            , fSrcRect(srcRect)
            , fInset(inset) {
        SkASSERT(srcRect.left() >= 0 && srcRect.top() >= 0 && inset >= 0);
    }

protected:
    void flatten(SkWriteBuffer&) const override;

    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

private:
    friend void ::SkRegisterMagnifierImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkMagnifierImageFilter)

    SkRect   fSrcRect;
    SkScalar fInset;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Magnifier(
        const SkRect& srcRect, SkScalar inset, sk_sp<SkImageFilter> input,
        const CropRect& cropRect) {
    if (!SkScalarIsFinite(inset) || !SkIsValidRect(srcRect)) {
        return nullptr;
    }
    if (inset < 0) {
        return nullptr;
    }
    // Negative numbers in src rect are not supported
    if (srcRect.fLeft < 0 || srcRect.fTop < 0) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkMagnifierImageFilter(srcRect, inset, std::move(input),
                                                           cropRect));
}

void SkRegisterMagnifierImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkMagnifierImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkMagnifierImageFilterImpl", SkMagnifierImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkMagnifierImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkRect src;
    buffer.readRect(&src);
    return SkImageFilters::Magnifier(src, buffer.readScalar(), common.getInput(0),
                                     common.cropRect());
}

void SkMagnifierImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeRect(fSrcRect);
    buffer.writeScalar(fInset);
}

////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
static std::unique_ptr<GrFragmentProcessor> make_magnifier_fp(
        std::unique_ptr<GrFragmentProcessor> input,
        SkIRect bounds,
        SkRect srcRect,
        float xInvZoom,
        float yInvZoom,
        float xInvInset,
        float yInvInset) {
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform shader src;
        uniform float4 boundsUniform;
        uniform float  xInvZoom;
        uniform float  yInvZoom;
        uniform float  xInvInset;
        uniform float  yInvInset;
        uniform half2  offset;

        half4 main(float2 coord) {
            float2 zoom_coord = offset + coord * float2(xInvZoom, yInvZoom);
            float2 delta = (coord - boundsUniform.xy) * boundsUniform.zw;
            delta = min(delta, float2(1.0) - delta);
            delta *= float2(xInvInset, yInvInset);

            float weight = 0.0;
            if (delta.s < 2.0 && delta.t < 2.0) {
                delta = float2(2.0) - delta;
                float dist = length(delta);
                dist = max(2.0 - dist, 0.0);
                weight = min(dist * dist, 1.0);
            } else {
                float2 delta_squared = delta * delta;
                weight = min(min(delta_squared.x, delta_squared.y), 1.0);
            }

            return src.eval(mix(coord, zoom_coord, weight));
        }
    )");

    SkV4 boundsUniform = {static_cast<float>(bounds.x()),
                          static_cast<float>(bounds.y()),
                          1.f / bounds.width(),
                          1.f / bounds.height()};

    return GrSkSLFP::Make(effect, "magnifier_fp", /*inputFP=*/nullptr, GrSkSLFP::OptFlags::kNone,
                          "src", std::move(input),
                          "boundsUniform", boundsUniform,
                          "xInvZoom", xInvZoom,
                          "yInvZoom", yInvZoom,
                          "xInvInset", xInvInset,
                          "yInvInset", yInvInset,
                          "offset", SkV2{srcRect.x(), srcRect.y()});
}
#endif

sk_sp<SkSpecialImage> SkMagnifierImageFilter::onFilterImage(const Context& ctx,
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

    SkScalar invInset = fInset > 0 ? SkScalarInvert(fInset) : SK_Scalar1;

    SkScalar invXZoom = fSrcRect.width() / bounds.width();
    SkScalar invYZoom = fSrcRect.height() / bounds.height();


#if SK_SUPPORT_GPU
    if (ctx.gpuBacked()) {
        auto context = ctx.getContext();

        GrSurfaceProxyView inputView = input->view(context);
        SkASSERT(inputView.asTextureProxy());

        const auto isProtected = inputView.proxy()->isProtected();

        offset->fX = bounds.left();
        offset->fY = bounds.top();
        bounds.offset(-inputOffset);

        // Map bounds and srcRect into the proxy space. Due to the zoom effect,
        // it's not just an offset for fSrcRect.
        bounds.offset(input->subset().x(), input->subset().y());
        SkRect srcRect = fSrcRect.makeOffset((1.f - invXZoom) * input->subset().x(),
                                             (1.f - invYZoom) * input->subset().y());
        auto inputFP = GrTextureEffect::Make(std::move(inputView), kPremul_SkAlphaType);

        auto fp = make_magnifier_fp(std::move(inputFP),
                                    bounds,
                                    srcRect,
                                    invXZoom,
                                    invYZoom,
                                    bounds.width() * invInset,
                                    bounds.height() * invInset);

        fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                           input->getColorSpace(), input->alphaType(),
                                           ctx.colorSpace(), kPremul_SkAlphaType);
        if (!fp) {
            return nullptr;
        }

        return DrawWithFP(context, std::move(fp), bounds, ctx.colorType(), ctx.colorSpace(),
                          ctx.surfaceProps(), isProtected);
    }
#endif

    SkBitmap inputBM;

    if (!input->getROPixels(&inputBM)) {
        return nullptr;
    }

    if ((inputBM.colorType() != kN32_SkColorType) ||
        (fSrcRect.width() >= inputBM.width()) || (fSrcRect.height() >= inputBM.height())) {
        return nullptr;
    }

    SkASSERT(inputBM.getPixels());
    if (!inputBM.getPixels() || inputBM.width() <= 0 || inputBM.height() <= 0) {
        return nullptr;
    }

    const SkImageInfo info = SkImageInfo::MakeN32Premul(bounds.width(), bounds.height());

    SkBitmap dst;
    if (!dst.tryAllocPixels(info)) {
        return nullptr;
    }

    SkColor* dptr = dst.getAddr32(0, 0);
    int dstWidth = dst.width(), dstHeight = dst.height();
    for (int y = 0; y < dstHeight; ++y) {
        for (int x = 0; x < dstWidth; ++x) {
            SkScalar x_dist = std::min(x, dstWidth - x - 1) * invInset;
            SkScalar y_dist = std::min(y, dstHeight - y - 1) * invInset;
            SkScalar weight = 0;

            static const SkScalar kScalar2 = SkScalar(2);

            // To create a smooth curve at the corners, we need to work on
            // a square twice the size of the inset.
            if (x_dist < kScalar2 && y_dist < kScalar2) {
                x_dist = kScalar2 - x_dist;
                y_dist = kScalar2 - y_dist;

                SkScalar dist = SkScalarSqrt(SkScalarSquare(x_dist) +
                                             SkScalarSquare(y_dist));
                dist = std::max(kScalar2 - dist, 0.0f);
                // SkTPin rather than std::max to handle potential NaN
                weight = SkTPin(SkScalarSquare(dist), 0.0f, SK_Scalar1);
            } else {
                SkScalar sqDist = std::min(SkScalarSquare(x_dist),
                                           SkScalarSquare(y_dist));
                // SkTPin rather than std::max to handle potential NaN
                weight = SkTPin(sqDist, 0.0f, SK_Scalar1);
            }

            SkScalar x_interp = weight * (fSrcRect.x() + x * invXZoom) + (1 - weight) * x;
            SkScalar y_interp = weight * (fSrcRect.y() + y * invYZoom) + (1 - weight) * y;

            int x_val = SkTPin(bounds.x() + SkScalarFloorToInt(x_interp), 0, inputBM.width() - 1);
            int y_val = SkTPin(bounds.y() + SkScalarFloorToInt(y_interp), 0, inputBM.height() - 1);

            *dptr = *inputBM.getAddr32(x_val, y_val);
            dptr++;
        }
    }

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(bounds.width(), bounds.height()),
                                          dst, ctx.surfaceProps());
}
