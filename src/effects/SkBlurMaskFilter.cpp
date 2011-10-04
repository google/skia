
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBlurMaskFilter.h"
#include "SkBlurMask.h"
#include "SkBuffer.h"
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

    // overrides from SkFlattenable
    virtual Factory getFactory() SK_OVERRIDE;
    virtual void flatten(SkFlattenableWriteBuffer&) SK_OVERRIDE;

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer&);

private:
    SkScalar                    fRadius;
    SkBlurMaskFilter::BlurStyle fBlurStyle;
    uint32_t                    fBlurFlags;

    SkBlurMaskFilterImpl(SkFlattenableReadBuffer&);
    
    typedef SkMaskFilter INHERITED;
};

SkMaskFilter* SkBlurMaskFilter::Create(SkScalar radius,
                                       SkBlurMaskFilter::BlurStyle style,
                                       uint32_t flags) {
    if (radius <= 0 || (unsigned)style >= SkBlurMaskFilter::kBlurStyleCount 
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

SkFlattenable* SkBlurMaskFilterImpl::CreateProc(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(SkBlurMaskFilterImpl, (buffer));
}

SkFlattenable::Factory SkBlurMaskFilterImpl::getFactory() {
    return CreateProc;
}

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkFlattenableReadBuffer& buffer)
        : SkMaskFilter(buffer) {
    fRadius = buffer.readScalar();
    fBlurStyle = (SkBlurMaskFilter::BlurStyle)buffer.readS32();
    fBlurFlags = buffer.readU32() & SkBlurMaskFilter::kAll_BlurFlag;
    SkASSERT(fRadius >= 0);
    SkASSERT((unsigned)fBlurStyle < SkBlurMaskFilter::kBlurStyleCount);
}

void SkBlurMaskFilterImpl::flatten(SkFlattenableWriteBuffer& buffer) {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fRadius);
    buffer.write32(fBlurStyle);
    buffer.write32(fBlurFlags);
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

///////////////////////////////////////////////////////////////////////////////

static SkFlattenable::Registrar gReg("SkBlurMaskFilter",
                                     SkBlurMaskFilterImpl::CreateProc);

