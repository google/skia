/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk2DPathEffect_DEFINED
#define Sk2DPathEffect_DEFINED

#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkMatrix.h"

class SK_API Sk2DPathEffect : public SkPathEffect {
public:
    Sk2DPathEffect(const SkMatrix& mat);

    virtual bool filterPath(SkPath*, const SkPath&,
                            SkStrokeRec*, const SkRect*) const SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(Sk2DPathEffect)

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
    Sk2DPathEffect(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

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
    SkLine2DPathEffect(SkScalar width, const SkMatrix& matrix)
    : Sk2DPathEffect(matrix), fWidth(width) {}

    virtual bool filterPath(SkPath* dst, const SkPath& src,
                            SkStrokeRec*, const SkRect*) const SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLine2DPathEffect)

protected:
    virtual void nextSpan(int u, int v, int ucount, SkPath*) const SK_OVERRIDE;

    SkLine2DPathEffect(SkFlattenableReadBuffer&);

    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    SkScalar fWidth;

    typedef Sk2DPathEffect INHERITED;
};

class SK_API SkPath2DPathEffect : public Sk2DPathEffect {
public:
    /**
     *  Stamp the specified path to fill the shape, using the matrix to define
     *  the latice.
     */
    SkPath2DPathEffect(const SkMatrix&, const SkPath&);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPath2DPathEffect)

protected:
    SkPath2DPathEffect(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    virtual void next(const SkPoint&, int u, int v, SkPath*) const SK_OVERRIDE;

private:
    SkPath  fPath;

    typedef Sk2DPathEffect INHERITED;
};

#endif
