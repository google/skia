/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDashPathEffect_DEFINED
#define SkDashPathEffect_DEFINED

#include "SkPathEffect.h"

/** \class SkDashPathEffect

    SkDashPathEffect is a subclass of SkPathEffect that implements dashing
*/
class SK_API SkDashPathEffect : public SkPathEffect {
public:
    /** intervals: array containing an even number of entries (>=2), with
         the even indices specifying the length of "on" intervals, and the odd
         indices specifying the length of "off" intervals.
        count: number of elements in the intervals array
        phase: offset into the intervals array (mod the sum of all of the
         intervals).

        For example: if intervals[] = {10, 20}, count = 2, and phase = 25,
         this will set up a dashed path like so:
         5 pixels off
         10 pixels on
         20 pixels off
         10 pixels on
         20 pixels off
         ...
        A phase of -5, 25, 55, 85, etc. would all result in the same path,
         because the sum of all the intervals is 30.

        Note: only affects stroked paths.
    */
    SkDashPathEffect(const SkScalar intervals[], int count, SkScalar phase,
                     bool scaleToFit = false);
    virtual ~SkDashPathEffect();

    virtual bool filterPath(SkPath* dst, const SkPath& src,
                            SkStrokeRec*, const SkRect*) const SK_OVERRIDE;

    virtual bool asPoints(PointData* results, const SkPath& src,
                          const SkStrokeRec&, const SkMatrix&,
                          const SkRect*) const SK_OVERRIDE;

    virtual Factory getFactory() const SK_OVERRIDE;

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer&);

protected:
    SkDashPathEffect(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    SkScalar*   fIntervals;
    int32_t     fCount;
    // computed from phase
    SkScalar    fInitialDashLength;
    int32_t     fInitialDashIndex;
    SkScalar    fIntervalLength;
    bool        fScaleToFit;

    typedef SkPathEffect INHERITED;
};

#endif
