/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOffsetPathEffect.h"
#include "SkPaint.h"
#include "SkPathOps.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

class SkOffsetPE : public SkPathEffect {
public:
    SkOffsetPE(SkScalar offset, SkPaint::Join jt, SkScalar miter)
        : fOffset(offset), fMiter(miter), fJoin(jt)
    {
        SkASSERT(offset != 0 && SkScalarIsFinite(offset));
        SkASSERT(miter >= 0 && SkScalarIsFinite(miter));
    }

    bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

    Factory getFactory() const override { return CreateProc; }

protected:
    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer&);
//    friend class SkFlattenable::PrivateInitializer;

    void flatten(SkWriteBuffer&) const override;

private:
    SkScalar        fOffset, fMiter;
    SkPaint::Join   fJoin;

    typedef SkPathEffect INHERITED;
};

bool SkOffsetPE::filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SkScalarAbs(fOffset));
    paint.setStrokeJoin(fJoin);
    paint.setStrokeMiter(fMiter);

    SkPath stroke;
    paint.getFillPath(src, &stroke, nullptr);

    return Op(src, stroke, fOffset > 0 ? kUnion_SkPathOp : kDifference_SkPathOp, dst);
}

void SkOffsetPE::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fOffset);
    buffer.writeScalar(fMiter);
    buffer.writeInt(fJoin);
}

sk_sp<SkFlattenable> SkOffsetPE::CreateProc(SkReadBuffer& buffer) {
    SkScalar offset  = buffer.readScalar();
    SkScalar miter   = buffer.readScalar();
    SkPaint::Join jt = buffer.read32LE(SkPaint::kLast_Join);
    return SkOffsetPathEffect::Make(offset, jt, miter);
}

sk_sp<SkPathEffect> SkOffsetPathEffect::Make(SkScalar offset, SkPaint::Join jt, SkScalar miter) {
    if (offset == 0 || SkScalarIsFinite(offset)) {
        return nullptr;
    }
    if (miter < 0 || SkScalarIsFinite(miter)) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkOffsetPE(offset, jt, miter));
}

