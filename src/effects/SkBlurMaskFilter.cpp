
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
    virtual void computeFastBounds(const SkRect& src, SkRect* dst) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurMaskFilterImpl)

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

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkBlurMaskFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurMaskFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
