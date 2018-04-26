/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSDFMaskFilter.h"
#include "SkDistanceFieldGen.h"
#include "SkMaskFilterBase.h"
#include "SkReadBuffer.h"
#include "SkSafeMath.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

class SK_API SkSDFMaskFilterImpl : public SkMaskFilterBase {
public:
    SkSDFMaskFilterImpl();

    // overrides from SkMaskFilterBase
    //  This method is not exported to java.
    SkMask::Format getFormat() const override;
    //  This method is not exported to java.
    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

    void toString(SkString* str) const override;
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSDFMaskFilterImpl)

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    typedef SkMaskFilterBase INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkSDFMaskFilterImpl::SkSDFMaskFilterImpl() {}

SkMask::Format SkSDFMaskFilterImpl::getFormat() const {
    return SkMask::kA8_Format;
}

bool SkSDFMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                 const SkMatrix& matrix, SkIPoint* margin) const {
    if (src.fFormat != SkMask::kA8_Format && src.fFormat != SkMask::kBW_Format) {
        return false;
    }

    *dst = SkMask::PrepareDestination(SK_DistanceFieldPad, SK_DistanceFieldPad, src);

    if (margin) {
        margin->set(SK_DistanceFieldPad, SK_DistanceFieldPad);
    }

    if (src.fImage == nullptr) {
        return true;
    }
    if (dst->fImage == nullptr) {
        dst->fBounds.setEmpty();
        return false;
    }

    if (src.fFormat == SkMask::kA8_Format) {
        return SkGenerateDistanceFieldFromA8Image(dst->fImage, src.fImage,
                                                  src.fBounds.width(), src.fBounds.height(),
                                                  src.fRowBytes);

    } else {
        return SkGenerateDistanceFieldFromBWImage(dst->fImage, src.fImage,
                                                  src.fBounds.width(), src.fBounds.height(),
                                                  src.fRowBytes);
    }
}

sk_sp<SkFlattenable> SkSDFMaskFilterImpl::CreateProc(SkReadBuffer& buffer) {
    return SkSDFMaskFilter::Make();
}

void SkSDFMaskFilterImpl::flatten(SkWriteBuffer& buffer) const {
    // nothing to write
}

void SkSDFMaskFilterImpl::toString(SkString* str) const {
    str->append("SkSDFMaskFilterImpl: ()");
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkMaskFilter> SkSDFMaskFilter::Make() {
    return sk_sp<SkMaskFilter>(new SkSDFMaskFilterImpl());
}
