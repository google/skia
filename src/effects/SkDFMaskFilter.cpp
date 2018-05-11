/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDFMaskFilter.h"
#include "../../src/gpu/text/GrSDFMaskFilter.h"
#include "SkMaskFilterBase.h"
#include "SkReadBuffer.h"
#include "SkSafeMath.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

#if 0
static float scurve(float x) {
    SkASSERT(x >= 0 && x <= 1);
    return x*x*(3 - 2*x);
}
#endif

static uint8_t convert_sdf_to_a8(uint8_t distanceByte, float window, float offset) {
    float distance = ((int)distanceByte - 128) * (1.0f/32);
    float radius = window * 0.5f;
    distance += offset;
    if (distance > radius) {
        return 0xFF;
    }
    if (distance < -radius) {
        return 0;
    }
    distance += radius;
    distance *= 255 / window;
    return distance + 0.5;
}

static void ConvertDFToA8(const SkMask& src, int dx, int dy, const SkMask& dst,
                          float window, float offset) {
    SkASSERT(dst.fFormat == SkMask::kA8_Format);
    SkASSERT(src.fBounds.contains(dst.fBounds));

    const uint8_t* srcP = src.getAddr8(src.fBounds.fLeft + dx, src.fBounds.fTop + dy);
    uint8_t* dstP = dst.getAddr8(dst.fBounds.fLeft, dst.fBounds.fTop);
    for (int y = 0; y < dst.fBounds.height(); ++y) {
        for (int x = 0; x < dst.fBounds.width(); ++x) {
            dstP[x] = convert_sdf_to_a8(srcP[x], window, offset);
        }
        srcP += src.fRowBytes;
        dstP += dst.fRowBytes;
    }
}

class SkDFMaskFilterImpl : public SkMaskFilterBase {
public:
    SkDFMaskFilterImpl(float window, float offset);

    SkMask::Format getFormat() const override { return SkMask::kA8_Format; }

    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

    void computeFastBounds(const SkRect&, SkRect*) const override;

    void toString(SkString* str) const override;
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDFMaskFilterImpl)

private:
    sk_sp<SkMaskFilter> fSDF;
    float fWindow;
    float fOffset;

    typedef SkMaskFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkDFMaskFilterImpl::SkDFMaskFilterImpl(float window, float offset) {
    fSDF = GrSDFMaskFilter::Make();
    fWindow = window;
    fOffset = offset;
}

bool SkDFMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                     const SkMatrix& matrix, SkIPoint* margin) const {
    if (src.fImage == nullptr) {
        *dst = src;
        dst->fFormat = SkMask::kA8_Format;
        return true;
    }

    SkMask sdf;
    if (!as_MFB(fSDF)->filterMask(&sdf, src, matrix, margin)) {
        return false;
    }

    *dst = src;
    dst->fImage = SkMask::AllocImage(src.computeImageSize());
    ConvertDFToA8(sdf,
                  src.fBounds.fLeft - sdf.fBounds.fLeft, src.fBounds.fTop - sdf.fBounds.fTop,
                  *dst, fWindow, fOffset);
    return true;
}

void SkDFMaskFilterImpl::computeFastBounds(const SkRect& src, SkRect* dst) const {
    *dst = src;
}

sk_sp<SkFlattenable> SkDFMaskFilterImpl::CreateProc(SkReadBuffer& buffer) {
    return SkDFMaskFilter::Make(1.0f, 0);
}

void SkDFMaskFilterImpl::toString(SkString* str) const {
    str->append("SkDFMaskFilterImpl: ()");
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkMaskFilter> SkDFMaskFilter::Make(float window, float offset) {
    return sk_sp<SkMaskFilter>(new SkDFMaskFilterImpl(window, offset));
}
