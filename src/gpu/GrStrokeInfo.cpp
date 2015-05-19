/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStrokeInfo.h"

#include "SkDashPathPriv.h"

bool GrStrokeInfo::applyDashToPath(SkPath* dst, GrStrokeInfo* dstStrokeInfo,
                                   const SkPath& src) const {
    if (this->isDashed()) {
        SkPathEffect::DashInfo info;
        info.fIntervals = fIntervals.get();
        info.fCount = fIntervals.count();
        info.fPhase = fDashPhase;
        GrStrokeInfo filteredStroke(*this, false);
        if (SkDashPath::FilterDashPath(dst, src, &filteredStroke, NULL, info)) {
            *dstStrokeInfo = filteredStroke;
            return true;
        }
    }
    return false;
}
