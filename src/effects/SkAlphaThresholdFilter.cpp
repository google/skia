/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAlphaThresholdFilter.h"

#include "SkBitmap.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkWriteBuffer.h"
#include "SkRegion.h"

#if SK_SUPPORT_GPU
#include "GrAlphaThresholdFragmentProcessor.h"
#include "GrContext.h"
#include "GrFixedClip.h"
#include "GrRenderTargetContext.h"
#include "GrTextureProxy.h"
#endif

class SK_API SkAlphaThresholdFilterImpl : public SkImageFilter {
public:
    SkAlphaThresholdFilterImpl(const SkRegion& region, SkScalar innerThreshold,
                               SkScalar outerThreshold, sk_sp<SkImageFilter> input,
                               const CropRect* cropRect = nullptr);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkAlphaThresholdFilterImpl)
    friend void SkAlphaThresholdFilter::InitializeFlattenables();

protected:
    void flatten(SkWriteBuffer&) const override;

    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> createMaskTexture(GrContext*,
                                            const SkMatrix&,
                                            const SkIRect& bounds) const;
#endif

private:
    SkRegion fRegion;
    SkScalar fInnerThreshold;
    SkScalar fOuterThreshold;
    typedef SkImageFilter INHERITED;
};

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkAlphaThresholdFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkAlphaThresholdFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

static SkScalar pin_0_1(SkScalar x) {
    return SkMinScalar(SkMaxScalar(x, 0), 1);
}

sk_sp<SkImageFilter> SkAlphaThresholdFilter::Make(const SkRegion& region,
                                                  SkScalar innerThreshold,
                                                  SkScalar outerThreshold,
                                                  sk_sp<SkImageFilter> input,
                                                  const SkImageFilter::CropRect* cropRect) {
    innerThreshold = pin_0_1(innerThreshold);
    outerThreshold = pin_0_1(outerThreshold);
    if (!SkScalarIsFinite(innerThreshold) || !SkScalarIsFinite(outerThreshold)) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkAlphaThresholdFilterImpl(region, innerThreshold,
                                                               outerThreshold,
                                                               std::move(input),
                                                               cropRect));
}

sk_sp<SkFlattenable> SkAlphaThresholdFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar inner = buffer.readScalar();
    SkScalar outer = buffer.readScalar();
    SkRegion rgn;
    buffer.readRegion(&rgn);
    return SkAlphaThresholdFilter::Make(rgn, inner, outer, common.getInput(0),
                                        &common.cropRect());
}

SkAlphaThresholdFilterImpl::SkAlphaThresholdFilterImpl(const SkRegion& region,
                                                       SkScalar innerThreshold,
                                                       SkScalar outerThreshold,
                                                       sk_sp<SkImageFilter> input,
                                                       const CropRect* cropRect)
    : INHERITED(&input, 1, cropRect)
    , fRegion(region)
    , fInnerThreshold(innerThreshold)
    , fOuterThreshold(outerThreshold) {
}

#if SK_SUPPORT_GPU
sk_sp<GrTextureProxy> SkAlphaThresholdFilterImpl::createMaskTexture(GrContext* context,
                                                                    const SkMatrix& inMatrix,
                                                                    const SkIRect& bounds) const {

    sk_sp<GrRenderTargetContext> rtContext(context->makeDeferredRenderTargetContextWithFallback(
        SkBackingFit::kApprox, bounds.width(), bounds.height(), kAlpha_8_GrPixelConfig, nullptr));
    if (!rtContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    SkRegion::Iterator iter(fRegion);
    rtContext->clear(nullptr, 0x0, true);

    GrFixedClip clip(SkIRect::MakeWH(bounds.width(), bounds.height()));
    while (!iter.done()) {
        SkRect rect = SkRect::Make(iter.rect());
        rtContext->drawRect(clip, std::move(paint), GrAA::kNo, inMatrix, rect);
        iter.next();
    }

    return rtContext->asTextureProxyRef();
}
#endif

void SkAlphaThresholdFilterImpl::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fInnerThreshold);
    buffer.writeScalar(fOuterThreshold);
    buffer.writeRegion(fRegion);
}

sk_sp<SkSpecialImage> SkAlphaThresholdFilterImpl::onFilterImage(SkSpecialImage* source,
                                                                const Context& ctx,
                                                                SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &inputOffset));
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
    if (source->isTextureBacked()) {
        GrContext* context = source->getContext();

        sk_sp<GrTextureProxy> inputProxy(input->asTextureProxyRef(context));
        SkASSERT(inputProxy);

        offset->fX = bounds.left();
        offset->fY = bounds.top();

        bounds.offset(-inputOffset);

        SkMatrix matrix(ctx.ctm());
        matrix.postTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));

        sk_sp<GrTextureProxy> maskProxy(this->createMaskTexture(context, matrix, bounds));
        if (!maskProxy) {
            return nullptr;
        }

        const OutputProperties& outProps = ctx.outputProperties();
        sk_sp<GrColorSpaceXform> colorSpaceXform = GrColorSpaceXform::Make(input->getColorSpace(),
                                                                           outProps.colorSpace());

        sk_sp<GrFragmentProcessor> fp(GrAlphaThresholdFragmentProcessor::Make(
                                            context->resourceProvider(),
                                            std::move(inputProxy),
                                            std::move(colorSpaceXform),
                                            std::move(maskProxy),
                                            fInnerThreshold,
                                            fOuterThreshold,
                                            bounds));
        if (!fp) {
            return nullptr;
        }

        return DrawWithFP(context, std::move(fp), bounds, outProps);
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

sk_sp<SkImageFilter> SkAlphaThresholdFilterImpl::onMakeColorSpace(SkColorSpaceXformer* xformer)
const {
    SkASSERT(1 == this->countInputs());
    if (!this->getInput(0)) {
        return sk_ref_sp(const_cast<SkAlphaThresholdFilterImpl*>(this));
    }

    sk_sp<SkImageFilter> input = this->getInput(0)->makeColorSpace(xformer);
    return SkAlphaThresholdFilter::Make(fRegion, fInnerThreshold, fOuterThreshold,
                                        std::move(input), this->getCropRectIfSet());
}

#ifndef SK_IGNORE_TO_STRING
void SkAlphaThresholdFilterImpl::toString(SkString* str) const {
    str->appendf("SkAlphaThresholdImageFilter: (");
    str->appendf("inner: %f outer: %f", fInnerThreshold, fOuterThreshold);
    str->append(")");
}
#endif
