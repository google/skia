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

class SkMatrixPE : public SkPathEffect {
public:
    SkMatrixPE(const SkMatrix&);

    bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

    Factory getFactory() const override { return CreateProc; }

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer&);
    friend class SkFlattenable::PrivateInitializer;

    SkMatrix    fMatrix;

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

#endif

