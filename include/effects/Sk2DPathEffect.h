/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk2DPathEffect_DEFINED
#define Sk2DPathEffect_DEFINED

#include "SkFlattenable.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkMatrix.h"

class SK_API Sk2DPathEffect : public SkPathEffect {
protected:
    /** New virtual, to be overridden by subclasses.
        This is called once from filterPath, and provides the
        uv parameter bounds for the path. Subsequent calls to
        next() will receive u and v values within these bounds,
        and then a call to end() will signal the end of processing.
    */
    virtual void begin(const SkIRect& uvBounds, SkPath* dst) const;
    virtual void next(const SkPoint& loc, int u, int v, SkPath* dst) const;
    virtual void end(SkPath* dst) const;

    /** Low-level virtual called per span of locations in the u-direction.
        The default implementation calls next() repeatedly with each
        location.
    */
    virtual void nextSpan(int u, int v, int ucount, SkPath* dst) const;

    const SkMatrix& getMatrix() const { return fMatrix; }

    // protected so that subclasses can call this during unflattening
    explicit Sk2DPathEffect(const SkMatrix& mat);
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath*, const SkPath&, SkStrokeRec*, const SkRect*) const override;

private:
    SkMatrix    fMatrix, fInverse;
    bool        fMatrixIsInvertible;

    // illegal
    Sk2DPathEffect(const Sk2DPathEffect&);
    Sk2DPathEffect& operator=(const Sk2DPathEffect&);

    friend class Sk2DPathEffectBlitter;
    typedef SkPathEffect INHERITED;
};

class SK_API SkLine2DPathEffect : public Sk2DPathEffect {
public:
    static sk_sp<SkPathEffect> Make(SkScalar width, const SkMatrix& matrix) {
        if (!(width >= 0)) {
            return nullptr;
        }
        return sk_sp<SkPathEffect>(new SkLine2DPathEffect(width, matrix));
    }


protected:
    SkLine2DPathEffect(SkScalar width, const SkMatrix& matrix)
        : Sk2DPathEffect(matrix), fWidth(width) {
            SkASSERT(width >= 0);
        }
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

    void nextSpan(int u, int v, int ucount, SkPath*) const override;

private:
    SK_FLATTENABLE_HOOKS(SkLine2DPathEffect)

    SkScalar fWidth;

    typedef Sk2DPathEffect INHERITED;
};

class SK_API SkPath2DPathEffect : public Sk2DPathEffect {
public:
    /**
     *  Stamp the specified path to fill the shape, using the matrix to define
     *  the latice.
     */
    static sk_sp<SkPathEffect> Make(const SkMatrix& matrix, const SkPath& path) {
        return sk_sp<SkPathEffect>(new SkPath2DPathEffect(matrix, path));
    }

protected:
    SkPath2DPathEffect(const SkMatrix&, const SkPath&);
    void flatten(SkWriteBuffer&) const override;

    void next(const SkPoint&, int u, int v, SkPath*) const override;

private:
    SK_FLATTENABLE_HOOKS(SkPath2DPathEffect)

    SkPath  fPath;

    typedef Sk2DPathEffect INHERITED;
};

#endif
