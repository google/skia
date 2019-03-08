// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkPenPathEffectImpl.h"
#include "SkPenPathEffect.h"
#include "SkStrokeRec.h"
#include "SkReadBuffer.h"

SkPenPathEffect::SkPenPathEffect(SkScalar a, SkScalar b, SkScalar c, SkScalar d)
    : fMat{a, b, c, d} {}

SkPenPathEffect::~SkPenPathEffect() = default;

SkRect SkPenPathEffect::onComputeFastBounds(const SkRect& src) const {
    return src.makeOutset(fMat[0] + fMat[1], fMat[2] + fMat[3]);
}

bool SkPenPathEffect::onFilterPath(SkPath* dst,
                                   const SkPath& src,
                                   SkStrokeRec* strokeRec,
                                   const SkRect* cullRect) const {
    SkASSERT(strokeRec);
    SkMatrix matrix = SkMatrix::MakeAll(fMat[0], fMat[1], 0, fMat[2], fMat[3], 0, 0, 0, 1);
    SkMatrix inverse;
    *dst = src;
    if (strokeRec->getStyle() != SkStrokeRec::kFill_Style && matrix.invert(&inverse)) {
        dst->transform(inverse);
        strokeRec->setStrokeStyle(1, strokeRec->getStyle() == SkStrokeRec::kStrokeAndFill_Style);
        strokeRec->applyToPath(dst, *dst);
        dst->transform(matrix);
        *strokeRec = SkStrokeRec(SkStrokeRec::kFill_InitStyle);
    }
    return true;
}

void SkPenPathEffect::flatten(SkWriteBuffer& buffer) const {
    for (float v : fMat) {
        buffer.writeScalar(v);
    }
}

sk_sp<SkFlattenable> SkPenPathEffect::CreateProc(SkReadBuffer& buffer) {
    SkScalar a = buffer.readScalar();
    SkScalar b = buffer.readScalar();
    SkScalar c = buffer.readScalar();
    SkScalar d = buffer.readScalar();
    return buffer.isValid() ? sk_make_sp<SkPenPathEffect>(a, b, c, d) : nullptr;
}

sk_sp<SkPathEffect> SkMakePenPathEffect(SkScalar a, SkScalar b, SkScalar c, SkScalar d) {
    return sk_make_sp<SkPenPathEffect>(a, b, c, d);
}
