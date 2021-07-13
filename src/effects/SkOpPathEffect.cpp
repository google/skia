/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStrokeRec.h"
#include "include/effects/SkOpPathEffect.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/SkOpPE.h"

sk_sp<SkPathEffect> SkMergePathEffect::Make(sk_sp<SkPathEffect> one, sk_sp<SkPathEffect> two,
                                            SkPathOp op) {
    return sk_sp<SkPathEffect>(new SkOpPE(std::move(one), std::move(two), op));
}

SkOpPE::SkOpPE(sk_sp<SkPathEffect> one, sk_sp<SkPathEffect> two, SkPathOp op)
    : fOne(std::move(one)), fTwo(std::move(two)), fOp(op) {}

bool SkOpPE::onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                          const SkRect* cull, const SkMatrix& ctm) const {
    SkPath one, two;
    if (fOne) {
        if (!fOne->filterPath(&one, src, rec, cull, ctm)) {
            return false;
        }
    } else {
        one = src;
    }
    if (fTwo) {
        if (!fTwo->filterPath(&two, src, rec, cull, ctm)) {
            return false;
        }
    } else {
        two = src;
    }
    return Op(one, two, fOp, dst);
}

bool SkOpPE::computeFastBounds(SkRect* bounds) const {
    if (!bounds) {
        return (!SkToBool(fOne) || as_PEB(fOne)->computeFastBounds(nullptr)) &&
               (!SkToBool(fTwo) || as_PEB(fTwo)->computeFastBounds(nullptr));
    }

    // bounds will hold the result of the fOne while b2 holds the result of fTwo's fast bounds
    SkRect b2 = *bounds;
    if (fOne && !as_PEB(fOne)->computeFastBounds(bounds)) {
        return false;
    }
    if (fTwo && !as_PEB(fTwo)->computeFastBounds(&b2)) {
        return false;
    }

    switch (fOp) {
        case SkPathOp::kIntersect_SkPathOp:
            if (!bounds->intersect(b2)) {
                bounds->setEmpty();
            }
            break;
        case SkPathOp::kDifference_SkPathOp:
            // (one - two) conservatively leaves one's bounds unmodified
            break;
        case SkPathOp::kReverseDifference_SkPathOp:
            // (two - one) conservatively leaves two's bounds unmodified
            *bounds = b2;
            break;
        case SkPathOp::kXOR_SkPathOp:
            // fall through to union since XOR computes a subset of regular OR
        case SkPathOp::kUnion_SkPathOp:
            bounds->join(b2);
            break;
    }

    return true;
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
    return sk_sp<SkPathEffect>(new SkMatrixPE(SkMatrix::Translate(dx, dy)));
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

bool SkMatrixPE::onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                              const SkMatrix&) const {
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

bool SkStrokePE::onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                              const SkMatrix&) const {
    SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
    rec.setStrokeStyle(fWidth);
    rec.setStrokeParams(fCap, fJoin, fMiter);
    return rec.applyToPath(dst, src);
}

bool SkStrokePE::computeFastBounds(SkRect* bounds) const {
    if (bounds) {
        SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
        rec.setStrokeStyle(fWidth);
        rec.setStrokeParams(fCap, fJoin, fMiter);
        bounds->outset(rec.getInflationRadius(), rec.getInflationRadius());
    }
    return true;
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

//////////////////////////////////////////////////////////////////////////////////////////////////

#include "include/effects/SkStrokeAndFillPathEffect.h"
#include "src/core/SkPathPriv.h"

sk_sp<SkPathEffect> SkStrokeAndFillPathEffect::Make() {
    static SkPathEffect* strokeAndFill = new SkStrokeAndFillPE;
    return sk_ref_sp(strokeAndFill);
}

void SkStrokeAndFillPE::flatten(SkWriteBuffer&) const {}

static bool known_to_be_opposite_directions(const SkPath& a, const SkPath& b) {
    auto a_dir = SkPathPriv::ComputeFirstDirection(a),
         b_dir = SkPathPriv::ComputeFirstDirection(b);

    return (a_dir == SkPathFirstDirection::kCCW &&
            b_dir == SkPathFirstDirection::kCW)
            ||
           (a_dir == SkPathFirstDirection::kCW &&
            b_dir == SkPathFirstDirection::kCCW);
}

bool SkStrokeAndFillPE::onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                                     const SkRect*, const SkMatrix&) const {
    // This one is weird, since we exist to allow this paint-style to go away. If we see it,
    // just let the normal machine run its course.
    if (rec->getStyle() == SkStrokeRec::kStrokeAndFill_Style) {
        *dst = src;
        return true;
    }

    if (rec->getStyle() == SkStrokeRec::kStroke_Style) {
        if (!rec->applyToPath(dst, src)) {
            return false;
        }

        if (known_to_be_opposite_directions(src, *dst)) {
            dst->reverseAddPath(src);
        } else {
            dst->addPath(src);
        }
    } else {
        *dst = src;
    }
    rec->setFillStyle();
    return true;
}

sk_sp<SkFlattenable> SkStrokeAndFillPE::CreateProc(SkReadBuffer& buffer) {
    return SkStrokeAndFillPathEffect::Make();
}
