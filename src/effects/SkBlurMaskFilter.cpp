
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMaskFilter.h"
#include "SkBlurMask.h"
#include "SkGpuBlurUtils.h"
#include "SkFlattenableBuffers.h"
#include "SkMaskFilter.h"
#include "SkRTConf.h"
#include "SkStringUtils.h"
#include "SkStrokeRec.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTexture.h"
#include "effects/GrSimpleTextureEffect.h"
#include "SkGrPixelRef.h"
#endif

class SkBlurMaskFilterImpl : public SkMaskFilter {
public:
    SkBlurMaskFilterImpl(SkScalar sigma, SkBlurMaskFilter::BlurStyle, uint32_t flags);

    // overrides from SkMaskFilter
    virtual SkMask::Format getFormat() const SK_OVERRIDE;
    virtual bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                            SkIPoint* margin) const SK_OVERRIDE;

#if SK_SUPPORT_GPU
    virtual bool canFilterMaskGPU(const SkRect& devBounds,
                                  const SkIRect& clipBounds,
                                  const SkMatrix& ctm,
                                  SkRect* maskRect) const SK_OVERRIDE;
    virtual bool filterMaskGPU(GrTexture* src,
                               const SkRect& maskRect,
                               GrTexture** result,
                               bool canOverwriteSrc) const;
#endif

    virtual void computeFastBounds(const SkRect&, SkRect*) const SK_OVERRIDE;

    SkDEVCODE(virtual void toString(SkString* str) const SK_OVERRIDE;)
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurMaskFilterImpl)

protected:
    virtual FilterReturn filterRectsToNine(const SkRect[], int count, const SkMatrix&,
                                           const SkIRect& clipBounds,
                                           NinePatch*) const SK_OVERRIDE;

    bool filterRectMask(SkMask* dstM, const SkRect& r, const SkMatrix& matrix,
                        SkIPoint* margin, SkMask::CreateMode createMode) const;

private:
    // To avoid unseemly allocation requests (esp. for finite platforms like
    // handset) we limit the radius so something manageable. (as opposed to
    // a request like 10,000)
    static const SkScalar kMAX_BLUR_SIGMA;

    SkScalar                    fSigma;
    SkBlurMaskFilter::BlurStyle fBlurStyle;
    uint32_t                    fBlurFlags;

    SkBlurMaskFilterImpl(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    SkScalar computeXformedSigma(const SkMatrix& ctm) const {
        bool ignoreTransform = SkToBool(fBlurFlags & SkBlurMaskFilter::kIgnoreTransform_BlurFlag);

        SkScalar xformedSigma = ignoreTransform ? fSigma : ctm.mapRadius(fSigma);
        return SkMinScalar(xformedSigma, kMAX_BLUR_SIGMA);
    }

    typedef SkMaskFilter INHERITED;
};

const SkScalar SkBlurMaskFilterImpl::kMAX_BLUR_SIGMA = SkIntToScalar(128);

SkMaskFilter* SkBlurMaskFilter::Create(SkScalar radius,
                                       SkBlurMaskFilter::BlurStyle style,
                                       uint32_t flags) {
    // use !(radius > 0) instead of radius <= 0 to reject NaN values
    if (!(radius > 0) || (unsigned)style >= SkBlurMaskFilter::kBlurStyleCount
        || flags > SkBlurMaskFilter::kAll_BlurFlag) {
        return NULL;
    }

    SkScalar sigma = SkBlurMask::ConvertRadiusToSigma(radius);

    return SkNEW_ARGS(SkBlurMaskFilterImpl, (sigma, style, flags));
}

SkMaskFilter* SkBlurMaskFilter::Create(SkBlurMaskFilter::BlurStyle style,
                                       SkScalar sigma,
                                       uint32_t flags) {
    // use !(sigma > 0) instead of sigma <= 0 to reject NaN values
    if (!(sigma > 0) || (unsigned)style >= SkBlurMaskFilter::kBlurStyleCount
        || flags > SkBlurMaskFilter::kAll_BlurFlag) {
        return NULL;
    }

    return SkNEW_ARGS(SkBlurMaskFilterImpl, (sigma, style, flags));
}

///////////////////////////////////////////////////////////////////////////////

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkScalar sigma,
                                           SkBlurMaskFilter::BlurStyle style,
                                           uint32_t flags)
    : fSigma(sigma), fBlurStyle(style), fBlurFlags(flags) {
#if 0
    fGamma = NULL;
    if (gammaScale) {
        fGamma = new U8[256];
        if (gammaScale > 0)
            SkBlurMask::BuildSqrGamma(fGamma, gammaScale);
        else
            SkBlurMask::BuildSqrtGamma(fGamma, -gammaScale);
    }
#endif
    SkASSERT(fSigma >= 0);
    SkASSERT((unsigned)style < SkBlurMaskFilter::kBlurStyleCount);
    SkASSERT(flags <= SkBlurMaskFilter::kAll_BlurFlag);
}

SkMask::Format SkBlurMaskFilterImpl::getFormat() const {
    return SkMask::kA8_Format;
}

bool SkBlurMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                      const SkMatrix& matrix,
                                      SkIPoint* margin) const{
    SkScalar sigma = this->computeXformedSigma(matrix);

    SkBlurMask::Quality blurQuality =
        (fBlurFlags & SkBlurMaskFilter::kHighQuality_BlurFlag) ?
            SkBlurMask::kHigh_Quality : SkBlurMask::kLow_Quality;

    return SkBlurMask::BoxBlur(dst, src, sigma, (SkBlurMask::Style)fBlurStyle,
                               blurQuality, margin);
}

bool SkBlurMaskFilterImpl::filterRectMask(SkMask* dst, const SkRect& r,
                                          const SkMatrix& matrix,
                                          SkIPoint* margin, SkMask::CreateMode createMode) const{
    SkScalar sigma = computeXformedSigma(matrix);

    return SkBlurMask::BlurRect(sigma, dst, r, (SkBlurMask::Style)fBlurStyle,
                                margin, createMode);
}

#include "SkCanvas.h"

static bool drawRectsIntoMask(const SkRect rects[], int count, SkMask* mask) {
    rects[0].roundOut(&mask->fBounds);
    mask->fRowBytes = SkAlign4(mask->fBounds.width());
    mask->fFormat = SkMask::kA8_Format;
    size_t size = mask->computeImageSize();
    mask->fImage = SkMask::AllocImage(size);
    if (NULL == mask->fImage) {
        return false;
    }
    sk_bzero(mask->fImage, size);

    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kA8_Config,
                     mask->fBounds.width(), mask->fBounds.height(),
                     mask->fRowBytes);
    bitmap.setPixels(mask->fImage);

    SkCanvas canvas(bitmap);
    canvas.translate(-SkIntToScalar(mask->fBounds.left()),
                     -SkIntToScalar(mask->fBounds.top()));

    SkPaint paint;
    paint.setAntiAlias(true);

    if (1 == count) {
        canvas.drawRect(rects[0], paint);
    } else {
        // todo: do I need a fast way to do this?
        SkPath path;
        path.addRect(rects[0]);
        path.addRect(rects[1]);
        path.setFillType(SkPath::kEvenOdd_FillType);
        canvas.drawPath(path, paint);
    }
    return true;
}

static bool rect_exceeds(const SkRect& r, SkScalar v) {
    return r.fLeft < -v || r.fTop < -v || r.fRight > v || r.fBottom > v ||
           r.width() > v || r.height() > v;
}

#ifdef SK_IGNORE_FAST_RECT_BLUR
SK_CONF_DECLARE( bool, c_analyticBlurNinepatch, "mask.filter.analyticNinePatch", false, "Use the faster analytic blur approach for ninepatch rects" );
#else
SK_CONF_DECLARE( bool, c_analyticBlurNinepatch, "mask.filter.analyticNinePatch", true, "Use the faster analytic blur approach for ninepatch rects" );
#endif

SkMaskFilter::FilterReturn
SkBlurMaskFilterImpl::filterRectsToNine(const SkRect rects[], int count,
                                        const SkMatrix& matrix,
                                        const SkIRect& clipBounds,
                                        NinePatch* patch) const {
    if (count < 1 || count > 2) {
        return kUnimplemented_FilterReturn;
    }

    // TODO: report correct metrics for innerstyle, where we do not grow the
    // total bounds, but we do need an inset the size of our blur-radius
    if (SkBlurMaskFilter::kInner_BlurStyle == fBlurStyle) {
        return kUnimplemented_FilterReturn;
    }

    // TODO: take clipBounds into account to limit our coordinates up front
    // for now, just skip too-large src rects (to take the old code path).
    if (rect_exceeds(rects[0], SkIntToScalar(32767))) {
        return kUnimplemented_FilterReturn;
    }

    SkIPoint margin;
    SkMask  srcM, dstM;
    rects[0].roundOut(&srcM.fBounds);
    srcM.fImage = NULL;
    srcM.fFormat = SkMask::kA8_Format;
    srcM.fRowBytes = 0;

    bool filterResult = false;
    if (count == 1 && c_analyticBlurNinepatch) {
        // special case for fast rect blur
        // don't actually do the blur the first time, just compute the correct size
        filterResult = this->filterRectMask(&dstM, rects[0], matrix, &margin,
                                            SkMask::kJustComputeBounds_CreateMode);
    } else {
        filterResult = this->filterMask(&dstM, srcM, matrix, &margin);
    }

    if (!filterResult) {
        return kFalse_FilterReturn;
    }

    /*
     *  smallR is the smallest version of 'rect' that will still guarantee that
     *  we get the same blur results on all edges, plus 1 center row/col that is
     *  representative of the extendible/stretchable edges of the ninepatch.
     *  Since our actual edge may be fractional we inset 1 more to be sure we
     *  don't miss any interior blur.
     *  x is an added pixel of blur, and { and } are the (fractional) edge
     *  pixels from the original rect.
     *
     *   x x { x x .... x x } x x
     *
     *  Thus, in this case, we inset by a total of 5 (on each side) beginning
     *  with our outer-rect (dstM.fBounds)
     */
    SkRect smallR[2];
    SkIPoint center;

    // +2 is from +1 for each edge (to account for possible fractional edges
    int smallW = dstM.fBounds.width() - srcM.fBounds.width() + 2;
    int smallH = dstM.fBounds.height() - srcM.fBounds.height() + 2;
    SkIRect innerIR;

    if (1 == count) {
        innerIR = srcM.fBounds;
        center.set(smallW, smallH);
    } else {
        SkASSERT(2 == count);
        rects[1].roundIn(&innerIR);
        center.set(smallW + (innerIR.left() - srcM.fBounds.left()),
                   smallH + (innerIR.top() - srcM.fBounds.top()));
    }

    // +1 so we get a clean, stretchable, center row/col
    smallW += 1;
    smallH += 1;

    // we want the inset amounts to be integral, so we don't change any
    // fractional phase on the fRight or fBottom of our smallR.
    const SkScalar dx = SkIntToScalar(innerIR.width() - smallW);
    const SkScalar dy = SkIntToScalar(innerIR.height() - smallH);
    if (dx < 0 || dy < 0) {
        // we're too small, relative to our blur, to break into nine-patch,
        // so we ask to have our normal filterMask() be called.
        return kUnimplemented_FilterReturn;
    }

    smallR[0].set(rects[0].left(), rects[0].top(), rects[0].right() - dx, rects[0].bottom() - dy);
    if (smallR[0].width() < 2 || smallR[0].height() < 2) {
        return kUnimplemented_FilterReturn;
    }
    if (2 == count) {
        smallR[1].set(rects[1].left(), rects[1].top(),
                      rects[1].right() - dx, rects[1].bottom() - dy);
        SkASSERT(!smallR[1].isEmpty());
    }

    if (count > 1 || !c_analyticBlurNinepatch) {
        if (!drawRectsIntoMask(smallR, count, &srcM)) {
            return kFalse_FilterReturn;
        }

        SkAutoMaskFreeImage amf(srcM.fImage);

        if (!this->filterMask(&patch->fMask, srcM, matrix, &margin)) {
            return kFalse_FilterReturn;
        }
    } else {
        if (!this->filterRectMask(&patch->fMask, smallR[0], matrix, &margin,
                                  SkMask::kComputeBoundsAndRenderImage_CreateMode)) {
            return kFalse_FilterReturn;
        }
    }
    patch->fMask.fBounds.offsetTo(0, 0);
    patch->fOuterRect = dstM.fBounds;
    patch->fCenter = center;
    return kTrue_FilterReturn;
}

void SkBlurMaskFilterImpl::computeFastBounds(const SkRect& src,
                                             SkRect* dst) const {
    SkScalar pad = 3.0f * fSigma;

    dst->set(src.fLeft  - pad, src.fTop    - pad,
             src.fRight + pad, src.fBottom + pad);
}

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkFlattenableReadBuffer& buffer)
        : SkMaskFilter(buffer) {
    fSigma = buffer.readScalar();
#ifndef DELETE_THIS_CODE_WHEN_SKPS_ARE_REBUILT_AT_V13_AND_ALL_OTHER_INSTANCES_TOO
    // Fixing this must be done in two stages. When the skps are recaptured in V13,
    // remove the ConvertRadiusToSigma but retain the absolute value.
    // At the same time, switch the code in flatten to write a positive value.
    // When the skps are captured in V14 the absolute value can be removed.
    if (fSigma > 0) {
        fSigma = SkBlurMask::ConvertRadiusToSigma(fSigma);
    } else {
        fSigma = -fSigma;
    }
#endif
    fBlurStyle = (SkBlurMaskFilter::BlurStyle)buffer.readInt();
    fBlurFlags = buffer.readUInt() & SkBlurMaskFilter::kAll_BlurFlag;
    SkASSERT(fSigma >= 0);
    SkASSERT((unsigned)fBlurStyle < SkBlurMaskFilter::kBlurStyleCount);
}

void SkBlurMaskFilterImpl::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(-fSigma);
    buffer.writeInt(fBlurStyle);
    buffer.writeUInt(fBlurFlags);
}

#if SK_SUPPORT_GPU

bool SkBlurMaskFilterImpl::canFilterMaskGPU(const SkRect& srcBounds,
                                            const SkIRect& clipBounds,
                                            const SkMatrix& ctm,
                                            SkRect* maskRect) const {
    SkScalar xformedSigma = this->computeXformedSigma(ctm);
    if (xformedSigma <= 0) {
        return false;
    }

    static const SkScalar kMIN_GPU_BLUR_SIZE  = SkIntToScalar(64);
    static const SkScalar kMIN_GPU_BLUR_SIGMA = SkIntToScalar(32);

    if (srcBounds.width() <= kMIN_GPU_BLUR_SIZE &&
        srcBounds.height() <= kMIN_GPU_BLUR_SIZE &&
        xformedSigma <= kMIN_GPU_BLUR_SIGMA) {
        // We prefer to blur small rect with small radius via CPU.
        return false;
    }

    if (NULL == maskRect) {
        // don't need to compute maskRect
        return true;
    }

    float sigma3 = 3 * SkScalarToFloat(xformedSigma);

    SkRect clipRect = SkRect::MakeFromIRect(clipBounds);
    SkRect srcRect(srcBounds);

    // Outset srcRect and clipRect by 3 * sigma, to compute affected blur area.
    srcRect.outset(SkFloatToScalar(sigma3), SkFloatToScalar(sigma3));
    clipRect.outset(SkFloatToScalar(sigma3), SkFloatToScalar(sigma3));
    srcRect.intersect(clipRect);
    *maskRect = srcRect;
    return true;
}

bool SkBlurMaskFilterImpl::filterMaskGPU(GrTexture* src,
                                         const SkRect& maskRect,
                                         GrTexture** result,
                                         bool canOverwriteSrc) const {
    SkRect clipRect = SkRect::MakeWH(maskRect.width(), maskRect.height());

    GrContext* context = src->getContext();

    GrContext::AutoWideOpenIdentityDraw awo(context, NULL);

    SkScalar xformedSigma = this->computeXformedSigma(context->getMatrix());
    SkASSERT(xformedSigma > 0);

    // If we're doing a normal blur, we can clobber the pathTexture in the
    // gaussianBlur.  Otherwise, we need to save it for later compositing.
    bool isNormalBlur = (SkBlurMaskFilter::kNormal_BlurStyle == fBlurStyle);
    *result = SkGpuBlurUtils::GaussianBlur(context, src, isNormalBlur && canOverwriteSrc,
                                           clipRect, false, xformedSigma, xformedSigma);
    if (NULL == *result) {
        return false;
    }

    if (!isNormalBlur) {
        context->setIdentityMatrix();
        GrPaint paint;
        SkMatrix matrix;
        matrix.setIDiv(src->width(), src->height());
        // Blend pathTexture over blurTexture.
        GrContext::AutoRenderTarget art(context, (*result)->asRenderTarget());
        paint.addColorEffect(GrSimpleTextureEffect::Create(src, matrix))->unref();
        if (SkBlurMaskFilter::kInner_BlurStyle == fBlurStyle) {
            // inner:  dst = dst * src
            paint.setBlendFunc(kDC_GrBlendCoeff, kZero_GrBlendCoeff);
        } else if (SkBlurMaskFilter::kSolid_BlurStyle == fBlurStyle) {
            // solid:  dst = src + dst - src * dst
            //             = (1 - dst) * src + 1 * dst
            paint.setBlendFunc(kIDC_GrBlendCoeff, kOne_GrBlendCoeff);
        } else if (SkBlurMaskFilter::kOuter_BlurStyle == fBlurStyle) {
            // outer:  dst = dst * (1 - src)
            //             = 0 * src + (1 - src) * dst
            paint.setBlendFunc(kZero_GrBlendCoeff, kISC_GrBlendCoeff);
        }
        context->drawRect(paint, clipRect);
    }

    return true;
}

#endif // SK_SUPPORT_GPU


#ifdef SK_DEVELOPER
void SkBlurMaskFilterImpl::toString(SkString* str) const {
    str->append("SkBlurMaskFilterImpl: (");

    str->append("sigma: ");
    str->appendScalar(fSigma);
    str->append(" ");

    static const char* gStyleName[SkBlurMaskFilter::kBlurStyleCount] = {
        "normal", "solid", "outer", "inner"
    };

    str->appendf("style: %s ", gStyleName[fBlurStyle]);
    str->append("flags: (");
    if (fBlurFlags) {
        bool needSeparator = false;
        SkAddFlagToString(str,
                          SkToBool(fBlurFlags & SkBlurMaskFilter::kIgnoreTransform_BlurFlag),
                          "IgnoreXform", &needSeparator);
        SkAddFlagToString(str,
                          SkToBool(fBlurFlags & SkBlurMaskFilter::kHighQuality_BlurFlag),
                          "HighQuality", &needSeparator);
    } else {
        str->append("None");
    }
    str->append("))");
}
#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkBlurMaskFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurMaskFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
