/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSDFMaskFilter.h"
#include "SkDistanceFieldGen.h"
#include "SkReadBuffer.h"
#include "SkSafeMath.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

sk_sp<SkMaskFilter> SkSDFMaskFilter::Make() {
    return sk_sp<SkMaskFilter>(new SkSDFMaskFilter());
}

///////////////////////////////////////////////////////////////////////////////

SkSDFMaskFilter::SkSDFMaskFilter() {}

SkMask::Format SkSDFMaskFilter::getFormat() const {
    return SkMask::kA8_Format;
}

bool SkSDFMaskFilter::filterMask(SkMask* dst, const SkMask& src,
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

sk_sp<SkFlattenable> SkSDFMaskFilter::CreateProc(SkReadBuffer& buffer) {
    return Make();
}

void SkSDFMaskFilter::flatten(SkWriteBuffer& buffer) const {
    // nothing to write
}

void SkSDFMaskFilter::toString(SkString* str) const {
    str->append("SkSDFMaskFilter: ()");
}
