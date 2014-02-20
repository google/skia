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
    static Sk2DPathEffect* Create(const SkMatrix& mat) {
        return SkNEW_ARGS(Sk2DPathEffect, (mat));
    }

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
    Sk2DPathEffect(SkReadBuffer&);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

#ifdef SK_SUPPORT_LEGACY_PUBLICEFFECTCONSTRUCTORS
public:
#endif
    Sk2DPathEffect(const SkMatrix& mat);

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
    static SkLine2DPathEffect* Create(SkScalar width, const SkMatrix& matrix) {
        return SkNEW_ARGS(SkLine2DPathEffect, (width, matrix));
    }

    virtual bool filterPath(SkPath* dst, const SkPath& src,
                            SkStrokeRec*, const SkRect*) const SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLine2DPathEffect)

protected:
    virtual void nextSpan(int u, int v, int ucount, SkPath*) const SK_OVERRIDE;

    SkLine2DPathEffect(SkReadBuffer&);

    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

#ifdef SK_SUPPORT_LEGACY_PUBLICEFFECTCONSTRUCTORS
public:
#endif
    SkLine2DPathEffect(SkScalar width, const SkMatrix& matrix)
    : Sk2DPathEffect(matrix), fWidth(width) {}

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
    static SkPath2DPathEffect* Create(const SkMatrix& matrix, const SkPath& path) {
        return SkNEW_ARGS(SkPath2DPathEffect, (matrix, path));
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPath2DPathEffect)

protected:
    SkPath2DPathEffect(SkReadBuffer& buffer);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual void next(const SkPoint&, int u, int v, SkPath*) const SK_OVERRIDE;

#ifdef SK_SUPPORT_LEGACY_PUBLICEFFECTCONSTRUCTORS
public:
#endif
    SkPath2DPathEffect(const SkMatrix&, const SkPath&);

private:
    SkPath  fPath;

    typedef Sk2DPathEffect INHERITED;
};

#endif
