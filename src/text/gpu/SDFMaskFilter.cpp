/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/gpu/SDFMaskFilter.h"

#if !defined(SK_DISABLE_SDF_TEXT)

namespace sktext::gpu {

class SDFMaskFilterImpl : public SkMaskFilterBase {
public:
    SDFMaskFilterImpl();

    // overrides from SkMaskFilterBase
    //  This method is not exported to java.
    SkMask::Format getFormat() const override;
    //  This method is not exported to java.
    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

    void computeFastBounds(const SkRect&, SkRect*) const override;

protected:

private:
    SK_FLATTENABLE_HOOKS(SDFMaskFilterImpl)

    using INHERITED = SkMaskFilter;
    friend void register_sdf_maskfilter_createproc();
};

///////////////////////////////////////////////////////////////////////////////

SDFMaskFilterImpl::SDFMaskFilterImpl() {}

SkMask::Format SDFMaskFilterImpl::getFormat() const {
    return SkMask::kSDF_Format;
}

bool SDFMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                     const SkMatrix& matrix, SkIPoint* margin) const {
    if (src.fFormat != SkMask::kA8_Format
        && src.fFormat != SkMask::kBW_Format
        && src.fFormat != SkMask::kLCD16_Format) {
        return false;
    }

    *dst = SkMask::PrepareDestination(SK_DistanceFieldPad, SK_DistanceFieldPad, src);
    dst->fFormat = SkMask::kSDF_Format;

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
    } else if (src.fFormat == SkMask::kLCD16_Format) {
        return SkGenerateDistanceFieldFromLCD16Mask(dst->fImage, src.fImage,
                                                     src.fBounds.width(), src.fBounds.height(),
                                                     src.fRowBytes);
    } else {
        return SkGenerateDistanceFieldFromBWImage(dst->fImage, src.fImage,
                                                  src.fBounds.width(), src.fBounds.height(),
                                                  src.fRowBytes);
    }
}

void SDFMaskFilterImpl::computeFastBounds(const SkRect& src,
                                            SkRect* dst) const {
    dst->setLTRB(src.fLeft  - SK_DistanceFieldPad, src.fTop    - SK_DistanceFieldPad,
                 src.fRight + SK_DistanceFieldPad, src.fBottom + SK_DistanceFieldPad);
}

sk_sp<SkFlattenable> SDFMaskFilterImpl::CreateProc(SkReadBuffer& buffer) {
    return SDFMaskFilter::Make();
}

void register_sdf_maskfilter_createproc() { SK_REGISTER_FLATTENABLE(SDFMaskFilterImpl); }

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkMaskFilter> SDFMaskFilter::Make() {
    return sk_sp<SkMaskFilter>(new SDFMaskFilterImpl());
}

}  // namespace sktext::gpu

#endif // !defined(SK_DISABLE_SDF_TEXT)
