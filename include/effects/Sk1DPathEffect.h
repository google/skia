/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk1DPathEffect_DEFINED
#define Sk1DPathEffect_DEFINED

#include "SkFlattenable.h"
#include "SkPathEffect.h"
#include "SkPath.h"

class SkPathMeasure;

// This class is not exported to java.
class SK_API Sk1DPathEffect : public SkPathEffect {
public:
protected:
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

    /** Called at the start of each contour, returns the initial offset
        into that contour.
    */
    virtual SkScalar begin(SkScalar contourLength) const = 0;
    /** Called with the current distance along the path, with the current matrix
        for the point/tangent at the specified distance.
        Return the distance to travel for the next call. If return <= 0, then that
        contour is done.
    */
    virtual SkScalar next(SkPath* dst, SkScalar dist, SkPathMeasure&) const = 0;

private:
    typedef SkPathEffect INHERITED;
};

class SK_API SkPath1DPathEffect : public Sk1DPathEffect {
public:
    enum Style {
        kTranslate_Style,   // translate the shape to each position
        kRotate_Style,      // rotate the shape about its center
        kMorph_Style,       // transform each point, and turn lines into curves

        kLastEnum_Style = kMorph_Style,
    };

    /** Dash by replicating the specified path.
        @param path The path to replicate (dash)
        @param advance The space between instances of path
        @param phase distance (mod advance) along path for its initial position
        @param style how to transform path at each point (based on the current
                     position and tangent)
    */
    static sk_sp<SkPathEffect> Make(const SkPath& path, SkScalar advance, SkScalar phase, Style);

protected:
    SkPath1DPathEffect(const SkPath& path, SkScalar advance, SkScalar phase, Style);
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath*, const SkPath&, SkStrokeRec*, const SkRect*) const override;

    // overrides from Sk1DPathEffect
    SkScalar begin(SkScalar contourLength) const override;
    SkScalar next(SkPath*, SkScalar, SkPathMeasure&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkPath1DPathEffect)

    SkPath      fPath;          // copied from constructor
    SkScalar    fAdvance;       // copied from constructor
    SkScalar    fInitialOffset; // computed from phase
    Style       fStyle;         // copied from constructor

    typedef Sk1DPathEffect INHERITED;
};

#endif
