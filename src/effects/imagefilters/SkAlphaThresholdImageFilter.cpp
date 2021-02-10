/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkRegion.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/SkTPin.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/effects/generated/GrAlphaThresholdFragmentProcessor.h"
#endif

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

#if SK_SUPPORT_GPU
    GrSurfaceProxyView createMaskTexture(GrRecordingContext*,
                                         const SkMatrix&,
                                         const SkIRect& bounds) const;
#endif

private:
    friend void ::SkRegisterAlphaThresholdImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkAlphaThresholdImageFilter)

    SkRegion fRegion;
    SkScalar fInnerThreshold;
    SkScalar fOuterThreshold;

    using INHERITED = SkImageFilter_Base;
};

}; // end namespace

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

#if SK_SUPPORT_GPU
GrSurfaceProxyView SkAlphaThresholdImageFilter::createMaskTexture(GrRecordingContext* context,
                                                                  const SkMatrix& inMatrix,
                                                                  const SkIRect& bounds) const {
    auto rtContext = GrSurfaceDrawContext::MakeWithFallback(
            context, GrColorType::kAlpha_8, nullptr, SkBackingFit::kApprox, bounds.size());
    if (!rtContext) {
        return {};
    }

    SkRegion::Iterator iter(fRegion);
    rtContext->clear(SK_PMColor4fTRANSPARENT);

    while (!iter.done()) {
        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

        SkRect rect = SkRect::Make(iter.rect());

        rtContext->drawRect(nullptr, std::move(paint), GrAA::kNo, inMatrix, rect);

        iter.next();
    }

    return rtContext->readSurfaceView();
}
#endif

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

#if SK_SUPPORT_GPU
    if (ctx.gpuBacked()) {
        auto context = ctx.getContext();

        GrSurfaceProxyView inputView = (input->view(context));
        SkASSERT(inputView.asTextureProxy());
        const GrProtected isProtected = inputView.proxy()->isProtected();

        offset->fX = bounds.left();
        offset->fY = bounds.top();

        bounds.offset(-inputOffset);

        SkMatrix matrix(ctx.ctm());
        matrix.postTranslate(SkIntToScalar(-offset->fX), SkIntToScalar(-offset->fY));

        GrSurfaceProxyView maskView = this->createMaskTexture(context, matrix, bounds);
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

        auto thresholdFP = GrAlphaThresholdFragmentProcessor::Make(
                std::move(textureFP), std::move(maskFP), fInnerThreshold, fOuterThreshold);
        if (!thresholdFP) {
            return nullptr;
        }

        return DrawWithFP(context, std::move(thresholdFP), bounds, ctx.colorType(),
                          ctx.colorSpace(), isProtected);
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
                                          dst);
}
