/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDashImpl_DEFINED
#define SkDashImpl_DEFINED

#include "include/core/SkPathEffect.h"

class SkDashImpl : public SkPathEffect {
public:
    SkDashImpl(const SkScalar intervals[], int count, SkScalar phase);

protected:
    ~SkDashImpl() override;
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

    bool onAsPoints(PointData* results, const SkPath& src, const SkStrokeRec&, const SkMatrix&,
                    const SkRect*) const override;

    DashType onAsADash(DashInfo* info) const override;

private:
    SK_FLATTENABLE_HOOKS(SkDashImpl)

    SkScalar*   fIntervals;
    int32_t     fCount;
    SkScalar    fPhase;
    // computed from phase

    SkScalar    fInitialDashLength;
    int32_t     fInitialDashIndex;
    SkScalar    fIntervalLength;

    typedef SkPathEffect INHERITED;
};

#endif
