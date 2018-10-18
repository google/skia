/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOpPE_DEFINED
#define SkOpPE_DEFINED

#include "SkOpPathEffect.h"

class SkOpPE : public SkPathEffect {
public:
    SkOpPE(sk_sp<SkPathEffect> one, sk_sp<SkPathEffect> two, SkPathOp op);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkOpPE);

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

private:
    sk_sp<SkPathEffect> fOne;
    sk_sp<SkPathEffect> fTwo;
    SkPathOp            fOp;

    typedef SkPathEffect INHERITED;
};

class SkMatrixPE : public SkPathEffect {
public:
    SkMatrixPE(const SkMatrix&);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMatrixPE);

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

private:
    SkMatrix    fMatrix;

    typedef SkPathEffect INHERITED;
};

class SkStrokePE : public SkPathEffect {
public:
    SkStrokePE(SkScalar width, SkPaint::Join, SkPaint::Cap, SkScalar miter);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkStrokePE);

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;
    // TODO: override onComputeFastBounds (I think)

private:
    SkScalar        fWidth,
                    fMiter;
    SkPaint::Join   fJoin;
    SkPaint::Cap    fCap;

    typedef SkPathEffect INHERITED;
};

#endif

