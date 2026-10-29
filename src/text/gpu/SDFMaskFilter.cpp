/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/SDFMaskFilter.h"

#include "include/core/SkFlattenable.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"

class SkMatrix;
class SkReadBuffer;

#if !defined(SK_DISABLE_SDF_TEXT)

namespace sktext::gpu {

class SDFMaskFilterImpl : public SkMaskFilterBase {
public:
    SDFMaskFilterImpl();

    // overrides from SkMaskFilterBase
    //  This method is not exported to java.
    SkMask::Format getFormat() const override;
    //  This method is not exported to java.
    bool filterMask(SkMaskBuilder* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;
    SkMaskFilterBase::Type type() const override { return SkMaskFilterBase::Type::kSDF; }
    void computeFastBounds(const SkRect&, SkRect*) const override;

private:
    SK_FLATTENABLE_HOOKS(SDFMaskFilterImpl)
};

///////////////////////////////////////////////////////////////////////////////

SDFMaskFilterImpl::SDFMaskFilterImpl() {}

SkMask::Format SDFMaskFilterImpl::getFormat() const {
    return SkMask::kSDF_Format;
}

bool SDFMaskFilterImpl::filterMask(SkMaskBuilder* dst, const SkMask& src,
                                   const SkMatrix& matrix, SkIPoint* margin) const {
    if (src.fFormat != SkMask::kA8_Format
        && src.fFormat != SkMask::kBW_Format
        && src.fFormat != SkMask::kLCD16_Format) {
        return false;
    }
    // Reject dimensions whose distance field or scratch storage cannot be allocated
    if (SkComputeDistanceFieldSize(src.fBounds.width(), src.fBounds.height()) == 0) {
        dst->image() = nullptr;
        dst->bounds().setEmpty();
        return false;
    }

    *dst = SkMaskBuilder::PrepareDestination(SK_DistanceFieldPad, SK_DistanceFieldPad, src);
    dst->format() = SkMask::kSDF_Format;

    if (margin) {
        margin->set(SK_DistanceFieldPad, SK_DistanceFieldPad);
    }

    if (src.fImage == nullptr) {
        return true;
    }
    if (dst->fImage == nullptr) {
        dst->bounds().setEmpty();
        return false;
    }

    bool success = false;
    if (src.fFormat == SkMask::kA8_Format) {
        success = SkGenerateDistanceFieldFromA8Image(
                dst->image(), src.fImage, src.fBounds.width(), src.fBounds.height(), src.fRowBytes);
    } else if (src.fFormat == SkMask::kLCD16_Format) {
        success = SkGenerateDistanceFieldFromLCD16Mask(
                dst->image(), src.fImage, src.fBounds.width(), src.fBounds.height(), src.fRowBytes);
    } else {
        success = SkGenerateDistanceFieldFromBWImage(
                dst->image(), src.fImage, src.fBounds.width(), src.fBounds.height(), src.fRowBytes);
    }

    // Release the destination buffer when distance field generation fails
    if (!success) {
        SkMaskBuilder::FreeImage(dst->image());
        dst->image() = nullptr;
        dst->bounds().setEmpty();
        return false;
    }
    return true;
}

void SDFMaskFilterImpl::computeFastBounds(const SkRect& src,
                                            SkRect* dst) const {
    dst->setLTRB(src.fLeft  - SK_DistanceFieldPad, src.fTop    - SK_DistanceFieldPad,
                 src.fRight + SK_DistanceFieldPad, src.fBottom + SK_DistanceFieldPad);
}

sk_sp<SkFlattenable> SDFMaskFilterImpl::CreateProc(SkReadBuffer& buffer) {
    return SDFMaskFilter::Make();
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkMaskFilter> SDFMaskFilter::Make() {
    return sk_sp<SkMaskFilter>(new SDFMaskFilterImpl());
}

}  // namespace sktext::gpu

#endif // !defined(SK_DISABLE_SDF_TEXT)
