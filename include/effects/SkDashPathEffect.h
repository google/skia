
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
    /** The intervals array must contain an even number of entries (>=2), with the even
        indices specifying the "on" intervals, and the odd indices specifying the "off"
        intervals. phase is an offset into the intervals array (mod the sum of all of the
        intervals).
        Note: only affects framed paths
    */
    SkDashPathEffect(const SkScalar intervals[], int count, SkScalar phase, bool scaleToFit = false);
    virtual ~SkDashPathEffect();

    // overrides for SkPathEffect
    //  This method is not exported to java.
    virtual bool filterPath(SkPath* dst, const SkPath& src, SkScalar* width);

    // overrides for SkFlattenable
    //  This method is not exported to java.
    virtual Factory getFactory();
    //  This method is not exported to java.
    virtual void flatten(SkFlattenableWriteBuffer&);

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer&);

protected:
    SkDashPathEffect(SkFlattenableReadBuffer&);
    
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

