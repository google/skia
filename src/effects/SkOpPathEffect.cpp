/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStrokeRec.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/SkOpPE.h"

sk_sp<SkPathEffect> SkMergePathEffect::Make(sk_sp<SkPathEffect> one, sk_sp<SkPathEffect> two,
                                            SkPathOp op) {
    return sk_sp<SkPathEffect>(new SkOpPE(std::move(one), std::move(two), op));
}

SkOpPE::SkOpPE(sk_sp<SkPathEffect> one, sk_sp<SkPathEffect> two, SkPathOp op)
    : fOne(std::move(one)), fTwo(std::move(two)), fOp(op) {}

bool SkOpPE::onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                          const SkRect* cull) const {
    SkPath one, two;
    if (fOne) {
        if (!fOne->filterPath(&one, src, rec, cull)) {
            return false;
        }
    } else {
        one = src;
    }
    if (fTwo) {
        if (!fTwo->filterPath(&two, src, rec, cull)) {
            return false;
        }
    } else {
        two = src;
    }
    return Op(one, two, fOp, dst);
}

void SkOpPE::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fOne.get());
    buffer.writeFlattenable(fTwo.get());
    buffer.write32(fOp);
}

sk_sp<SkFlattenable> SkOpPE::CreateProc(SkReadBuffer& buffer) {
    auto one = buffer.readPathEffect();
    auto two = buffer.readPathEffect();
    SkPathOp op = buffer.read32LE(kReverseDifference_SkPathOp);
    return buffer.isValid() ? SkMergePathEffect::Make(std::move(one), std::move(two), op) : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkMatrixPathEffect::MakeTranslate(SkScalar dx, SkScalar dy) {
    if (!SkScalarsAreFinite(dx, dy)) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkMatrixPE(SkMatrix::MakeTrans(dx, dy)));
}

sk_sp<SkPathEffect> SkMatrixPathEffect::Make(const SkMatrix& matrix) {
    if (!matrix.isFinite()) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkMatrixPE(matrix));
}

SkMatrixPE::SkMatrixPE(const SkMatrix& matrix) : fMatrix(matrix) {
    SkASSERT(matrix.isFinite());
}

bool SkMatrixPE::onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const {
    src.transform(fMatrix, dst);
    return true;
}

void SkMatrixPE::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(fMatrix);
}

sk_sp<SkFlattenable> SkMatrixPE::CreateProc(SkReadBuffer& buffer) {
    SkMatrix mx;
    buffer.readMatrix(&mx);
    return buffer.isValid() ? SkMatrixPathEffect::Make(mx) : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkStrokePathEffect::Make(SkScalar width, SkPaint::Join join, SkPaint::Cap cap,
                                             SkScalar miter) {
    if (!SkScalarsAreFinite(width, miter) || width < 0 || miter < 0) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkStrokePE(width, join, cap, miter));
}

SkStrokePE::SkStrokePE(SkScalar width, SkPaint::Join join, SkPaint::Cap cap, SkScalar miter)
    : fWidth(width), fMiter(miter), fJoin(join), fCap(cap) {}

bool SkStrokePE::onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const {
    SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
    rec.setStrokeStyle(fWidth);
    rec.setStrokeParams(fCap, fJoin, fMiter);
    return rec.applyToPath(dst, src);
}

void SkStrokePE::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fWidth);
    buffer.writeScalar(fMiter);
    buffer.write32(fJoin);
    buffer.write32(fCap);
}

sk_sp<SkFlattenable> SkStrokePE::CreateProc(SkReadBuffer& buffer) {
    SkScalar width = buffer.readScalar();
    SkScalar miter = buffer.readScalar();
    SkPaint::Join join = buffer.read32LE(SkPaint::kLast_Join);
    SkPaint::Cap cap = buffer.read32LE(SkPaint::kLast_Cap);
    return buffer.isValid() ? SkStrokePathEffect::Make(width, join, cap, miter) : nullptr;
}


