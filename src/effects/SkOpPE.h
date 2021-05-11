/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOpPE_DEFINED
#define SkOpPE_DEFINED

#include "include/effects/SkOpPathEffect.h"

class SkOpPE : public SkPathEffect {
public:
    SkOpPE(sk_sp<SkPathEffect> one, sk_sp<SkPathEffect> two, SkPathOp op);


protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

private:
    SK_FLATTENABLE_HOOKS(SkOpPE)

    bool computeFastBounds(SkRect* bounds) const override;

    sk_sp<SkPathEffect> fOne;
    sk_sp<SkPathEffect> fTwo;
    SkPathOp            fOp;

    using INHERITED = SkPathEffect;
};

class SkMatrixPE : public SkPathEffect {
public:
    SkMatrixPE(const SkMatrix&);

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

private:
    SK_FLATTENABLE_HOOKS(SkMatrixPE)

    bool computeFastBounds(SkRect* bounds) const override {
        if (bounds) {
            fMatrix.mapRect(bounds);
        }
        return true;
    }

    SkMatrix    fMatrix;

    using INHERITED = SkPathEffect;
};

class SkStrokePE : public SkPathEffect {
public:
    SkStrokePE(SkScalar width, SkPaint::Join, SkPaint::Cap, SkScalar miter);

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

private:
    SK_FLATTENABLE_HOOKS(SkStrokePE)

    bool computeFastBounds(SkRect* bounds) const override;

    SkScalar        fWidth,
                    fMiter;
    SkPaint::Join   fJoin;
    SkPaint::Cap    fCap;

    using INHERITED = SkPathEffect;
};

class SkStrokeAndFillPE : public SkPathEffect {
public:
    SkStrokeAndFillPE() {}

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

private:
    SK_FLATTENABLE_HOOKS(SkStrokeAndFillPE)

    bool computeFastBounds(SkRect* bounds) const override {
        // The effect's bounds depend on the StrokeRect that is not yet available
        return false;
    }

    using INHERITED = SkPathEffect;
};

#endif
