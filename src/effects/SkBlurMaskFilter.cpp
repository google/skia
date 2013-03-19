
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMaskFilter.h"
#include "SkBlurMask.h"
#include "SkFlattenableBuffers.h"
#include "SkMaskFilter.h"
#include "SkBounder.h"
#include "SkRasterClip.h"
#include "SkRTConf.h"
#include "SkStringUtils.h"

class SkBlurMaskFilterImpl : public SkMaskFilter {
public:
    SkBlurMaskFilterImpl(SkScalar radius, SkBlurMaskFilter::BlurStyle,
                         uint32_t flags);

    // overrides from SkMaskFilter
    virtual SkMask::Format getFormat() const SK_OVERRIDE;
    virtual bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                            SkIPoint* margin) const SK_OVERRIDE;

    virtual BlurType asABlur(BlurInfo*) const SK_OVERRIDE;
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
    SkScalar                    fRadius;
    SkBlurMaskFilter::BlurStyle fBlurStyle;
    uint32_t                    fBlurFlags;

    SkBlurMaskFilterImpl(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    typedef SkMaskFilter INHERITED;
};

SkMaskFilter* SkBlurMaskFilter::Create(SkScalar radius,
                                       SkBlurMaskFilter::BlurStyle style,
                                       uint32_t flags) {
    // use !(radius > 0) instead of radius <= 0 to reject NaN values
    if (!(radius > 0) || (unsigned)style >= SkBlurMaskFilter::kBlurStyleCount
        || flags > SkBlurMaskFilter::kAll_BlurFlag) {
        return NULL;
    }

    return SkNEW_ARGS(SkBlurMaskFilterImpl, (radius, style, flags));
}

///////////////////////////////////////////////////////////////////////////////

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkScalar radius,
                                           SkBlurMaskFilter::BlurStyle style,
                                           uint32_t flags)
    : fRadius(radius), fBlurStyle(style), fBlurFlags(flags) {
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
    SkASSERT(radius >= 0);
    SkASSERT((unsigned)style < SkBlurMaskFilter::kBlurStyleCount);
    SkASSERT(flags <= SkBlurMaskFilter::kAll_BlurFlag);
}

SkMask::Format SkBlurMaskFilterImpl::getFormat() const {
    return SkMask::kA8_Format;
}

bool SkBlurMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                      const SkMatrix& matrix,
                                      SkIPoint* margin) const{
    SkScalar radius;
    if (fBlurFlags & SkBlurMaskFilter::kIgnoreTransform_BlurFlag) {
        radius = fRadius;
    } else {
        radius = matrix.mapRadius(fRadius);
    }

    // To avoid unseemly allocation requests (esp. for finite platforms like
    // handset) we limit the radius so something manageable. (as opposed to
    // a request like 10,000)
    static const SkScalar MAX_RADIUS = SkIntToScalar(128);
    radius = SkMinScalar(radius, MAX_RADIUS);
    SkBlurMask::Quality blurQuality =
        (fBlurFlags & SkBlurMaskFilter::kHighQuality_BlurFlag) ?
            SkBlurMask::kHigh_Quality : SkBlurMask::kLow_Quality;

#ifndef SK_DISABLE_SEPARABLE_MASK_BLUR
    return SkBlurMask::BlurSeparable(dst, src, radius, (SkBlurMask::Style)fBlurStyle,
                            blurQuality, margin);
#else
    return SkBlurMask::Blur(dst, src, radius, (SkBlurMask::Style)fBlurStyle,
                            blurQuality, margin);
#endif
}

bool SkBlurMaskFilterImpl::filterRectMask(SkMask* dst, const SkRect& r,
                                          const SkMatrix& matrix,
                                          SkIPoint* margin, SkMask::CreateMode createMode) const{
    SkScalar radius;
    if (fBlurFlags & SkBlurMaskFilter::kIgnoreTransform_BlurFlag) {
        radius = fRadius;
    } else {
        radius = matrix.mapRadius(fRadius);
    }

    // To avoid unseemly allocation requests (esp. for finite platforms like
    // handset) we limit the radius so something manageable. (as opposed to
    // a request like 10,000)
    static const SkScalar MAX_RADIUS = SkIntToScalar(128);
    radius = SkMinScalar(radius, MAX_RADIUS);

    return SkBlurMask::BlurRect(dst, r, radius, (SkBlurMask::Style)fBlurStyle,
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
    SkASSERT(!smallR[0].isEmpty());
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
    dst->set(src.fLeft - fRadius, src.fTop - fRadius,
             src.fRight + fRadius, src.fBottom + fRadius);
}

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkFlattenableReadBuffer& buffer)
        : SkMaskFilter(buffer) {
    fRadius = buffer.readScalar();
    fBlurStyle = (SkBlurMaskFilter::BlurStyle)buffer.readInt();
    fBlurFlags = buffer.readUInt() & SkBlurMaskFilter::kAll_BlurFlag;
    SkASSERT(fRadius >= 0);
    SkASSERT((unsigned)fBlurStyle < SkBlurMaskFilter::kBlurStyleCount);
}

void SkBlurMaskFilterImpl::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fRadius);
    buffer.writeInt(fBlurStyle);
    buffer.writeUInt(fBlurFlags);
}

static const SkMaskFilter::BlurType gBlurStyle2BlurType[] = {
    SkMaskFilter::kNormal_BlurType,
    SkMaskFilter::kSolid_BlurType,
    SkMaskFilter::kOuter_BlurType,
    SkMaskFilter::kInner_BlurType,
};

SkMaskFilter::BlurType SkBlurMaskFilterImpl::asABlur(BlurInfo* info) const {
    if (info) {
        info->fRadius = fRadius;
        info->fIgnoreTransform = SkToBool(fBlurFlags & SkBlurMaskFilter::kIgnoreTransform_BlurFlag);
        info->fHighQuality = SkToBool(fBlurFlags & SkBlurMaskFilter::kHighQuality_BlurFlag);
    }
    return gBlurStyle2BlurType[fBlurStyle];
}

#ifdef SK_DEVELOPER
void SkBlurMaskFilterImpl::toString(SkString* str) const {
    str->append("SkBlurMaskFilterImpl: (");

    str->append("radius: ");
    str->appendScalar(fRadius);
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
