
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

class SkBlurMaskFilterImpl : public SkMaskFilter {
public:
    SkBlurMaskFilterImpl(SkScalar radius, SkBlurMaskFilter::BlurStyle,
                         uint32_t flags);

    // overrides from SkMaskFilter
    virtual SkMask::Format getFormat() SK_OVERRIDE;
    virtual bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                            SkIPoint* margin) SK_OVERRIDE;
    virtual BlurType asABlur(BlurInfo*) const SK_OVERRIDE;
    virtual void setAsABlur(const BlurInfo&) SK_OVERRIDE;
    virtual void computeFastBounds(const SkRect& src, SkRect* dst) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurMaskFilterImpl)

protected:
    virtual FilterReturn filterRectToNine(const SkRect&, const SkMatrix&,
                                          const SkIRect& clipBounds,
                                          SkMask* ninePatchMask,
                                          SkIRect* outerRect) SK_OVERRIDE;

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

SkMask::Format SkBlurMaskFilterImpl::getFormat() {
    return SkMask::kA8_Format;
}

bool SkBlurMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                      const SkMatrix& matrix, SkIPoint* margin) {
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

    return SkBlurMask::Blur(dst, src, radius, (SkBlurMask::Style)fBlurStyle,
                            blurQuality, margin);
}

#include "SkCanvas.h"

static bool drawRectIntoMask(const SkRect& r, SkMask* mask) {
    r.roundOut(&mask->fBounds);
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
    canvas.translate(-SkScalarFloorToScalar(r.left()),
                    -SkScalarFloorToScalar(r.top()));

    SkPaint paint;
    paint.setAntiAlias(true);

    canvas.drawRect(r, paint);
    return true;
}

SkMaskFilter::FilterReturn
SkBlurMaskFilterImpl::filterRectToNine(const SkRect& rect, const SkMatrix& matrix,
                                       const SkIRect& clipBounds,
                                       SkMask* ninePatchMask,
                                       SkIRect* outerRect) {
    SkIPoint margin;
    SkMask  srcM, dstM;
    rect.roundOut(&srcM.fBounds);
    srcM.fImage = NULL;
    srcM.fFormat = SkMask::kA8_Format;
    srcM.fRowBytes = 0;
    if (!this->filterMask(&dstM, srcM, matrix, &margin)) {
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
    SkRect smallR = rect;
    {
        // +3 is from +1 for each edge (to account for possible fractional pixel
        // edges, and +1 to make room for a center rol/col.
        int smallW = dstM.fBounds.width() - srcM.fBounds.width() + 3;
        int smallH = dstM.fBounds.height() - srcM.fBounds.height() + 3;
        // we want the inset amounts to be integral, so we don't change any
        // fractional phase on the fRight or fBottom of our smallR.
        SkScalar dx = SkIntToScalar(srcM.fBounds.width() - smallW);
        SkScalar dy = SkIntToScalar(srcM.fBounds.height() - smallH);
        if (dx < 0 || dy < 0) {
            // we're too small, relative to our blur, to break into nine-patch,
            // so we ask to have our normal filterMask() be called.
            return kUnimplemented_FilterReturn;
        }
        SkASSERT(dx >= 0 && dy >= 0);
        smallR.set(rect.left(), rect.top(), rect.right() - dx, rect.bottom() - dy);
        SkASSERT(!smallR.isEmpty());
    }

    if (!drawRectIntoMask(smallR, &srcM)) {
        return kFalse_FilterReturn;
    }

    if (!this->filterMask(ninePatchMask, srcM, matrix, &margin)) {
        return kFalse_FilterReturn;
    }
    ninePatchMask->fBounds.offsetTo(0, 0);
    *outerRect = dstM.fBounds;
    return kTrue_FilterReturn;
}

void SkBlurMaskFilterImpl::computeFastBounds(const SkRect& src, SkRect* dst) {
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

void SkBlurMaskFilterImpl::setAsABlur(const BlurInfo& info) {
    fRadius = info.fRadius;
    fBlurFlags = (fBlurFlags & ~(SkBlurMaskFilter::kIgnoreTransform_BlurFlag
                                 | SkBlurMaskFilter::kHighQuality_BlurFlag))
            | (info.fIgnoreTransform ? SkBlurMaskFilter::kIgnoreTransform_BlurFlag : 0)
            | (info.fHighQuality ? SkBlurMaskFilter::kHighQuality_BlurFlag : 0);
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkBlurMaskFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurMaskFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
