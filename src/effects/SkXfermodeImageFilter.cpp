/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXfermodeImageFilter.h"
#include "SkArithmeticImageFilter.h"
#include "SkArithmeticModePriv.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "GrClip.h"
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

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkXfermodeImageFilter_Base)

protected:
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;

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
    sk_sp<GrFragmentProcessor> makeFGFrag(sk_sp<GrFragmentProcessor> bgFP) const;
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

static int unflatten_blendmode(SkReadBuffer& buffer, SkArithmeticParams* arith) {
    if (buffer.isVersionLT(SkReadBuffer::kXfermodeToBlendMode_Version)) {
        sk_sp<SkXfermode> xfer = buffer.readXfermode();
        if (xfer) {
            if (xfer->isArithmetic(arith)) {
                return -1;
            }
            return (int)xfer->blend();
        } else {
            return (int)SkBlendMode::kSrcOver;
        }
    } else {
        uint32_t mode = buffer.read32();
        (void)buffer.validate(mode <= (unsigned)SkBlendMode::kLastMode);
        return mode;
    }
}

sk_sp<SkFlattenable> SkXfermodeImageFilter_Base::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    SkArithmeticParams arith;
    int mode = unflatten_blendmode(buffer, &arith);
    if (mode >= 0) {
        return SkXfermodeImageFilter::Make((SkBlendMode)mode, common.getInput(0),
                                           common.getInput(1), &common.cropRect());
    } else {
        return SkArithmeticImageFilter::Make(arith.fK[0], arith.fK[1], arith.fK[2], arith.fK[3],
                                             arith.fEnforcePMColor, common.getInput(0),
                                             common.getInput(1), &common.cropRect());
    }
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

sk_sp<SkImageFilter> SkXfermodeImageFilter_Base::onMakeColorSpace(SkColorSpaceXformer* xformer)
const {
    SkASSERT(2 == this->countInputs());
    if (!this->getInput(0) && !this->getInput(1)) {
        return sk_ref_sp(const_cast<SkXfermodeImageFilter_Base*>(this));
    }

    sk_sp<SkImageFilter> background =
            this->getInput(0) ? this->getInput(0)->makeColorSpace(xformer) : nullptr;
    sk_sp<SkImageFilter> foreground =
            this->getInput(1) ? this->getInput(1)->makeColorSpace(xformer) : nullptr;

    return SkXfermodeImageFilter::Make(fMode, std::move(background), std::move(foreground),
                                       this->getCropRectIfSet());
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

#ifndef SK_IGNORE_TO_STRING
void SkXfermodeImageFilter_Base::toString(SkString* str) const {
    str->appendf("SkXfermodeImageFilter: (");
    str->appendf("blendmode: (%d)", (int)fMode);
    if (this->getInput(0)) {
        str->appendf("foreground: (");
        this->getInput(0)->toString(str);
        str->appendf(")");
    }
    if (this->getInput(1)) {
        str->appendf("background: (");
        this->getInput(1)->toString(str);
        str->appendf(")");
    }
    str->append(")");
}
#endif

#if SK_SUPPORT_GPU

#include "SkXfermode_proccoeff.h"

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
    sk_sp<GrFragmentProcessor> bgFP;

    if (backgroundProxy) {
        SkMatrix bgMatrix = SkMatrix::MakeTrans(-SkIntToScalar(backgroundOffset.fX),
                                                -SkIntToScalar(backgroundOffset.fY));
        sk_sp<GrColorSpaceXform> bgXform = GrColorSpaceXform::Make(background->getColorSpace(),
                                                                   outputProperties.colorSpace());
        bgFP = GrTextureDomainEffect::Make(
                            context->resourceProvider(), std::move(backgroundProxy),
                            std::move(bgXform), bgMatrix,
                            GrTextureDomain::MakeTexelDomain(background->subset()),
                            GrTextureDomain::kDecal_Mode,
                            GrSamplerParams::kNone_FilterMode);
    } else {
        bgFP = GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                           GrConstColorProcessor::kIgnore_InputMode);
    }

    if (foregroundProxy) {
        SkMatrix fgMatrix = SkMatrix::MakeTrans(-SkIntToScalar(foregroundOffset.fX),
                                                -SkIntToScalar(foregroundOffset.fY));
        sk_sp<GrColorSpaceXform> fgXform = GrColorSpaceXform::Make(foreground->getColorSpace(),
                                                                   outputProperties.colorSpace());
        sk_sp<GrFragmentProcessor> foregroundFP;

        foregroundFP = GrTextureDomainEffect::Make(
                            context->resourceProvider(), std::move(foregroundProxy),
                            std::move(fgXform), fgMatrix,
                            GrTextureDomain::MakeTexelDomain(foreground->subset()),
                            GrTextureDomain::kDecal_Mode,
                            GrSamplerParams::kNone_FilterMode);

        paint.addColorFragmentProcessor(std::move(foregroundFP));

        sk_sp<GrFragmentProcessor> xferFP = this->makeFGFrag(bgFP);

        // A null 'xferFP' here means kSrc_Mode was used in which case we can just proceed
        if (xferFP) {
            paint.addColorFragmentProcessor(std::move(xferFP));
        }
    } else {
        paint.addColorFragmentProcessor(std::move(bgFP));
    }

    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    sk_sp<GrRenderTargetContext> renderTargetContext(context->makeDeferredRenderTargetContext(
                                    SkBackingFit::kApprox, bounds.width(), bounds.height(),
                                    GrRenderableConfigForColorSpace(outputProperties.colorSpace()),
                                    sk_ref_sp(outputProperties.colorSpace())));
    if (!renderTargetContext) {
        return nullptr;
    }
    paint.setGammaCorrect(renderTargetContext->isGammaCorrect());

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, matrix,
                                  SkRect::Make(bounds));

    return SkSpecialImage::MakeDeferredFromGpu(context,
                                               SkIRect::MakeWH(bounds.width(), bounds.height()),
                                               kNeedNewImageUniqueID_SpecialImage,
                                               renderTargetContext->asTextureProxyRef(),
                                               renderTargetContext->refColorSpace());
}

sk_sp<GrFragmentProcessor>
SkXfermodeImageFilter_Base::makeFGFrag(sk_sp<GrFragmentProcessor> bgFP) const {
    // A null fMode is interpreted to mean kSrcOver_Mode (to match raster).
    SkXfermode* xfer = SkXfermode::Peek(fMode);
    sk_sp<SkXfermode> srcover;
    if (!xfer) {
        // It would be awesome to use SkXfermode::Create here but it knows better
        // than us and won't return a kSrcOver_Mode SkXfermode. That means we
        // have to get one the hard way.
        struct ProcCoeff rec;
        rec.fProc = SkXfermode::GetProc(SkBlendMode::kSrcOver);
        SkXfermode::ModeAsCoeff(SkBlendMode::kSrcOver, &rec.fSC, &rec.fDC);

        srcover.reset(new SkProcCoeffXfermode(rec, SkBlendMode::kSrcOver));
        xfer = srcover.get();

    }
    return xfer->makeFragmentProcessorForImageFilter(std::move(bgFP));
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkXfermodeImageFilter_Base::LegacyArithmeticCreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    // skip the unused mode (srcover) field
    SkDEBUGCODE(int mode =) unflatten_blendmode(buffer, nullptr);
    if (!buffer.isValid()) {
        return nullptr;
    }
    SkASSERT(SkBlendMode::kSrcOver == (SkBlendMode)mode);
    float k[4];
    for (int i = 0; i < 4; ++i) {
        k[i] = buffer.readScalar();
    }
    const bool enforcePMColor = buffer.readBool();
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
