/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDashImpl_DEFINED
#define SkDashImpl_DEFINED

#include "SkPathEffect.h"

class SK_API SkDashImpl : public SkPathEffect {
public:
    SkDashImpl(const SkScalar intervals[], int count, SkScalar phase);

    bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

    bool asPoints(PointData* results, const SkPath& src, const SkStrokeRec&, const SkMatrix&,
                  const SkRect*) const override;

    DashType asADash(DashInfo* info) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDashImpl)

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    bool exposedInAndroidJavaAPI() const override { return true; }
#endif

protected:
    ~SkDashImpl() override;
    void flatten(SkWriteBuffer&) const override;

private:
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
