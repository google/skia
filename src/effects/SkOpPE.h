/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOpPE_DEFINED
#define SkOpPE_DEFINED

#include "include/pathops/SkPathOps.h"
#include "src/core/SkPathEffectBase.h"

class SkOpPE : public SkPathEffectBase {
public:
    SkOpPE(sk_sp<SkPathEffect> one, sk_sp<SkPathEffect> two, SkPathOp op);


protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                      const SkMatrix&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkOpPE)

    bool computeFastBounds(SkRect* bounds) const override;

    sk_sp<SkPathEffect> fOne;
    sk_sp<SkPathEffect> fTwo;
    SkPathOp            fOp;

    using INHERITED = SkPathEffectBase;
};

class SkMatrixPE : public SkPathEffectBase {
public:
    SkMatrixPE(const SkMatrix&);

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                      const SkMatrix&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkMatrixPE)

    bool computeFastBounds(SkRect* bounds) const override {
        if (bounds) {
            fMatrix.mapRect(bounds);
        }
        return true;
    }

    SkMatrix    fMatrix;

    using INHERITED = SkPathEffectBase;
};

class SkStrokePE : public SkPathEffectBase {
public:
    SkStrokePE(SkScalar width, SkPaint::Join, SkPaint::Cap, SkScalar miter);

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                      const SkMatrix&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkStrokePE)

    bool computeFastBounds(SkRect* bounds) const override;

    SkScalar        fWidth,
                    fMiter;
    SkPaint::Join   fJoin;
    SkPaint::Cap    fCap;

    using INHERITED = SkPathEffectBase;
};

class SkStrokeAndFillPE : public SkPathEffectBase {
public:
    SkStrokeAndFillPE() {}

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                      const SkMatrix&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkStrokeAndFillPE)

    bool computeFastBounds(SkRect* bounds) const override {
        // The effect's bounds depend on the StrokeRect that is not yet available
        return false;
    }

    using INHERITED = SkPathEffectBase;
};

#endif
