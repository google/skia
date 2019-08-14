/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkAlphaThresholdFilter.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkRegion.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/effects/generated/GrAlphaThresholdFragmentProcessor.h"
#include "src/gpu/effects/generated/GrSimpleTextureEffect.h"
#endif

namespace {

class SkAlphaThresholdFilterImpl final : public SkImageFilter_Base {
public:
    SkAlphaThresholdFilterImpl(const SkRegion& region, SkScalar innerThreshold,
                               SkScalar outerThreshold, sk_sp<SkImageFilter> input,
                               const CropRect* cropRect = nullptr)
            : INHERITED(&input, 1, cropRect)
            , fRegion(region)
            , fInnerThreshold(innerThreshold)
            , fOuterThreshold(outerThreshold) {}

protected:
    void flatten(SkWriteBuffer&) const override;

    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> createMaskTexture(GrRecordingContext*,
                                            const SkMatrix&,
                                            const SkIRect& bounds) const;
#endif

private:
    friend void SkAlphaThresholdFilter::RegisterFlattenables();
    SK_FLATTENABLE_HOOKS(SkAlphaThresholdFilterImpl)

    SkRegion fRegion;
    SkScalar fInnerThreshold;
    SkScalar fOuterThreshold;

    typedef SkImageFilter_Base INHERITED;
};

}; // end namespace

sk_sp<SkImageFilter> SkAlphaThresholdFilter::Make(const SkRegion& region, SkScalar innerThreshold,
                                                  SkScalar outerThreshold,
                                                  sk_sp<SkImageFilter> input,
                                                  const SkImageFilter::CropRect* cropRect) {
    innerThreshold = SkScalarPin(innerThreshold, 0.f, 1.f);
    outerThreshold = SkScalarPin(outerThreshold, 0.f, 1.f);
    if (!SkScalarIsFinite(innerThreshold) || !SkScalarIsFinite(outerThreshold)) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkAlphaThresholdFilterImpl(
            region, innerThreshold, outerThreshold, std::move(input), cropRect));
    }

void SkAlphaThresholdFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkAlphaThresholdFilterImpl);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkAlphaThresholdFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar inner = buffer.readScalar();
    SkScalar outer = buffer.readScalar();
    SkRegion rgn;
    buffer.readRegion(&rgn);
    return SkAlphaThresholdFilter::Make(rgn, inner, outer, common.getInput(0),
                                        &common.cropRect());
}

void SkAlphaThresholdFilterImpl::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fInnerThreshold);
    buffer.writeScalar(fOuterThreshold);
    buffer.writeRegion(fRegion);
}

#if SK_SUPPORT_GPU
sk_sp<GrTextureProxy> SkAlphaThresholdFilterImpl::createMaskTexture(GrRecordingContext* context,
                                                                    const SkMatrix& inMatrix,
                                                                    const SkIRect& bounds) const {
    sk_sp<GrRenderTargetContext> rtContext(
            context->priv().makeDeferredRenderTargetContextWithFallback(
                    SkBackingFit::kApprox, bounds.width(), bounds.height(), GrColorType::kAlpha_8,
                    nullptr));
    if (!rtContext) {
        return nullptr;
    }

    SkRegion::Iterator iter(fRegion);
    rtContext->clear(nullptr, SK_PMColor4fTRANSPARENT,
                     GrRenderTargetContext::CanClearFullscreen::kYes);

    GrFixedClip clip(SkIRect::MakeWH(bounds.width(), bounds.height()));
    while (!iter.done()) {
        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

        SkRect rect = SkRect::Make(iter.rect());

        rtContext->drawRect(clip, std::move(paint), GrAA::kNo, inMatrix, rect);

        iter.next();
    }

    return rtContext->asTextureProxyRef();
}
#endif

sk_sp<SkSpecialImage> SkAlphaThresholdFilterImpl::onFilterImage(const Context& ctx,
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

        sk_sp<GrTextureProxy> inputProxy(input->asTextureProxyRef(context));
        SkASSERT(inputProxy);
        const bool isProtected = inputProxy->isProtected();

        offset->fX = bounds.left();
        offset->fY = bounds.top();

        bounds.offset(-inputOffset);

        SkMatrix matrix(ctx.ctm());
        matrix.postTranslate(SkIntToScalar(-offset->fX), SkIntToScalar(-offset->fY));

        sk_sp<GrTextureProxy> maskProxy(this->createMaskTexture(context, matrix, bounds));
        if (!maskProxy) {
            return nullptr;
        }

        auto textureFP = GrSimpleTextureEffect::Make(std::move(inputProxy),
                                                     SkMatrix::MakeTrans(input->subset().x(),
                                                                         input->subset().y()));
        textureFP = GrColorSpaceXformEffect::Make(std::move(textureFP), input->getColorSpace(),
                                                  input->alphaType(), ctx.colorSpace());
        if (!textureFP) {
            return nullptr;
        }

        auto thresholdFP = GrAlphaThresholdFragmentProcessor::Make(std::move(maskProxy),
                                                                   fInnerThreshold,
                                                                   fOuterThreshold,
                                                                   bounds);
        if (!thresholdFP) {
            return nullptr;
        }

        std::unique_ptr<GrFragmentProcessor> fpSeries[] = { std::move(textureFP),
                                                            std::move(thresholdFP) };
        auto fp = GrFragmentProcessor::RunInSeries(fpSeries, 2);

        return DrawWithFP(context, std::move(fp), bounds, ctx.colorType(), ctx.colorSpace(),
                          isProtected ? GrProtected::kYes : GrProtected::kNo);
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
