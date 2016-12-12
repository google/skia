/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXfermodeImageFilter.h"
#include "SkArithmeticModePriv.h"

#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "GrTextureProxy.h"

#include "effects/GrConstColorProcessor.h"
#include "effects/GrTextureDomain.h"
#include "effects/GrSimpleTextureEffect.h"
#include "SkArithmeticMode_gpu.h"
#include "SkGr.h"
#include "SkGrPriv.h"
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

    virtual void drawForeground(SkCanvas* canvas, SkSpecialImage*, const SkIRect&) const;
#if SK_SUPPORT_GPU
    virtual sk_sp<GrFragmentProcessor> makeFGFrag(sk_sp<GrFragmentProcessor> bgFP) const;
#endif

private:
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
        return SkXfermodeImageFilter::MakeArithmetic(arith.fK[0], arith.fK[1], arith.fK[2],
                                                     arith.fK[3], arith.fEnforcePMColor,
                                                     common.getInput(0),
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

    sk_sp<GrTexture> backgroundTex, foregroundTex;
    
    if (background) {
        backgroundTex = background->asTextureRef(context);
    }

    if (foreground) {
        foregroundTex = foreground->asTextureRef(context);
    }

    GrPaint paint;
    sk_sp<GrFragmentProcessor> bgFP;

    if (backgroundTex) {
        SkMatrix backgroundMatrix;
        backgroundMatrix.setIDiv(backgroundTex->width(), backgroundTex->height());
        backgroundMatrix.preTranslate(-SkIntToScalar(backgroundOffset.fX),
                                      -SkIntToScalar(backgroundOffset.fY));
        bgFP = GrTextureDomainEffect::Make(
                            backgroundTex.get(), nullptr, backgroundMatrix,
                            GrTextureDomain::MakeTexelDomain(backgroundTex.get(),
                                                             background->subset()),
                            GrTextureDomain::kDecal_Mode,
                            GrSamplerParams::kNone_FilterMode);
    } else {
        bgFP = GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                           GrConstColorProcessor::kIgnore_InputMode);
    }

    if (foregroundTex) {
        SkMatrix foregroundMatrix;
        foregroundMatrix.setIDiv(foregroundTex->width(), foregroundTex->height());
        foregroundMatrix.preTranslate(-SkIntToScalar(foregroundOffset.fX),
                                      -SkIntToScalar(foregroundOffset.fY));

        sk_sp<GrFragmentProcessor> foregroundFP;

        foregroundFP = GrTextureDomainEffect::Make(
                            foregroundTex.get(), nullptr, foregroundMatrix,
                            GrTextureDomain::MakeTexelDomain(foregroundTex.get(), 
                                                             foreground->subset()),
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
    renderTargetContext->drawRect(GrNoClip(), paint, GrAA::kNo, matrix, SkRect::Make(bounds));

    return SkSpecialImage::MakeDeferredFromGpu(context,
                                               SkIRect::MakeWH(bounds.width(), bounds.height()),
                                               kNeedNewImageUniqueID_SpecialImage,
                                               sk_ref_sp(renderTargetContext->asDeferredTexture()),
                                               sk_ref_sp(renderTargetContext->getColorSpace()));
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

class SkArithmeticImageFilter : public SkXfermodeImageFilter_Base {
public:
    SkArithmeticImageFilter(float k1, float k2, float k3, float k4, bool enforcePMColor,
                            sk_sp<SkImageFilter> inputs[2], const CropRect* cropRect)
        // need to pass a blendmode to our inherited constructor, but we ignore it
        : SkXfermodeImageFilter_Base(SkBlendMode::kSrcOver, inputs, cropRect)
        , fK{ k1, k2, k3, k4 }
        , fEnforcePMColor(enforcePMColor)
    {}

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkArithmeticImageFilter)

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        this->INHERITED::flatten(buffer);
        for (int i = 0; i < 4; ++i) {
            buffer.writeScalar(fK[i]);
        }
        buffer.writeBool(fEnforcePMColor);
    }
    void drawForeground(SkCanvas* canvas, SkSpecialImage*, const SkIRect&) const override;
#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> makeFGFrag(sk_sp<GrFragmentProcessor> bgFP) const override {
        return GrArithmeticFP::Make(fK[0], fK[1], fK[2], fK[3], fEnforcePMColor, std::move(bgFP));
    }
#endif

private:
    const float fK[4];
    const bool fEnforcePMColor;

    friend class SkXfermodeImageFilter;

    typedef SkXfermodeImageFilter_Base INHERITED;
};

sk_sp<SkFlattenable> SkArithmeticImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);

    // skip the mode (srcover) our parent-class wrote
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
    return SkXfermodeImageFilter::MakeArithmetic(k[0], k[1], k[2], k[3], enforcePMColor,
                                                 common.getInput(0), common.getInput(1),
                                                 &common.cropRect());
}

#include "SkNx.h"

static Sk4f pin(float min, const Sk4f& val, float max) {
    return Sk4f::Max(min, Sk4f::Min(val, max));
}

template<bool EnforcePMColor> void arith_span(const float k[], SkPMColor dst[],
                                              const SkPMColor src[], int count) {
    const Sk4f k1 = k[0] * (1/255.0f),
               k2 = k[1],
               k3 = k[2],
               k4 = k[3] * 255.0f + 0.5f;

    for (int i = 0; i < count; i++) {
        Sk4f s = SkNx_cast<float>(Sk4b::Load(src+i)),
        d = SkNx_cast<float>(Sk4b::Load(dst+i)),
        r = pin(0, k1*s*d + k2*s + k3*d + k4, 255);
        if (EnforcePMColor) {
            Sk4f a = SkNx_shuffle<3,3,3,3>(r);
            r = Sk4f::Min(a, r);
        }
        SkNx_cast<uint8_t>(r).store(dst+i);
    }
}

// apply mode to src==transparent (0)
template<bool EnforcePMColor> void arith_transparent(const float k[], SkPMColor dst[], int count) {
    const Sk4f k3 = k[2],
               k4 = k[3] * 255.0f + 0.5f;

    for (int i = 0; i < count; i++) {
        Sk4f d = SkNx_cast<float>(Sk4b::Load(dst+i)),
             r = pin(0, k3*d + k4, 255);
        if (EnforcePMColor) {
            Sk4f a = SkNx_shuffle<3,3,3,3>(r);
            r = Sk4f::Min(a, r);
        }
        SkNx_cast<uint8_t>(r).store(dst+i);
    }
}

static bool intersect(SkPixmap* dst, SkPixmap* src, int srcDx, int srcDy) {
    SkIRect dstR = SkIRect::MakeWH(dst->width(), dst->height());
    SkIRect srcR = SkIRect::MakeXYWH(srcDx, srcDy, src->width(), src->height());
    SkIRect sect;
    if (!sect.intersect(dstR, srcR)) {
        return false;
    }
    *dst = SkPixmap(dst->info().makeWH(sect.width(), sect.height()),
                    dst->addr(sect.fLeft, sect.fTop),
                    dst->rowBytes());
    *src = SkPixmap(src->info().makeWH(sect.width(), sect.height()),
                    src->addr(SkTMax(0, -srcDx), SkTMax(0, -srcDy)),
                    src->rowBytes());
    return true;
}

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

    if (img) {
        SkBitmap srcBM;
        SkPixmap src;
        if (!img->getROPixels(&srcBM)) {
            return;
        }
        srcBM.lockPixels();
        if (!srcBM.peekPixels(&src)) {
            return;
        }

        auto proc = fEnforcePMColor ? arith_span<true> : arith_span<false>;
        SkPixmap tmpDst = dst;
        if (intersect(&tmpDst, &src, fgBounds.fLeft + dx, fgBounds.fTop + dy)) {
            for (int y = 0; y < tmpDst.height(); ++y) {
                proc(fK, tmpDst.writable_addr32(0, y), src.addr32(0, y), tmpDst.width());
            }
        }
    }

    // Now apply the mode with transparent-color to the outside of the fg image
    SkRegion outside(SkIRect::MakeWH(dst.width(), dst.height()));
    outside.op(fgBounds.makeOffset(dx, dy), SkRegion::kDifference_Op);
    auto proc = fEnforcePMColor ? arith_transparent<true> : arith_transparent<false>;
    for (SkRegion::Iterator iter(outside); !iter.done(); iter.next()) {
        const SkIRect r = iter.rect();
        for (int y = r.fTop; y < r.fBottom; ++y) {
            proc(fK, dst.writable_addr32(r.fLeft, y), r.width());
        }
    }
}

sk_sp<SkImageFilter> SkXfermodeImageFilter::MakeArithmetic(float k1, float k2, float k3, float k4,
                                                           bool enforcePMColor,
                                                           sk_sp<SkImageFilter> background,
                                                           sk_sp<SkImageFilter> foreground,
                                                           const SkImageFilter::CropRect* crop) {
    if (!SkScalarIsFinite(k1) || !SkScalarIsFinite(k2) ||
        !SkScalarIsFinite(k3) || !SkScalarIsFinite(k4)) {
        return nullptr;
    }

    // are we nearly some other "std" mode?
    int mode = -1;  // illegal mode
    if (SkScalarNearlyZero(k1) && SkScalarNearlyEqual(k2, SK_Scalar1) &&
        SkScalarNearlyZero(k3) && SkScalarNearlyZero(k4)) {
        mode = (int)SkBlendMode::kSrc;
    } else if (SkScalarNearlyZero(k1) && SkScalarNearlyZero(k2) &&
               SkScalarNearlyEqual(k3, SK_Scalar1) && SkScalarNearlyZero(k4)) {
        mode = (int)SkBlendMode::kDst;
    } else if (SkScalarNearlyZero(k1) && SkScalarNearlyZero(k2) &&
               SkScalarNearlyZero(k3) && SkScalarNearlyZero(k4)) {
        mode = (int)SkBlendMode::kClear;
    }
    if (mode >= 0) {
        return SkXfermodeImageFilter::Make((SkBlendMode)mode,
                                           std::move(background), std::move(foreground), crop);
    }

    sk_sp<SkImageFilter> inputs[2] = { std::move(background), std::move(foreground) };
    return sk_sp<SkImageFilter>(new SkArithmeticImageFilter(k1, k2, k3, k4, enforcePMColor,
                                                            inputs, crop));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkXfermodeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkXfermodeImageFilter_Base)
    // manually register the legacy serialized name "SkXfermodeImageFilter"
    SkFlattenable::Register("SkXfermodeImageFilter", SkXfermodeImageFilter_Base::CreateProc,
                            SkFlattenable::kSkImageFilter_Type);
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkArithmeticImageFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
