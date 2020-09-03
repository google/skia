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

    SkMatrix    fMatrix;

    using INHERITED = SkPathEffect;
};

class SkStrokePE : public SkPathEffect {
public:
    SkStrokePE(SkScalar width, SkPaint::Join, SkPaint::Cap, SkScalar miter);

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;
    // TODO: override onComputeFastBounds (I think)

private:
    SK_FLATTENABLE_HOOKS(SkStrokePE)

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
    // TODO: override onComputeFastBounds (I think)

private:
    SK_FLATTENABLE_HOOKS(SkStrokeAndFillPE)

    using INHERITED = SkPathEffect;
};

#endif

