// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkPenPathEffectImpl.h"
#include "SkPenPathEffect.h"
#include "SkStrokeRec.h"
#include "SkReadBuffer.h"

SkPenPathEffect::SkPenPathEffect(SkScalar a, SkScalar b, SkScalar c, SkScalar d)
    : fA(a), fB(b), fC(c), fD(d), fW(1), fX(0), fY(0), fZ(1) {
    SkMatrix matrix = SkMatrix::MakeAll(a, b, 0, c, d, 0, 0, 0, 1);
    SkMatrix inverse = SkMatrix::I();
    if (matrix.invert(&inverse)) {
        SkASSERT(inverse[SkMatrix::kMTransX] == 0);
        SkASSERT(inverse[SkMatrix::kMTransY] == 0);
        SkASSERT(inverse[SkMatrix::kMPersp0] == 0);
        SkASSERT(inverse[SkMatrix::kMPersp1] == 0);
        SkASSERT(inverse[SkMatrix::kMPersp2] == 1);
        fW = inverse[SkMatrix::kMScaleX];
        fX = inverse[SkMatrix::kMSkewX];
        fY = inverse[SkMatrix::kMSkewY];
        fZ = inverse[SkMatrix::kMScaleY];
    } else {
        fA = fD = 1;
        fB = fC = 0;
    }
}

SkPenPathEffect::~SkPenPathEffect() = default;

SkRect SkPenPathEffect::onComputeFastBounds(const SkRect& src) const {
    return src.makeOutset(fA + fB, fC + fD);
}

bool SkPenPathEffect::onFilterPath(SkPath* dst,
                                   const SkPath& src,
                                   SkStrokeRec* strokeRec,
                                   const SkRect* cullRect) const {
    SkASSERT(strokeRec);
    SkMatrix matrix = SkMatrix::MakeAll(fA, fB, 0, fC, fD, 0, 0, 0, 1);
    SkMatrix inverse = SkMatrix::MakeAll(fW, fX, 0, fY, fZ, 0, 0, 0, 1);
    *dst = src;
    if (strokeRec->getStyle() != SkStrokeRec::kFill_Style) {
        dst->transform(inverse);
        strokeRec->setStrokeStyle(1, strokeRec->getStyle() == SkStrokeRec::kStrokeAndFill_Style);
        strokeRec->applyToPath(dst, *dst);
        dst->transform(matrix);
        *strokeRec = SkStrokeRec(SkStrokeRec::kFill_InitStyle);
    }
    return true;
}

void SkPenPathEffect::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fA);
    buffer.writeScalar(fB);
    buffer.writeScalar(fC);
    buffer.writeScalar(fD);
}

sk_sp<SkFlattenable> SkPenPathEffect::CreateProc(SkReadBuffer& buffer) {
    SkScalar a = buffer.readScalar();
    SkScalar b = buffer.readScalar();
    SkScalar c = buffer.readScalar();
    SkScalar d = buffer.readScalar();
    return buffer.isValid() ? sk_sp<SkFlattenable>(new SkPenPathEffect(a, b, c, d)): nullptr;
}

sk_sp<SkPathEffect> SkMakePenPathEffect(SkScalar a, SkScalar b, SkScalar c, SkScalar d) {
    return sk_make_sp<SkPenPathEffect>(a, b, c, d);
}
