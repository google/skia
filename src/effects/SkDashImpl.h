/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDashImpl_DEFINED
#define SkDashImpl_DEFINED

#include "include/core/SkSpan.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkPathEffectBase.h"

#include <optional>

class SkDashImpl : public SkPathEffectBase {
public:
    SkDashImpl(SkSpan<const SkScalar> intervals, SkScalar phase);

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPathBuilder* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                      const SkMatrix&) const override;

    bool onAsPoints(PointData* results, const SkPath& src, const SkStrokeRec&, const SkMatrix&,
                    const SkRect*) const override;

    std::optional<DashInfo> asADash() const override;

private:
    SK_FLATTENABLE_HOOKS(SkDashImpl)

    bool computeFastBounds(SkRect* bounds) const override {
        // Dashing a path returns a subset of the input path so just return true and leave
        // bounds unmodified
        return true;
    }

    skia_private::AutoTArray<SkScalar> fIntervals;
    SkScalar fPhase;

    // computed from phase
    SkScalar    fInitialDashLength;
    SkScalar    fIntervalLength;
    size_t      fInitialDashIndex;

    using INHERITED = SkPathEffectBase;
};

#endif
