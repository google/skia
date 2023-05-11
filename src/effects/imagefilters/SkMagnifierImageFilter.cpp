/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkTPin.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/imagefilters/SkCropImageFilter.h"

#include <algorithm>
#include <memory>
#include <utility>

#ifdef SK_ENABLE_SKSL
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"
#endif

#if defined(SK_GANESH)
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#endif

namespace {

// DEPRECATED: This implementation does not perform any bounds calculations, or respect the CTM,
// and only stores part of the state needed to correctly produce the magnifying lens effect. The
// rest of the state is calculated in Chromium and it relies on the fact that this implementation
// breaks the rules for everything to work out in the compositor.
// TODO: Delete this once Chromium has been updated to use the new magnifier factory and impl.
class SkLegacyMagnifierImageFilter final : public SkImageFilter_Base {
public:
    SkLegacyMagnifierImageFilter(const SkRect& srcRect, SkScalar inset, sk_sp<SkImageFilter> input,
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
    friend class SkMagnifierImageFilter; // For CreateProc on out-of-date SKPs
    friend void ::SkRegisterMagnifierImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkLegacyMagnifierImageFilter)

    SkRect   fSrcRect;
    SkScalar fInset;

    using INHERITED = SkImageFilter_Base;
};

class SkMagnifierImageFilter final : public SkImageFilter_Base {
public:
    SkMagnifierImageFilter(const SkRect& lensBounds,
                           float zoomAmount,
                           float inset,
                           const SkSamplingOptions& sampling,
                           sk_sp<SkImageFilter> input)
        : SkImageFilter_Base(&input, 1, nullptr)
        , fLensBounds(lensBounds)
        , fZoomAmount(zoomAmount)
        , fInset(inset)
        , fSampling(sampling) {}

    SkRect computeFastBounds(const SkRect&) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterMagnifierImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkMagnifierImageFilter)

    skif::FilterResult onFilterImage(const skif::Context& context) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds,
            VisitChildren recurse) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::ParameterSpace<SkRect> fLensBounds;
    // Zoom is relative so does not belong to a coordinate space, see note in onFilterImage().
    float fZoomAmount;
    // Inset is really a ParameterSpace<SkSize> where width = height = fInset, but we store just the
    // float here for easier serialization and convert to a size in onFilterImage().
    float fInset;
    SkSamplingOptions fSampling;
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
    return sk_sp<SkImageFilter>(new SkLegacyMagnifierImageFilter(srcRect, inset, std::move(input),
                                                                 cropRect));
}

sk_sp<SkImageFilter> SkImageFilters::Magnifier(const SkRect& lensBounds,
                                               SkScalar zoomAmount,
                                               SkScalar inset,
                                               const SkSamplingOptions& sampling,
                                               sk_sp<SkImageFilter> input,
                                               const CropRect& cropRect) {
    if (lensBounds.isEmpty() || !lensBounds.isFinite() ||
        zoomAmount <= 0.f || !SkScalarIsFinite(zoomAmount) ||
        inset < 0.f || !SkScalarIsFinite(inset)) {
        return nullptr; // invalid
    }
    // The magnifier automatically restricts its output based on the size of the image it receives
    // as input, so 'cropRect' only applies to its input.
    if (cropRect) {
        input = SkMakeCropImageFilter(*cropRect, std::move(input));
    }

    if (zoomAmount > 1.f) {
        return sk_sp<SkImageFilter>(new SkMagnifierImageFilter(lensBounds, zoomAmount, inset,
                                                               sampling, std::move(input)));
    } else {
        // Zooming with a value less than 1 is technically a downscaling, which "works" but the
        // non-linear distortion behaves unintuitively. At zoomAmount = 1, this filter is an
        // expensive identity function so treat zoomAmount <= 1 as a no-op.
        return input;
    }
}

void SkRegisterMagnifierImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkMagnifierImageFilter);
    SK_REGISTER_FLATTENABLE(SkLegacyMagnifierImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkMagnifierImageFilterImpl", SkLegacyMagnifierImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkLegacyMagnifierImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkRect src;
    buffer.readRect(&src);
    return SkImageFilters::Magnifier(src, buffer.readScalar(), common.getInput(0),
                                     common.cropRect());
}

void SkLegacyMagnifierImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeRect(fSrcRect);
    buffer.writeScalar(fInset);
}

sk_sp<SkFlattenable> SkMagnifierImageFilter::CreateProc(SkReadBuffer& buffer) {
    if (buffer.isVersionLT(SkPicturePriv::kRevampMagnifierFilter)) {
        // This was actually a legacy magnifier image filter that was serialized.
        return SkLegacyMagnifierImageFilter::CreateProc(buffer);
    }

    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    SkRect lensBounds;
    buffer.readRect(&lensBounds);
    SkScalar zoomAmount = buffer.readScalar();
    SkScalar inset = buffer.readScalar();
    SkSamplingOptions sampling = buffer.readSampling();
    return SkImageFilters::Magnifier(lensBounds, zoomAmount, inset, sampling, common.getInput(0));
}

void SkMagnifierImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeRect(SkRect(fLensBounds));
    buffer.writeScalar(fZoomAmount);
    buffer.writeScalar(fInset);
    buffer.writeSampling(fSampling);
}

////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkShader> make_magnifier_shader(
        const skif::Context& context,
        const skif::FilterResult& input,
        const SkSamplingOptions& sampling,
        const skif::LayerSpace<SkRect>& lensBounds,
        const skif::LayerSpace<SkRect>& srcRect,
        const skif::LayerSpace<SkSize>& inset) {
#ifdef SK_ENABLE_SKSL
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader src;"
        "uniform float4 lensBounds;"
        "uniform float4 zoomXform;"
        "uniform float2 invInset;"

        "half4 main(float2 coord) {"
            "float2 zoomCoord = zoomXform.xy + zoomXform.zw*coord;"
            // edgeInset is the smallest distance to the lens bounds edges,
            // in units of "insets".
            "float2 edgeInset = min(coord - lensBounds.xy, lensBounds.zw - coord) * invInset;"

            // The equations for 'weight' ensure that it is 0 along the outside of lensBounds so
            // it seams with any un-zoomed, un-filtered content. The zoomed content fills a rounded
            // rectangle that is 1 "inset" in from lensBounds with circular corners with radii
            // equal to the inset distance. Outside of this region, there is a non-linear weighting
            // to compress the un-zoomed content to the zoomed content. The critical zone about
            // each corner is limited to 2x"inset" square.
            "float weight = (edgeInset.x < 2.0 && edgeInset.y < 2.0)"
                // Circular distortion weighted by distance to inset corner
                "? (2.0 - length(2.0 - edgeInset))"
                // Linear zoom, or single-axis compression outside of the inset area (if delta < 1)
                ": min(edgeInset.x, edgeInset.y);"

            // Saturate before squaring so that negative weights are clamped to 0 before squaring
            "weight = saturate(weight);"
            "return src.eval(mix(coord, zoomCoord, weight*weight));"
        "}"
    );

    // TODO: FilterResult or FilterBuilder should hide the details of turning a FilterResult into
    // an SkShader (and possibly wrap binding the input for an SkRuntimeEffect, too).
    SkIPoint inputOrigin;
    sk_sp<SkSpecialImage> inputImage = input.imageAndOffset(context, &inputOrigin);
    if (!inputImage) {
        return nullptr;
    }
    sk_sp<SkShader> inputShader = inputImage->asShader(
            SkTileMode::kDecal, sampling, SkMatrix::Translate(inputOrigin.fX, inputOrigin.fY));
    if (!inputShader) {
        return nullptr;
    }

    SkRuntimeShaderBuilder builder(sk_ref_sp(effect));
    builder.child("src") = std::move(inputShader);

    SkMatrix zoomXform = SkMatrix::RectToRect(SkRect(lensBounds), SkRect(srcRect));
    builder.uniform("lensBounds") = SkRect(lensBounds);
    builder.uniform("zoomXform") = SkV4{zoomXform.getTranslateX(), zoomXform.getTranslateY(),
                                        zoomXform.getScaleX(),     zoomXform.getScaleY()};
    builder.uniform("invInset") = SkV2{1.f / inset.width(),
                                       1.f / inset.height()};

    return builder.makeShader();
#else
    // TODO (michaelludwig): Once the legacy magnifier is deleted, this SK_ENABLE_SKSL guard can
    // be moved to surround the entire implementation. Since sksl is required for this filter, the
    // Magnifier factory can be stubbed out easily at that point to return the input image filter.
    return nullptr;
#endif // SK_ENABLE_SKSL
}

#if defined(SK_GANESH)
static std::unique_ptr<GrFragmentProcessor> make_magnifier_fp(
        std::unique_ptr<GrFragmentProcessor> input,
        SkIRect bounds,
        SkRect srcRect,
        float xInvZoom,
        float yInvZoom,
        float xInvInset,
        float yInvInset) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader src;"
        "uniform float4 boundsUniform;"
        "uniform float  xInvZoom;"
        "uniform float  yInvZoom;"
        "uniform float  xInvInset;"
        "uniform float  yInvInset;"
        "uniform half2  offset;"

        "half4 main(float2 coord) {"
            "float2 zoom_coord = offset + coord * float2(xInvZoom, yInvZoom);"
            "float2 delta = (coord - boundsUniform.xy) * boundsUniform.zw;"
            "delta = min(delta, float2(1.0) - delta);"
            "delta *= float2(xInvInset, yInvInset);"

            "float weight = 0.0;"
            "if (delta.s < 2.0 && delta.t < 2.0) {"
                "delta = float2(2.0) - delta;"
                "float dist = length(delta);"
                "dist = max(2.0 - dist, 0.0);"
                "weight = min(dist * dist, 1.0);"
            "} else {"
                "float2 delta_squared = delta * delta;"
                "weight = min(min(delta_squared.x, delta_squared.y), 1.0);"
            "}"

            "return src.eval(mix(coord, zoom_coord, weight));"
        "}"
    );
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

sk_sp<SkSpecialImage> SkLegacyMagnifierImageFilter::onFilterImage(const Context& ctx,
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


#if defined(SK_GANESH)
    if (ctx.gpuBacked()) {
        auto context = ctx.getContext();

        GrSurfaceProxyView inputView = input->view(context);
        SkASSERT(inputView.asTextureProxy());

        const auto isProtected = inputView.proxy()->isProtected();
        const auto origin = inputView.origin();

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
                          ctx.surfaceProps(), origin, isProtected);
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

////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkMagnifierImageFilter::onFilterImage(const skif::Context& context) const {
    skif::FilterResult childOutput = this->filterInput(0, context);

    skif::LayerSpace<SkRect> lensBounds = context.mapping().paramToLayer(fLensBounds);
    skif::LayerSpace<SkPoint> zoomCenter = lensBounds.center();

    // If lensBounds is not partially off screen, 'childOutput' should exactly match the layer-space
    // lens bounds. However, when this is used as a backdrop filter, or if there was a crop on the
    // input, this may not be the case. Stylistically, this filter adjusts the lens bounds and
    // zoomed-in content such that the non-linear inset does not extend beyond what was provided.
    // This avoids zooming in on a clamped texture boundary.
    if (!lensBounds.intersect(skif::LayerSpace<SkRect>(childOutput.layerBounds()))) {
        return {};
    }
    // Clamp the zoom center to be within the childOutput image
    zoomCenter = lensBounds.clamp(zoomCenter);

    // The zoom we want to apply in layer-space is equal to
    // mapping.paramToLayer(SkMatrix::Scale(fZoomAmount)).decomposeScale(&layerZoom).
    // Because this filter only supports scale+translate matrices, the paramToLayer transform of
    // the parameter-space scale matrix is a no-op. Thus layerZoom == fZoomAmount and we can avoid
    // all of that math. This assumption is invalid if the matrix complexity is more than S+T.
    SkASSERT(this->getCTMCapability() == MatrixCapability::kScaleTranslate);
    float invZoom = 1.f / fZoomAmount;

    // The srcRect is the bounding box of the pixels that are linearly scaled up, about zoomCenter.
    // This is not the visual bounds of this upscaled region, but the bounds of the source pixels
    // that will fill the main magnified region (which is simply the inset of lensBounds). When
    // lensBounds has not been cropped by the actual input image, these equations are identical to
    // the more intuitive L/R = center.x -/+ width/(2*zoom) and T/B = center.y -/+ height/(2*zoom).
    // However, when lensBounds is cropped this automatically shifts the source rectangle away from
    // the original zoom center such that the upscaled area is contained within the input image.
    skif::LayerSpace<SkRect> srcRect{{
            lensBounds.left()  * invZoom + zoomCenter.x()*(1.f - invZoom),
            lensBounds.top()   * invZoom + zoomCenter.y()*(1.f - invZoom),
            lensBounds.right() * invZoom + zoomCenter.x()*(1.f - invZoom),
            lensBounds.bottom()* invZoom + zoomCenter.y()*(1.f - invZoom)}};

    skif::LayerSpace<SkSize> inset = context.mapping().paramToLayer(
            skif::ParameterSpace<SkSize>({fInset, fInset}));

    // TODO: FilterResult will eventually have a builder API to hide a lot of this boilerplate,
    // since it will likely be the same for many other image filter implementations. The magnifier
    // filter is just the first port to FilterResult that doesn't rely on applying meta transforms.
    skif::LayerSpace<SkIRect> outputBounds = lensBounds.roundOut();
    sk_sp<SkSpecialSurface> surf = context.makeSurface(SkISize(outputBounds.size()));
    if (!surf) {
        return {};
    }

    SkCanvas* canvas = surf->getCanvas();
    canvas->translate(-outputBounds.left(), -outputBounds.top());
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setShader(make_magnifier_shader(context, childOutput, fSampling,
                                          lensBounds, srcRect, inset));

    canvas->drawPaint(paint);

    return {surf->makeImageSnapshot(), outputBounds.topLeft()};
}

skif::LayerSpace<SkIRect> SkMagnifierImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds,
        VisitChildren recurse) const {
    // The required input is always the lens bounds. The filter distorts the pixels contained within
    // these bounds to zoom in on a portion of it, depending on the inset and zoom amount. However,
    // it adjusts the region based on cropping that occurs between what's requested and what's
    // provided. Theoretically it's possible that we could restrict the required input by the
    // desired output, but that cropping should not adjust the zoom region or inset. This is non
    // trivial to separate and is an unlikely use case so for now just require fLensBounds.
    skif::LayerSpace<SkIRect> requiredInput = mapping.paramToLayer(fLensBounds).roundOut();
    if (recurse == VisitChildren::kNo) {
        return requiredInput;
    } else {
        // Our required input is the desired output for our child image filter.
        return this->visitInputLayerBounds(mapping, requiredInput, contentBounds);
    }
}

skif::LayerSpace<SkIRect> SkMagnifierImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // The output of this filter is fLensBounds intersected with its child's output.
    skif::LayerSpace<SkIRect> output = this->visitOutputLayerBounds(mapping, contentBounds);
    if (output.intersect(mapping.paramToLayer(fLensBounds).roundOut())) {
        return output;
    } else {
        // Nothing to magnify
        return skif::LayerSpace<SkIRect>::Empty();
    }
}

SkRect SkMagnifierImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    if (bounds.intersect(SkRect(fLensBounds))) {
        return bounds;
    } else {
        return SkRect::MakeEmpty();
    }
}
