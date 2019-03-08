// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkPenPathEffectImpl.h"
#include "SkPenPathEffect.h"
#include "SkStrokeRec.h"
#include "SkReadBuffer.h"

SkPenPathEffect::SkPenPathEffect(const SkMatrix& matrix) : fMat(matrix) {}

SkPenPathEffect::~SkPenPathEffect() = default;

SkRect SkPenPathEffect::onComputeFastBounds(const SkRect& src) const {
    return src.makeOutset(fabsf(fMat[0]) + fabsf(fMat[1]) + fabsf(fMat[2]),
                          fabsf(fMat[3]) + fabsf(fMat[4]) + fabsf(fMat[5]));
}

bool SkPenPathEffect::onFilterPath(SkPath* dst,
                                   const SkPath& src,
                                   SkStrokeRec* strokeRec,
                                   const SkRect* cullRect) const {
    SkASSERT(strokeRec);
    SkMatrix inverse;
    *dst = src;
    if (strokeRec->getStyle() != SkStrokeRec::kFill_Style && fMat.invert(&inverse)) {
        dst->transform(inverse);
        strokeRec->setStrokeStyle(1, strokeRec->getStyle() == SkStrokeRec::kStrokeAndFill_Style);
        strokeRec->applyToPath(dst, *dst);
        dst->transform(fMat);
        *strokeRec = SkStrokeRec(SkStrokeRec::kFill_InitStyle);
    }
    return true;
}

void SkPenPathEffect::flatten(SkWriteBuffer& buffer) const { buffer.writeMatrix(fMat); }

sk_sp<SkFlattenable> SkPenPathEffect::CreateProc(SkReadBuffer& buffer) {
    SkMatrix matrix;
    buffer.readMatrix(&matrix);
    return buffer.isValid() ? sk_make_sp<SkPenPathEffect>(matrix) : nullptr;
}

sk_sp<SkPathEffect> SkMakePenPathEffect(const SkMatrix& matrix) {
    return sk_make_sp<SkPenPathEffect>(matrix);
}
