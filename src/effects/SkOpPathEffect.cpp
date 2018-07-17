/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpPathEffect.h"
#include "SkStrokeRec.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

class SkOpPE : public SkPathEffect {
public:
    SkOpPE(sk_sp<SkPathEffect> one, sk_sp<SkPathEffect> two, SkPathOp op);

    bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

    Factory getFactory() const override { return CreateProc; }

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer&);
    friend class SkFlattenable::PrivateInitializer;

    sk_sp<SkPathEffect> fOne;
    sk_sp<SkPathEffect> fTwo;
    SkPathOp            fOp;

    typedef SkPathEffect INHERITED;
};

class SkTranslatePE : public SkPathEffect {
public:
    SkTranslatePE(SkScalar dx, SkScalar dy);

    bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

    Factory getFactory() const override { return CreateProc; }

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer&);
    friend class SkFlattenable::PrivateInitializer;

    SkVector    fTranslate;

    typedef SkPathEffect INHERITED;
};

class SkStrokePE : public SkPathEffect {
public:
    SkStrokePE(SkScalar width, SkPaint::Join, SkPaint::Cap, SkScalar miter);

    bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

    Factory getFactory() const override { return CreateProc; }

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer&);
    friend class SkFlattenable::PrivateInitializer;

    SkScalar        fWidth,
                    fMiter;
    SkPaint::Join   fJoin;
    SkPaint::Cap    fCap;

    typedef SkPathEffect INHERITED;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkOpPathEffect::Make(sk_sp<SkPathEffect> one, sk_sp<SkPathEffect> two,
                                         SkPathOp op) {
    return sk_sp<SkPathEffect>(new SkOpPE(std::move(one), std::move(two), op));
}

SkOpPE::SkOpPE(sk_sp<SkPathEffect> one, sk_sp<SkPathEffect> two, SkPathOp op)
    : fOne(std::move(one)), fTwo(std::move(two)), fOp(op) {}

bool SkOpPE::filterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                        const SkRect* cull) const {
    SkPath one, two;
    if (fOne) {
        if (!fOne->filterPath(&one, src, rec, cull)) {
            return false;
        } else {
            one = src;
        }
    }
    if (fTwo) {
        if (!fTwo->filterPath(&one, src, rec, cull)) {
            return false;
        } else {
            two = src;
        }
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
    return buffer.isValid() ? SkOpPathEffect::Make(std::move(one), std::move(two), op) : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkTranslatePathEffect::Make(SkScalar dx, SkScalar dy) {
    if (!SkScalarsAreFinite(dx, dy)) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkTranslatePE(dx, dy));
}

SkTranslatePE::SkTranslatePE(SkScalar dx, SkScalar dy) : fTranslate{dx, dy} {
    SkASSERT(SkScalarsAreFinite(dx, dy));
}

bool SkTranslatePE::filterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                               const SkRect* cull) const {
    src.offset(fTranslate.fX, fTranslate.fY, dst);
    return true;
}

void SkTranslatePE::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fTranslate.fX);
    buffer.writeScalar(fTranslate.fY);
}

sk_sp<SkFlattenable> SkTranslatePE::CreateProc(SkReadBuffer& buffer) {
    SkScalar dx = buffer.readScalar();
    SkScalar dy = buffer.readScalar();
    return buffer.isValid() ? SkTranslatePathEffect::Make(dx, dy) : nullptr;
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

bool SkStrokePE::filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const {
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
    auto two = buffer.readPathEffect();
    SkPaint::Join join = buffer.read32LE(SkPaint::kLast_Join);
    SkPaint::Cap cap = buffer.read32LE(SkPaint::kLast_Cap);
    return buffer.isValid() ? SkStrokePathEffect::Make(width, join, cap, miter) : nullptr;
}


