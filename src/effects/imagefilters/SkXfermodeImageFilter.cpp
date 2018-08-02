/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXfermodeImageFilter.h"
#include "SkArithmeticImageFilter.h"
#include "SkCanvas.h"
#include "SkColorData.h"
#include "SkColorSpaceXformer.h"
#include "SkFlattenablePriv.h"
#include "SkImageFilterPriv.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "GrClip.h"
#include "GrColorSpaceXform.h"
#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "GrTextureProxy.h"

#include "effects/GrConstColorProcessor.h"
#include "effects/GrTextureDomain.h"
#include "effects/GrSimpleTextureEffect.h"
#include "SkGr.h"
#endif
#include "SkClipOpPriv.h"

class SkXfermodeImageFilter_Base : public SkImageFilter {
public:
    SkXfermodeImageFilter_Base(SkBlendMode mode, sk_sp<SkImageFilter> inputs[2],
                               const CropRect* cropRect);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkXfermodeImageFilter_Base)

protected:
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;

    SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;

#if SK_SUPPORT_GPU
    sk_sp<SkSpecialImage> filterImageGPU(SkSpecialImage* source,
                                         sk_sp<SkSpecialImage> background,
                                         const SkIPoint& backgroundOffset,
                                         sk_sp<SkSpecialImage> foreground,
                                         const SkIPoint& foregroundOffset,
                                         const SkIRect& bounds,
                                         const OutputProperties& outputProperties) const;
#endif

    void flatten(SkWriteBuffer&) const override;

    void drawForeground(SkCanvas* canvas, SkSpecialImage*, const SkIRect&) const;
#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> makeFGFrag(
            std::unique_ptr<GrFragmentProcessor> bgFP) const;
#endif

private:
    static sk_sp<SkFlattenable> LegacyArithmeticCreateProc(SkReadBuffer& buffer);

    SkBlendMode fMode;

    friend class SkXfermodeImageFilter;

    typedef SkImageFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkXfermodeImageFilter::Make(SkBlendMode mode,
                                                 sk_sp<SkImageFilter> background,
                                                 sk_sp<SkImageFilter> foreground,
                                                 const SkImageFilter::CropRect* cropRect) {
    sk_sp<SkImageFilter> inputs[2] = { std::move(background), std::move(foreground) };
    return sk_sp<SkImageFilter>(new SkXfermodeImageFilter_Base(mode, inputs, cropRect));
}

SkXfermodeImageFilter_Base::SkXfermodeImageFilter_Base(SkBlendMode mode,
                                                       sk_sp<SkImageFilter> inputs[2],
                                                       const CropRect* cropRect)
    : INHERITED(inputs, 2, cropRect)
    , fMode(mode)
{}

static unsigned unflatten_blendmode(SkReadBuffer& buffer) {
    unsigned mode = buffer.read32();
    (void)buffer.validate(mode <= (unsigned)SkBlendMode::kLastMode);
    return mode;
}

sk_sp<SkFlattenable> SkXfermodeImageFilter_Base::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    unsigned mode = unflatten_blendmode(buffer);
    if (!buffer.isValid()) {
        return nullptr;
    }
    return SkXfermodeImageFilter::Make((SkBlendMode)mode, common.getInput(0),
                                       common.getInput(1), &common.cropRect());
}

void SkXfermodeImageFilter_Base::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.write32((unsigned)fMode);
}

sk_sp<SkSpecialImage> SkXfermodeImageFilter_Base::onFilterImage(SkSpecialImage* source,
                                                                const Context& ctx,
                                                                SkIPoint* offset) const {
    SkIPoint backgroundOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> background(this->filterInput(0, source, ctx, &backgroundOffset));

    SkIPoint foregroundOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> foreground(this->filterInput(1, source, ctx, &foregroundOffset));

    SkIRect foregroundBounds = SkIRect::EmptyIRect();
    if (foreground) {
        foregroundBounds = SkIRect::MakeXYWH(foregroundOffset.x(), foregroundOffset.y(),
                                             foreground->width(), foreground->height());
    }

    SkIRect srcBounds = SkIRect::EmptyIRect();
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

#if SK_SUPPORT_GPU
    if (source->isTextureBacked()) {
        return this->filterImageGPU(source,
                                    background, backgroundOffset,
                                    foreground, foregroundOffset,
                                    bounds, ctx.outputProperties());
    }
#endif

    sk_sp<SkSpecialSurface> surf(source->makeSurface(ctx.outputProperties(), bounds.size()));
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
                         &paint);
    }

    this->drawForeground(canvas, foreground.get(), foregroundBounds);

    return surf->makeImageSnapshot();
}

SkIRect SkXfermodeImageFilter_Base::onFilterBounds(const SkIRect& src,
                                                   const SkMatrix& ctm,
                                                   MapDirection dir,
                                                   const SkIRect* inputRect) const {
    if (kReverse_MapDirection == dir) {
        return SkImageFilter::onFilterBounds(src, ctm, dir, inputRect);
    }

    SkASSERT(!inputRect);
    SkASSERT(2 == this->countInputs());
    auto getBackground = [&]() {
        return this->getInput(0) ? this->getInput(0)->filterBounds(src, ctm, dir, inputRect) : src;
    };
    auto getForeground = [&]() {
        return this->getInput(1) ? this->getInput(1)->filterBounds(src, ctm, dir, inputRect) : src;
    };
    switch (fMode) {
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

        default: {
            auto result = getBackground();
            result.join(getForeground());
            return result;
        }
    }
}

sk_sp<SkImageFilter> SkXfermodeImageFilter_Base::onMakeColorSpace(SkColorSpaceXformer* xformer)
const {
    SkASSERT(2 == this->countInputs());
    auto background = xformer->apply(this->getInput(0));
    auto foreground = xformer->apply(this->getInput(1));
    if (background.get() != this->getInput(0) || foreground.get() != this->getInput(1)) {
        return SkXfermodeImageFilter::Make(fMode, std::move(background), std::move(foreground),
                                           this->getCropRectIfSet());
    }
    return this->refMe();
}

void SkXfermodeImageFilter_Base::drawForeground(SkCanvas* canvas, SkSpecialImage* img,
                                                const SkIRect& fgBounds) const {
    SkPaint paint;
    paint.setBlendMode(fMode);
    if (img) {
        img->draw(canvas, SkIntToScalar(fgBounds.fLeft), SkIntToScalar(fgBounds.fTop), &paint);
    }

    SkAutoCanvasRestore acr(canvas, true);
    canvas->clipRect(SkRect::Make(fgBounds), kDifference_SkClipOp);
    paint.setColor(0);
    canvas->drawPaint(paint);
}

#if SK_SUPPORT_GPU

#include "effects/GrXfermodeFragmentProcessor.h"

sk_sp<SkSpecialImage> SkXfermodeImageFilter_Base::filterImageGPU(
                                                   SkSpecialImage* source,
                                                   sk_sp<SkSpecialImage> background,
                                                   const SkIPoint& backgroundOffset,
                                                   sk_sp<SkSpecialImage> foreground,
                                                   const SkIPoint& foregroundOffset,
                                                   const SkIRect& bounds,
                                                   const OutputProperties& outputProperties) const {
    SkASSERT(source->isTextureBacked());

    GrContext* context = source->getContext();

    sk_sp<GrTextureProxy> backgroundProxy, foregroundProxy;

    if (background) {
        backgroundProxy = background->asTextureProxyRef(context);
    }

    if (foreground) {
        foregroundProxy = foreground->asTextureProxyRef(context);
    }

    GrPaint paint;
    std::unique_ptr<GrFragmentProcessor> bgFP;

    if (backgroundProxy) {
        SkMatrix bgMatrix = SkMatrix::MakeTrans(-SkIntToScalar(backgroundOffset.fX),
                                                -SkIntToScalar(backgroundOffset.fY));
        bgFP = GrTextureDomainEffect::Make(std::move(backgroundProxy), bgMatrix,
                                           GrTextureDomain::MakeTexelDomain(background->subset()),
                                           GrTextureDomain::kDecal_Mode,
                                           GrSamplerState::Filter::kNearest);
        bgFP = GrColorSpaceXformEffect::Make(std::move(bgFP), background->getColorSpace(),
                                             outputProperties.colorSpace());
    } else {
        bgFP = GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                           GrConstColorProcessor::InputMode::kIgnore);
    }

    if (foregroundProxy) {
        SkMatrix fgMatrix = SkMatrix::MakeTrans(-SkIntToScalar(foregroundOffset.fX),
                                                -SkIntToScalar(foregroundOffset.fY));
        auto foregroundFP = GrTextureDomainEffect::Make(
                std::move(foregroundProxy), fgMatrix,
                GrTextureDomain::MakeTexelDomain(foreground->subset()),
                GrTextureDomain::kDecal_Mode, GrSamplerState::Filter::kNearest);
        foregroundFP = GrColorSpaceXformEffect::Make(std::move(foregroundFP),
                                                     foreground->getColorSpace(),
                                                     outputProperties.colorSpace());
        paint.addColorFragmentProcessor(std::move(foregroundFP));

        std::unique_ptr<GrFragmentProcessor> xferFP = this->makeFGFrag(std::move(bgFP));

        // A null 'xferFP' here means kSrc_Mode was used in which case we can just proceed
        if (xferFP) {
            paint.addColorFragmentProcessor(std::move(xferFP));
        }
    } else {
        paint.addColorFragmentProcessor(std::move(bgFP));
    }

    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    sk_sp<GrRenderTargetContext> renderTargetContext(
        context->contextPriv().makeDeferredRenderTargetContext(
                                    SkBackingFit::kApprox, bounds.width(), bounds.height(),
                                    SkColorType2GrPixelConfig(outputProperties.colorType()),
                                    sk_ref_sp(outputProperties.colorSpace())));
    if (!renderTargetContext) {
        return nullptr;
    }

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, matrix,
                                  SkRect::Make(bounds));

    return SkSpecialImage::MakeDeferredFromGpu(
            context,
            SkIRect::MakeWH(bounds.width(), bounds.height()),
            kNeedNewImageUniqueID_SpecialImage,
            renderTargetContext->asTextureProxyRef(),
            renderTargetContext->colorSpaceInfo().refColorSpace());
}

std::unique_ptr<GrFragmentProcessor> SkXfermodeImageFilter_Base::makeFGFrag(
        std::unique_ptr<GrFragmentProcessor> bgFP) const {
    return GrXfermodeFragmentProcessor::MakeFromDstProcessor(std::move(bgFP), fMode);
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkXfermodeImageFilter_Base::LegacyArithmeticCreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    // skip the unused mode (srcover) field
    SkDEBUGCODE(unsigned mode =) unflatten_blendmode(buffer);
    if (!buffer.isValid()) {
        return nullptr;
    }
    SkASSERT(SkBlendMode::kSrcOver == (SkBlendMode)mode);
    float k[4];
    for (int i = 0; i < 4; ++i) {
        k[i] = buffer.readScalar();
    }
    const bool enforcePMColor = buffer.readBool();
    if (!buffer.isValid()) {
        return nullptr;
    }
    return SkArithmeticImageFilter::Make(k[0], k[1], k[2], k[3], enforcePMColor, common.getInput(0),
                                         common.getInput(1), &common.cropRect());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkXfermodeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkXfermodeImageFilter_Base)
    // manually register the legacy serialized name "SkXfermodeImageFilter"
    SkFlattenable::Register("SkXfermodeImageFilter", SkXfermodeImageFilter_Base::CreateProc,
                            SkFlattenable::kSkImageFilter_Type);
    // manually register the legacy serialized name "SkArithmeticImageFilter" from when that filter
    // was implemented as a xfermode image filter.
    SkFlattenable::Register("SkArithmeticImageFilter",
                            SkXfermodeImageFilter_Base::LegacyArithmeticCreateProc,
                            SkFlattenable::kSkImageFilter_Type);
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
